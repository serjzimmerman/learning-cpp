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

namespace triangles {

static constexpr uint32_t c_color_count = 4;

struct ubo {
  glm::mat4 vp; // 1. View-Projection matrix

  // 2. Color pallete for runtime configurable colors
  std::array<glm::vec4, c_color_count> colors;

  // 3. Light color
  glm::vec4 light_color;

  // 4. Diffuse direction
  glm::vec4 light_direction;

  // 5. Ambient strength, so that there are no padding elements with std140
  float ambient_light_strength;
};

} // namespace triangles
