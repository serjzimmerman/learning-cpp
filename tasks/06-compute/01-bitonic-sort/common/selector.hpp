/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */
#pragma once

#include "opencl_include.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cassert>
#include <charconv>
#include <compare>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <system_error>

namespace clutils {

struct platform_version {
  int major, minor;
};

inline auto operator<=>(const platform_version &lhs,
                        const platform_version &rhs) {
  if (auto cmp = lhs.major <=> rhs.major; cmp != 0)
    return cmp;
  return lhs.minor <=> rhs.minor;
}

struct platform_version_ext {
  platform_version ver;
  std::string platform_specific;
};

// Pass only valid opencl version string to this function
inline platform_version_ext
decode_platform_version(std::string version_string) {
  // This should always work
  // https://registry.khronos.org/OpenCL/sdk/3.0/docs/man/html/clGetPlatformInfo.html
  auto version_start = version_string.find_first_of("0123456789");

  if (version_start == std::string::npos)
    throw std::invalid_argument{"OpenCL platform version string is invalid"};

  version_string = version_string.substr(version_start);
  auto version_finish = version_string.find_first_of(" ");
  std::string major_minor_string = version_string.substr(0, version_finish);

  std::string platform_specific;
  if (version_finish != std::string::npos) {
    platform_specific = version_string.substr(version_finish + 1);
  }

  auto to_int = [](std::string_view s) -> std::optional<int> {
    if (int value; std::from_chars(s.data(), s.data() + s.size(), value).ec ==
                   std::errc{}) {
      return value;
    } else {
      return std::nullopt;
    }
  };

  // Split into minor/major version numbers
  auto sep = major_minor_string.find(".");
  if (sep == std::string::npos)
    throw std::invalid_argument{"OpenCL platform version string is invalid"};

  auto major_string = major_minor_string.substr(0, sep);
  auto minor_string = major_minor_string.substr(sep + 1);

  auto major = to_int(major_string), minor = to_int(minor_string);
  if (!major || !minor)
    throw std::invalid_argument{"OpenCL platform version string is invalid"};

  return platform_version_ext{platform_version{major.value(), minor.value()},
                              platform_specific};
}

using support_result = typename std::pair<bool, std::vector<std::string>>;

inline support_result
device_supports_extensions(cl::Device device, auto ext_start, auto ext_finish) {
  const auto supported_extensions = device.getInfo<CL_DEVICE_EXTENSIONS>();
  std::vector<std::string> missing_extensions;

  for (; ext_start != ext_finish; ++ext_start) {
    if (supported_extensions.find(*ext_start) == std::string::npos)
      missing_extensions.push_back(*ext_start);
  }

  return std::make_pair(missing_extensions.empty(), missing_extensions);
}

class platform_selector {
protected:
  cl::Platform m_platform;
  cl::Device m_device;

public:
  static constexpr auto default_pred = [](auto) { return true; };

  using platform_pred_type = std::function<bool(cl::Platform)>;
  using device_pred_type = std::function<bool(cl::Device)>;

  platform_selector(platform_version min_ver, bool verbose = true,
                    platform_pred_type platform_pred = default_pred,
                    device_pred_type device_pred = default_pred) {
    std::vector<cl::Platform> platforms, suitable_platforms;
    cl::Platform::get(&platforms);

    std::copy_if(
        platforms.begin(), platforms.end(),
        std::back_inserter(suitable_platforms),
        [min_ver, platform_pred, verbose](auto p) {
          const auto version =
              decode_platform_version(p.template getInfo<CL_PLATFORM_VERSION>())
                  .ver;
          if (verbose) {
            std::cout << "Info: Found platform: "
                      << p.template getInfo<CL_PLATFORM_NAME>()
                      << ", version: " << version.major << "." << version.minor
                      << "\n";
          }

          if (version < min_ver && verbose) {
            std::cout
                << "Info: Does not fit minimum version requirements, have: "
                << version.major << "." << version.minor
                << ", requested: " << min_ver.major << "." << min_ver.minor
                << "\n";
          }

          return (version >= min_ver) && platform_pred(p);
        });

    if (suitable_platforms.empty())
      throw std::runtime_error{"No fitting OpenCL platform found"};

    auto chosen_platform = std::find_if(
        suitable_platforms.begin(), suitable_platforms.end(), [&](auto p) {
          std::vector<cl::Device> devices;
          p.getDevices(CL_DEVICE_TYPE_GPU, &devices);

          auto chosen_device =
              std::find_if(devices.begin(), devices.end(),
                           [device_pred](auto d) { return device_pred(d); });
          if (chosen_device == devices.end())
            return false;

          if (verbose) {
            std::cout << "Info: Found suitable device: "
                      << chosen_device->template getInfo<CL_DEVICE_NAME>()
                      << "\n";
          }

          m_device = *chosen_device;
          return true;
        });

    if (chosen_platform == suitable_platforms.end())
      throw std::runtime_error{"No suitable OpenCL device found"};
    m_platform = *chosen_platform;
  }
};

}; // namespace clutils
