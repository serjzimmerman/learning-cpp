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

#include "geometry/equal.hpp"
#include "geometry/narrowphase/aabb.hpp"
#include "geometry/point3.hpp"
#include "geometry/primitives/segment3.hpp"
#include "geometry/primitives/triangle3.hpp"

#include <variant>

namespace throttle {
namespace geometry {

template <typename T> class collision_shape {
public:
  using segment_type = segment3<T>;
  using point_type = point3<T>;
  using triangle_type = triangle3<T>;
  using aabb_type = axis_aligned_bb<T>;
  using variant_type = std::variant<segment_type, point_type, triangle_type>;

private:
  variant_type m_shape;
  aabb_type m_aabb;

public:
  collision_shape(const segment_type &seg)
      : m_shape{seg}, m_aabb{seg.a, seg.b} {}
  collision_shape(const point_type &point) : m_shape{point}, m_aabb{point} {}
  collision_shape(const triangle_type &tri)
      : m_shape{tri}, m_aabb{tri.a, tri.b, tri.c} {}

  bool collide(const collision_shape &other) const {
    if (!m_aabb.intersect(other.m_aabb))
      return false;
    return std::visit(
        [](auto &&first, auto &&second) -> bool {
          return intersect(first, second);
        },
        m_shape, other.m_shape);
  }

  aabb_type bounding_box() const { return m_aabb; }
};

template <typename T>
bool collide(const collision_shape<T> &shape1,
             const collision_shape<T> &shape2) {
  return shape1.collide(shape2);
}

// Overloads for triangles

template <typename T>
bool intersect(const triangle3<T> &t1, const triangle3<T> &t2) {
  return t1.intersect(t2);
}
template <typename T>
bool intersect(const triangle3<T> &tri, const segment3<T> &seg) {
  return tri.intersect(seg);
}
template <typename T>
bool intersect(const triangle3<T> &tri, const point3<T> &point) {
  return tri.intersect(point);
}

// Overloads for segments

template <typename T>
bool intersect(const segment3<T> &seg1, const segment3<T> &seg2) {
  return seg1.intersect(seg2);
}
template <typename T>
bool intersect(const segment3<T> &seg, const triangle3<T> &tri) {
  return intersect(tri, seg);
}
template <typename T>
bool intersect(const segment3<T> &seg, const point3<T> &point) {
  return seg.contains(point);
}

// Overloads for points

template <typename T> bool intersect(const point3<T> &p1, const point3<T> &p2) {
  return is_roughly_equal(p1, p2);
}
template <typename T>
bool intersect(const point3<T> &point, const triangle3<T> &tri) {
  return intersect(tri, point);
}
template <typename T>
bool intersect(const point3<T> &point, const segment3<T> &seg) {
  return intersect(seg, point);
}
} // namespace geometry
} // namespace throttle
