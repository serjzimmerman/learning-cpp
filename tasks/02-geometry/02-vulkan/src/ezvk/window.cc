/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "unified_includes/glfw_include.hpp"

#include "ezvk/window.hpp"

#include <cassert>

namespace ezvk {

unique_glfw_window::unique_glfw_window(const std::string &name,
                                       const vk::Extent2D &extent,
                                       bool resizable) {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, (resizable ? GLFW_TRUE : GLFW_FALSE));

  auto ptr = glfwCreateWindow(extent.width, extent.height, name.c_str(),
                              nullptr, nullptr);
  if (!ptr)
    check_glfw_error();
  assert(ptr);

  m_handle = {ptr, glfw_window_deleter{}};
}

surface::surface(const vk::raii::Instance &instance,
                 const unique_glfw_window &window) {
  VkSurfaceKHR c_style_surface;

  auto res =
      glfwCreateWindowSurface(*instance, window(), nullptr, &c_style_surface);
  if (res != VK_SUCCESS)
    throw ezvk::vk_error{"Failed to create a surface"};

  m_surface = vk::raii::SurfaceKHR{instance, c_style_surface};
}

} // namespace ezvk
