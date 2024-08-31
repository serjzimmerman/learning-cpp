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

#include <array>
#include <string>
#include <vector>

namespace triangles {

constexpr auto hex_to_rgba(uint32_t hex) {
  return std::to_array(
      {((hex >> 24) & 0xff) / 255.0f, ((hex >> 16) & 0xff) / 255.0f,
       ((hex >> 8) & 0xff) / 255.0f, ((hex >> 0) & 0xff) / 255.0f});
}

template <typename T> constexpr auto glm_vec_from_array(std::array<T, 4> arr) {
  return glm::vec4{arr[0], arr[1], arr[2], arr[3]};
}

template <typename T> constexpr auto glm_vec_from_array(std::array<T, 3> arr) {
  return glm::vec3{arr[0], arr[1], arr[2]};
}

}; // namespace triangles
