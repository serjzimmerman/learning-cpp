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

#include "unified_includes/vulkan_hpp_include.hpp"

#include "queues.hpp"

#include "ezvk/utils/algorithm.hpp"
#include "ezvk/utils/utility.hpp"

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

namespace ezvk {

namespace physical_device_selector {

using supports_result = std::pair<bool, std::vector<std::string>>;

[[nodiscard]] inline supports_result
supports_extensions(const vk::raii::PhysicalDevice &device, auto ext_start,
                    auto ext_finish) {
  const auto supported_extensions = device.enumerateDeviceExtensionProperties();
  auto missing_extensions = utils::find_all_missing(
      supported_extensions.begin(), supported_extensions.end(), ext_start,
      ext_finish, [](auto a) { return std::string_view{a.extensionName}; });
  return std::make_pair(missing_extensions.empty(), missing_extensions);
}

inline std::vector<vk::raii::PhysicalDevice>
enumerate_suitable_physical_devices(const vk::raii::Instance &instance,
                                    auto ext_start, auto ext_finish) {
  auto available_devices = instance.enumeratePhysicalDevices();
  std::vector<vk::raii::PhysicalDevice> suitable_devices;

  for (const auto &device : available_devices) {
    if (supports_extensions(device, ext_start, ext_finish).first)
      suitable_devices.push_back(std::move(device));
  }

  return suitable_devices;
}

}; // namespace physical_device_selector

class logical_device final {
  vk::raii::Device m_device = nullptr;

public:
  logical_device() = default;

  using device_queue_create_infos = std::vector<vk::DeviceQueueCreateInfo>;
  logical_device(const vk::raii::PhysicalDevice &p_device,
                 device_queue_create_infos requested_queues, auto ext_start,
                 auto ext_finish,
                 vk::PhysicalDeviceFeatures features = {
                     .fillModeNonSolid = VK_TRUE}) {
    const auto extensions = utils::to_c_strings(ext_start, ext_finish);

    const auto device_create_info = vk::DeviceCreateInfo{
        .queueCreateInfoCount = static_cast<uint32_t>(requested_queues.size()),
        .pQueueCreateInfos = requested_queues.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
        .pEnabledFeatures = &features};

    m_device = p_device.createDevice(device_create_info);
  }

  const auto &operator()() const & { return m_device; }
};

} // namespace ezvk
