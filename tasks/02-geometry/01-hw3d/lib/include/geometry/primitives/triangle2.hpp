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

#include <cassert>
#include <cmath>

#include "geometry/equal.hpp"
#include "geometry/point2.hpp"
#include "geometry/vec2.hpp"
#include "segment2.hpp"

namespace throttle {
namespace geometry {

template <typename T> struct triangle2 {
  using point_type = point2<T>;
  using vec_type = vec2<T>;
  using segment_type = segment2<T>;

  point_type a;
  point_type b;
  point_type c;

  bool operator==(const triangle2 &other) const {
    return (a == other.a && b == other.b && c == other.c);
  }
  bool operator!=(const triangle2 &other) const { return (*this == other); }

public:
  bool point_in_triangle(const point_type &point) const {
    // Step 1. Check if the vectors ab and ac for a right oriented basis.
    T perp_product = perp_dot(b - a, c - a);
    triangle2 right =
        (perp_product > 0 ? triangle2{a, b, c} : triangle2{a, c, b});

#ifndef NDEBUG
    T new_perp = perp_dot(right.b - right.a, right.c - right.a);
    assert(new_perp > 0);
#endif

    T dist_ab = segment_type{right.a, right.b}.signed_distance(point);
    if (is_definitely_less(dist_ab, T{0}))
      return false;
    T dist_bc = segment_type{right.b, right.c}.signed_distance(point);
    if (is_definitely_less(dist_bc, T{0}))
      return false;
    T dist_ca = segment_type{right.c, right.a}.signed_distance(point);
    if (is_definitely_less(dist_ca, T{0}))
      return false;

    return true;
  }

  bool intersect(const segment_type &seg) const {
    segment_type ab = {a, b}, ac = {a, c}, cb = {c, b};
    return (ab.intersect(seg) || ac.intersect(seg) || cb.intersect(seg));
  }

  bool intersect(const triangle2 &other) const {
    // Step 1. Check if there are intersections between the sides of the
    // triangles.
    segment_type ab2 = {other.a, other.b}, ac2 = {other.a, other.c},
                 cb2 = {other.c, other.b};

    if (intersect(ab2) || intersect(ac2) || intersect(cb2))
      return true;

    // Step 2. If no sides intersects then either one triangle contains the
    // other or they don't intersect. Check if one triangle contains another.
    if (point_in_triangle(other.a) || other.point_in_triangle(a))
      return true;

    // Step 3. Triangles don't itersect.
    return false;
  }
};

} // namespace geometry
} // namespace throttle
