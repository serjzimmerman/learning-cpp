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

#include "ezvk/debug.hpp"
#include "ezvk/error.hpp"
#include "ezvk/utils/algorithm.hpp"
#include "ezvk/utils/utility.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <span>

namespace ezvk {

class unsupported_error : public ezvk::error {
  std::vector<std::string> m_missing;

public:
  unsupported_error(std::string msg, auto start, auto finish)
      : ezvk::error{msg}, m_missing{start, finish} {}

  auto missing() const { return m_missing; }
};

class i_instance {
public:
  virtual const vk::raii::Instance &operator()() const & = 0;
  virtual ~i_instance() {}
};

class instance : public i_instance {
  vk::raii::Instance m_instance = nullptr;

public:
  instance() = default;

  instance(const vk::raii::Context &ctx, vk::ApplicationInfo app_info,
           auto ext_start, auto ext_finish, auto layers_start,
           auto layers_finish) {
    auto [ext_ok, missing_ext] =
        supports_extensions(ext_start, ext_finish, ctx);
    auto [layers_ok, missing_layers] =
        supports_layers(layers_start, layers_finish, ctx);

    if (!ext_ok || !layers_ok) {
      std::move(missing_layers.begin(), missing_layers.end(),
                std::back_inserter(missing_ext));
      throw unsupported_error{
          "Vulkan does not support some required extensions/layers",
          missing_ext.begin(), missing_ext.end()};
    }

    auto extensions = utils::to_c_strings(ext_start, ext_finish),
         layers = utils::to_c_strings(layers_start, layers_finish);

    const auto create_info = vk::InstanceCreateInfo{
        .pApplicationInfo = &app_info,
        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data()};

    m_instance = vk::raii::Instance{ctx, create_info};
  }

  using supports_result = std::pair<bool, std::vector<std::string>>;

  [[nodiscard]] static supports_result
  supports_extensions(auto start, auto finish, const vk::raii::Context &ctx) {
    const auto supported_extensions =
        ctx.enumerateInstanceExtensionProperties();
    const auto missing_extensions = utils::find_all_missing(
        supported_extensions.begin(), supported_extensions.end(), start, finish,
        [](auto a) { return std::string_view{a.extensionName}; });
    return std::make_pair(missing_extensions.empty(), missing_extensions);
  }

  [[nodiscard]] static supports_result
  supports_layers(auto start, auto finish, const vk::raii::Context &ctx) {
    const auto supported_layers = ctx.enumerateInstanceLayerProperties();
    const auto missing_layers = utils::find_all_missing(
        supported_layers.begin(), supported_layers.end(), start, finish,
        [](auto a) { return std::string_view{a.layerName}; });
    return std::make_pair(missing_layers.empty(), missing_layers);
  }

  const vk::raii::Instance &operator()() const & override { return m_instance; }
};

} // namespace ezvk
