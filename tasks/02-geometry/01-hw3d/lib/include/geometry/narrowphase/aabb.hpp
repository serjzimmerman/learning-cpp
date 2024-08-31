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

#include <cmath>
#include <type_traits>

#include "geometry/equal.hpp"
#include "geometry/point3.hpp"
#include "geometry/vec3.hpp"

namespace throttle {
namespace geometry {

template <typename T> struct axis_aligned_bb {
  using vec_type = vec3<T>;
  using point_type = point3<T>;

  point_type m_center;
  T m_halfwidth_x;
  T m_halfwidth_y;
  T m_halfwidth_z;

  axis_aligned_bb(const point_type &p_center, T half_x, T half_y, T half_z)
      : m_center{p_center}, m_halfwidth_x{half_x}, m_halfwidth_y{half_y},
        m_halfwidth_z{half_z} {}

  axis_aligned_bb(const point_type &p_center, T halfwidth)
      : m_center{p_center}, m_halfwidth_x{halfwidth}, m_halfwidth_y{halfwidth},
        m_halfwidth_z{halfwidth} {}

  axis_aligned_bb(const point_type &first, const point_type &second)
      : m_center{first + T{0.5f} * (second - first)},
        m_halfwidth_x{std::abs(first.x - second.x) / T{2.0f}},
        m_halfwidth_y{std::abs(first.y - second.y) / T{2.0f}},
        m_halfwidth_z{std::abs(first.z - second.z) / T{2.0f}} {}

  template <typename... Ts, typename = std::enable_if_t<std::conjunction_v<
                                std::is_convertible<Ts, point_type>...>>>
  axis_aligned_bb(Ts... points)
      : axis_aligned_bb(
            point_type{vmin(points.x...), vmin(points.y...), vmin(points.z...)},
            point_type{vmax(points.x...), vmax(points.y...),
                       vmax(points.z...)}) {}

  point_type minimum_corner() const {
    return {m_center.x - m_halfwidth_x, m_center.y - m_halfwidth_y,
            m_center.z - m_halfwidth_z};
  }

  point_type maximum_corner() const {
    return {m_center.x + m_halfwidth_x, m_center.y + m_halfwidth_y,
            m_center.z + m_halfwidth_z};
  }

  // returns the maximum width in all 3 dimensions
  T max_width() const {
    return 2 * vmax(m_halfwidth_x, m_halfwidth_y, m_halfwidth_z);
  }

  bool intersect(const axis_aligned_bb &a) const {
    if (is_definitely_greater(std::abs(m_center.x - a.m_center.x),
                              (m_halfwidth_x + a.m_halfwidth_x)))
      return false;
    if (is_definitely_greater(std::abs(m_center.y - a.m_center.y),
                              (m_halfwidth_y + a.m_halfwidth_y)))
      return false;
    if (is_definitely_greater(std::abs(m_center.z - a.m_center.z),
                              (m_halfwidth_z + a.m_halfwidth_z)))
      return false;
    return true;
  }

  bool intersect_xy(T z) const {
    return (is_roughly_greater_eq(z, m_center.z - m_halfwidth_z) &&
            is_roughly_less_eq(z, m_center.z + m_halfwidth_z));
  }

  bool intersect_xz(T y) const {
    return (is_roughly_greater_eq(y, m_center.y - m_halfwidth_y) &&
            is_roughly_less_eq(y, m_center.y + m_halfwidth_y));
  }

  bool intersect_yz(T x) const {
    return (is_roughly_greater_eq(x, m_center.x - m_halfwidth_x) &&
            is_roughly_less_eq(x, m_center.x + m_halfwidth_x));
  }

  bool intersect_coodrinate_plane(unsigned idx, T val) const {
    switch (idx) {
    case 0:
      return intersect_yz(val);
    case 1:
      return intersect_xz(val);
    case 2:
      return intersect_xy(val);
    default:
      throw std::out_of_range("Index of plane out of range.");
    }
  }
};

} // namespace geometry
} // namespace throttle
