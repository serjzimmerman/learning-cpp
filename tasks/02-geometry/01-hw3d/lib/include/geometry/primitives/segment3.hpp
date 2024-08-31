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

#include "geometry/equal.hpp"
#include "geometry/point3.hpp"
#include "geometry/vec3.hpp"
#include "segment2.hpp"

namespace throttle {
namespace geometry {

template <typename T> struct segment3 {
  using vec_type = vec3<T>;
  using point_type = point3<T>;

  point_type a;
  point_type b;

  segment3(const point_type &p_a, const point_type &p_b) : a{p_a}, b{p_b} {}

  bool contains(const point_type &point) const {
    vec_type segment_vec = b - a, vec = b - point;
    return (co_directional(segment_vec, vec) &&
            is_roughly_greater_eq(segment_vec.length_sq(), vec.length_sq()));
  }

  bool intersect(const segment3 &other) const {
    vec_type d1 = other.a - a, d2 = other.b - a, d3 = b - a;
    if (!is_roughly_equal(triple_product(d1, d2, d3), T{0}))
      return false;

    vec_type normal = cross(d1, d2);
    if (is_roughly_equal(normal, vec_type::zero()))
      normal = cross(d2, d3);
    auto max_index = normal.max_component().first;

    return segment2{other.a.project_coord(max_index),
                    other.b.project_coord(max_index)}
        .intersect(
            segment2{a.project_coord(max_index), b.project_coord(max_index)});
  }
};

} // namespace geometry
} // namespace throttle
