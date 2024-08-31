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

#include "geometry/equal.hpp"
#include "geometry/point2.hpp"
#include "geometry/segment1.hpp"
#include "geometry/vec2.hpp"

namespace throttle {
namespace geometry {

template <typename T> struct segment2 {
  using vec_type = vec2<T>;
  using point_type = point2<T>;

  point_type a;
  point_type b;

  segment2(const point_type &p_a, const point_type &p_b) : a{p_a}, b{p_b} {}

  T signed_distance(const point_type &p_point) const {
    vec_type radius = p_point - b, dir = (b - a).norm(), norm = dir.perp();
    vec_type proj = dir * dot(dir, radius), perp_component = radius - proj;
    return (dot(norm, perp_component) > 0 ? perp_component.length()
                                          : -perp_component.length());
  }

  bool contains(const point_type &point) const {
    vec_type segment_vec = b - a, vec = b - point;
    return (co_directional(segment_vec, vec) &&
            is_roughly_greater_eq(segment_vec.length_sq(), vec.length_sq()));
  }

  vec_type normal() { return (b - a).perp(); }

  bool intersect(const segment2 &other) const {
    const vec_type r = b - a, s = other.b - other.a;
    const T dir_cross = perp_dot(r, s);
    const vec_type diff_start = other.a - a;

    if (is_roughly_equal(dir_cross, T{0})) {
      if (!is_roughly_equal(perp_dot(diff_start, r), T{0}))
        return false; // Parallel
      auto max_index = r.max_component().first;
      return segment1<T>{a[max_index], b[max_index]}.intersect(
          {other.a[max_index], other.b[max_index]});
    }

    const T t = perp_dot(diff_start, s) / perp_dot(r, s);
    const T u = perp_dot(diff_start, r) / perp_dot(r, s);

    return segment1<T>::unity().contains(t) && segment1<T>::unity().contains(u);
  }
};

} // namespace geometry
} // namespace throttle
