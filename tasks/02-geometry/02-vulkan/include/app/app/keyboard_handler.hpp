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

#include "unified_includes/glfw_include.hpp"

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <mutex>
#include <unordered_map>

namespace ezio {

class keyboard_handler {
public:
  enum class button_state : uint32_t { e_idle, e_held_down, e_pressed };
  using key_index = int;

private:
  struct tracked_key_info {
    button_state current_state, look_for;
  };

  std::unordered_map<key_index, tracked_key_info> m_tracked_keys;
  std::mutex m_mx; // I'm not sure about the efficiency of a mutex in input.

  keyboard_handler() {}

  static void key_callback(GLFWwindow *, int key, int, int action, int) {
    auto &me = instance();

    std::lock_guard guard{me.m_mx};

    auto found = me.m_tracked_keys.find(key);
    if (found == me.m_tracked_keys.end())
      return;

    auto &btn_info = found->second;

    if (action == GLFW_PRESS) {
      btn_info.current_state = button_state::e_held_down;
    } else if (action == GLFW_RELEASE) {
      btn_info.current_state = button_state::e_pressed;
    }
  }

public:
  static keyboard_handler &instance() {
    static keyboard_handler handler;
    return handler;
  }

public:
  void monitor(key_index key, button_state state_to_notify) {
    std::lock_guard guard{m_mx};
    m_tracked_keys[key] =
        tracked_key_info{button_state::e_idle, state_to_notify};
  }

  template <std::input_iterator t_it> void monitor(t_it start, t_it end) {
    std::lock_guard guard{m_mx};
    std::for_each(start, end, [&](auto k) {
      auto [key, state_to_notify] = k;
      m_tracked_keys[key] =
          tracked_key_info{button_state::e_idle, state_to_notify};
    });
  }

  static void bind(GLFWwindow *window) {
    glfwSetKeyCallback(window, key_callback);
  }

  std::unordered_map<key_index, button_state> poll() {
    std::unordered_map<key_index, button_state> result;

    std::lock_guard guard{m_mx};
    for (auto &v : m_tracked_keys) {
      if (v.second.current_state != v.second.look_for)
        continue;

      result.insert({v.first, v.second.current_state});
      // After polling the button change pressed to idle flag
      if (v.second.current_state == button_state::e_pressed)
        v.second.current_state = button_state::e_idle;
    }

    return result;
  }
};

}; // namespace ezio
