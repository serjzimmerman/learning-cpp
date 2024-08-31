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

#include <spdlog/spdlog.h>

#include "unified_includes/vulkan_hpp_include.hpp"

#include "ezvk/debug.hpp"
#include "instance.hpp"

#include <cstddef>
#include <functional>
#include <iostream>
#include <memory>
#include <span>
#include <sstream>
#include <string>

namespace ezvk {

class debugged_instance : public i_instance {
  instance m_instance;
  debug_messenger m_dmes;

public:
  debugged_instance() = default;

  debugged_instance(
      instance p_instance,
      std::function<debug_messenger::callback_type> callback =
          default_debug_callback,
      vk::DebugUtilsMessageSeverityFlagsEXT severity_flags =
          default_severity_flags,
      vk::DebugUtilsMessageTypeFlagsEXT type_flags = default_type_flags)
      : m_instance{std::move(p_instance)},
        m_dmes{m_instance(), callback, severity_flags, type_flags} {}

  const vk::raii::Instance &operator()() const & override {
    return m_instance();
  }
};

class generic_instance {
  std::unique_ptr<i_instance> m_abstract_instance_ptr = nullptr;

public:
  generic_instance() = default;
  generic_instance(instance inst)
      : m_abstract_instance_ptr{std::make_unique<instance>(std::move(inst))} {}
  generic_instance(debugged_instance inst)
      : m_abstract_instance_ptr{
            std::make_unique<debugged_instance>(std::move(inst))} {}

  const vk::raii::Instance &operator()() const & {
    return m_abstract_instance_ptr->operator()();
  }
};

}; // namespace ezvk
