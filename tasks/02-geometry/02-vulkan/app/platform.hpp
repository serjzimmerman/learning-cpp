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

#include <numeric>
#include <tuple>
#include <vector>

#include "ezvk/debug.hpp"
#include "ezvk/window.hpp"
#include "ezvk/wrappers/debugged_instance.hpp"
#include "ezvk/wrappers/device.hpp"
#include "ezvk/wrappers/instance.hpp"

namespace triangles {

struct applicaton_platform {
  ezvk::generic_instance m_instance;
  ezvk::unique_glfw_window m_window = nullptr;
  vk::raii::PhysicalDevice m_p_device = nullptr;
  ezvk::surface m_surface;

public:
  applicaton_platform(ezvk::generic_instance instance,
                      ezvk::unique_glfw_window window, ezvk::surface surface,
                      vk::raii::PhysicalDevice p_device)
      : m_instance{std::move(instance)}, m_window{std::move(window)},
        m_p_device{std::move(p_device)}, m_surface{std::move(surface)} {}

  const auto &instance() const & { return m_instance(); }
  const auto &window() const & { return m_window; }
  const auto &surface() const & { return m_surface(); }
  const auto &p_device() const & { return m_p_device; }
};

} // namespace triangles
