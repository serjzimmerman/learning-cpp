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

#include "unified_includes/glm_inlcude.hpp"
#include "unified_includes/vulkan_hpp_include.hpp"

#include "ezvk/window.hpp"
#include <array>

namespace triangles::config {

constexpr uint32_t intersect_index = 1u, regular_index = 0u,
                   wiremesh_index = 2u, bbox_index = 3u;

inline auto required_vk_extensions() {
  auto glfw_extensions = ezvk::glfw_required_vk_extensions();
  glfw_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  return glfw_extensions;
}

inline std::vector<std::string> required_vk_layers(bool validation = false) {
  if (validation)
    return {"VK_LAYER_KHRONOS_validation"};
  return {};
}

inline std::vector<std::string> required_physical_device_extensions() {
  return {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
}

} // namespace triangles::config
