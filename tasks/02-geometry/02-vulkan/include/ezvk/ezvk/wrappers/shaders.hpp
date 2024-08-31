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

#include <fstream>
#include <iostream>
#include <string>

#include "ezvk/utils/utility.hpp"
#include "unified_includes/vulkan_hpp_include.hpp"

namespace ezvk {

inline auto create_module(const std::string &filename,
                          const vk::raii::Device &device) {
  auto sprv = utils::read_file(filename);

  const auto module_info = vk::ShaderModuleCreateInfo{
      .codeSize = sprv.size(),
      .pCode = reinterpret_cast<const uint32_t *>(sprv.data())};

  return vk::raii::ShaderModule{device, module_info};
}

} // namespace ezvk
