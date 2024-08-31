/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include <cmath>

#include "geometry/point2.hpp"
#include "geometry/vec2.hpp"

namespace throttle {
namespace geometry {

template <typename T> struct line2 {
  using vec_type = vec2<T>;
  using point_type = point2<T>;

private:
  vec_type m_normal; // Normalized normal vector
  T m_dist;          // Distance from origin

  vec_type compute_normal(const vec_type &p_vec) {
    return vec_type{-p_vec.y, p_vec.x}.norm();
  }

public:
  line2(const point_type &p_point, const vec_type &p_dir)
      : m_normal{compute_normal(p_dir)},
        m_dist{dot(p_point - point_type::origin(), m_normal)} {}
  line2(const point_type &p_a, const point_type &p_b) : line2{p_a, p_b - p_a} {}

  static line2 line_x(T p_y = T{0}) {
    return line2{point_type{0, p_y}, vec_type{1, 0}};
  }
  static line2 line_y(T p_x = T{0}) {
    return line2{point_type{p_x, 0}, vec_type{0, 1}};
  }

  T signed_distance(const point_type &p_point) const {
    return dot(p_point - point_type::origin(), m_normal) - m_dist;
  }
  T distance(const point_type &p_point) const {
    return std::abs(signed_distance(p_point));
  }
  T distance_origin() const { return std::abs(m_dist); }

  vec_type normal() const { return m_normal; }
};

template <typename T>
T signed_distance_from_line2(const line2<T> &p_line, const point2<T> &p_point) {
  return p_line.signed_distance(p_point);
}

template <typename T>
T distance_from_line2(const line2<T> &p_line, const point2<T> &p_point) {
  return p_line.distance(p_point);
}

} // namespace geometry
} // namespace throttle
