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
#include <optional>

#include "geometry/point3.hpp"
#include "geometry/vec3.hpp"
#include "segment3.hpp"

namespace throttle {
namespace geometry {

template <typename T> struct plane {
  using vec_type = vec3<T>;
  using point_type = point3<T>;
  using segment_type = segment3<T>;

private:
  vec_type m_normal; // Normalized normal vector
  T m_dist;          // Distance from origin

public:
  plane(const point_type &p_point, const vec_type &p_normal)
      : m_normal{p_normal.norm()},
        m_dist{dot(p_point - point_type{0, 0, 0}, m_normal)} {}

  plane(const point_type &p_point, const vec_type &p_u, const vec_type &p_v)
      : plane{p_point, cross(p_u, p_v).norm()} {}
  plane(const point_type &p_a, const point_type &p_b, const point_type &p_c)
      : plane{p_a, p_b - p_a, p_c - p_a} {}

  static plane plane_xy(T p_z = T{0}) { return plane{{0, 0, p_z}, {0, 0, 1}}; }
  static plane plane_yz(T p_x = T{0}) { return plane{{p_x, 0, 0}, {1, 0, 0}}; }
  static plane plane_xz(T p_y = T{0}) { return plane{{0, p_y, 0}, {0, 1, 0}}; }

  T signed_distance(const point3<T> &p_point) const {
    return dot(p_point - point_type::origin(), m_normal) - m_dist;
  }
  T distance(const point3<T> &p_point) const {
    return std::abs(signed_distance(p_point));
  }
  T distance_origin() const { return std::abs(m_dist); }

  std::optional<point_type> segment_intersection(const segment_type &segment) {
    T dist_a = signed_distance(segment.a);
    T dist_b = signed_distance(segment.b);

    if (is_roughly_equal(dist_a, T{0}))
      return segment.a;
    if (is_roughly_equal(dist_b, T{0}))
      return segment.b;
    if (are_same_sign(dist_a, dist_b))
      return std::nullopt;

      // This code used to be this, but it's possible to avoid calling std::abs.
      // dist_a and dist_b have different signs if we get here. Then consider if
      // dist_a > 0, dist_b < 0, then dist_a - dist_b = abs(dist_a) +
      // abs(dist_b). Next consider if dist_a < 0, dist_b > 0. dist_a - dist_b =
      // -(abs(dist_a) + abs(dist_b)).
#if 0
    return std::abs(dist_a) / (std::abs(dist_a) + std::abs(dist_b)) * (segment.b - segment.a) + segment.a;
#endif
    return ((dist_a) / (dist_a - dist_b)) * (segment.b - segment.a) + segment.a;
  }

  vec_type normal() const { return m_normal; }
};

template <typename T>
T signed_distance_from_plane(const plane<T> &p_plane,
                             const point3<T> &p_point) {
  return p_plane.signed_distance(p_point);
}

template <typename T>
T distance_from_plane(const plane<T> &p_plane, const point3<T> &p_point) {
  return p_plane.distance(p_point);
}

} // namespace geometry
} // namespace throttle

#include "geometry/equal.hpp"

namespace throttle {
namespace geometry {

template <typename T, typename... Ts,
          typename = std::enable_if_t<
              std::conjunction_v<std::is_convertible<Ts, point3<T>>...>>>
bool lie_on_the_same_side(const plane<T> &p_plane, Ts... p_points) {
  return are_same_sign(p_plane.signed_distance(p_points)...);
}

} // namespace geometry
} // namespace throttle
