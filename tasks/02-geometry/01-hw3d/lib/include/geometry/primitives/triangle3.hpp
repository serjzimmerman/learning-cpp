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

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <utility>

#include "geometry/primitives/plane.hpp"
#include "geometry/primitives/segment3.hpp"
#include "geometry/primitives/triangle2.hpp"

#include "geometry/point3.hpp"
#include "geometry/vec3.hpp"

namespace throttle {
namespace geometry {

template <typename> struct triangle3;

namespace detail {
template <typename T>
bool triangle_triangle_intersect(const triangle3<T> &, const triangle3<T> &);
}

template <typename T> struct triangle3 {
  using point_type = point3<T>;
  using vec_type = vec3<T>;
  using plane_type = plane<T>;
  using flat_triangle_type = triangle2<T>;
  using segment_type = segment3<T>;

  point_type a;
  point_type b;
  point_type c;

  plane_type plane_of() const { return plane_type{a, b, c}; }
  bool lies_on_one_side(const plane_type &p_plane) const {
    return lie_on_the_same_side(p_plane, a, b, c);
  }

  vec_type norm() const {
    vec_type first = a - b, second = c - b;
    if (is_roughly_equal(dot(first, second), T{}))
      return vec_type::zero();
    return cross(second, first);
  }

  flat_triangle_type project_coord(unsigned axis) const {
    return flat_triangle_type{a.project_coord(axis), b.project_coord(axis),
                              c.project_coord(axis)};
  }

  bool intersect(const triangle3 &other) const {
    return detail::triangle_triangle_intersect(*this, other);
  }

  bool intersect(const segment_type &seg) const {
    plane_type plane = plane_of();
    auto intersection = plane.segment_intersection(seg);
    if (!intersection)
      return false;
    auto max_index = plane.normal().max_component().first;
    return project_coord(max_index).point_in_triangle(
        intersection.value().project_coord(max_index));
  }

  bool intersect(const point_type &point) const {
    plane_type plane = plane_of();
    if (is_definitely_greater(plane.distance(point), T{0}))
      return false;
    auto max_index = plane.normal().max_component().first;
    return project_coord(max_index).point_in_triangle(
        point.project_coord(max_index));
  }
};

namespace detail {
// Rearrange triangle vertices
template <typename T>
std::pair<triangle3<T>, std::array<T, 3>>
canonical_triangle(const triangle3<T> &p_tri, std::array<T, 3> p_dist) {
  auto greater_count = std::count_if(p_dist.begin(), p_dist.end(),
                                     [](T elem) { return elem > 0; });
  switch (greater_count) {
  case 1:
    break; // clang-format off
  case 2: { std::for_each(p_dist.begin(), p_dist.end(), [](T &elem) { elem *= -1; }); break; } // clang-format on
  default:
    throw std::invalid_argument(
        "Elements of distance array should all be of different signs");
  }

  auto max_index = std::distance(
      p_dist.begin(), std::max_element(p_dist.begin(), p_dist.end()));
  switch (max_index) {
  case 0:
    return std::make_pair(triangle3<T>{p_tri.b, p_tri.a, p_tri.c},
                          std::array<T, 3>{p_dist[1], p_dist[0], p_dist[2]});
  case 1:
    return std::make_pair(triangle3<T>{p_tri.a, p_tri.b, p_tri.c},
                          std::array<T, 3>{p_dist[0], p_dist[1], p_dist[2]});
  case 2:
    return std::make_pair(triangle3<T>{p_tri.a, p_tri.c, p_tri.b},
                          std::array<T, 3>{p_dist[0], p_dist[2], p_dist[1]});
  }

  throw std::runtime_error{"Something unexpected has occured"};
}

template <typename T>
bool triangle_triangle_intersect(const triangle3<T> &t1,
                                 const triangle3<T> &t2) {
  // 1. Compute the plane pi1 of the first triangle
  auto pi1 = t1.plane_of();
  // 2. Compute djstances from t2 to pi1
  std::array<T, 3> d_2 = {pi1.signed_distance(t2.a), pi1.signed_distance(t2.b),
                          pi1.signed_distance(t2.c)};
  std::for_each(d_2.begin(), d_2.end(), [](auto &elem) {
    if (are_all_roughly_zero(elem))
      elem = T{0};
  });
  // 3. Rejection test. If none of the points lie on the plane and all distances
  // have the same sign, then triangles can't intersect.
  if (are_same_sign(d_2[0], d_2[1], d_2[2]))
    return false;

  // 4. Same for triangle t2
  auto pi2 = t2.plane_of();
  // 5. Compute djstances from t1 to pi2
  std::array<T, 3> d_1 = {pi2.signed_distance(t1.a), pi2.signed_distance(t1.b),
                          pi2.signed_distance(t1.c)};
  std::for_each(d_1.begin(), d_1.end(), [](auto &elem) {
    if (are_all_roughly_zero(elem))
      elem = T{0};
  });
  // 6. Rejection test. If none of the points lie on the plane and all distances
  // have the same sign, then triangles can't intersect.
  if (are_same_sign(d_1[0], d_1[1], d_1[2]))
    return false;

  // 7. Handle degenerate cases when one or more points lies on the plane of the
  // other triangle.
  auto num_zeros1 = std::count_if(
      d_1.begin(), d_1.end(), [](const auto &elem) { return elem == T{0}; });
  if (num_zeros1) {
    using triangle_vertex_distance_pair =
        std::pair<typename triangle3<T>::point_type, T>;
    std::array<triangle_vertex_distance_pair, 3> vert_dist_arr = {
        std::make_pair(t1.a, d_1[0]), std::make_pair(t1.b, d_1[1]),
        std::make_pair(t1.c, d_1[2])};
    std::sort(vert_dist_arr.begin(), vert_dist_arr.end(),
              [](const auto &p_first, const auto &p_second) {
                return std::abs(p_first.second) < std::abs(p_second.second);
              });
    auto max_index = pi2.normal().max_component().first;
    auto t2_flat = t2.project_coord(max_index);

    switch (num_zeros1) {
    case 1: {
      auto proj_first = vert_dist_arr[0].first.project_coord(max_index);
      if (t2_flat.point_in_triangle(proj_first))
        return true;
      if (!are_same_sign(vert_dist_arr[1].second, vert_dist_arr[2].second))
        return t2_flat.point_in_triangle(
            pi2.segment_intersection(
                   {vert_dist_arr[1].first, vert_dist_arr[2].first})
                .value()
                .project_coord(max_index));
      return false;
    }

    case 2: {
      return t2_flat.point_in_triangle(
                 vert_dist_arr[0].first.project_coord(max_index)) ||
             t2_flat.point_in_triangle(
                 vert_dist_arr[1].first.project_coord(max_index));
    }

    case 3:
      return t2_flat.intersect(t1.project_coord(max_index));
    default:
      throw std::runtime_error{"Something has gone terribly wrong."};
    }
  }

  // Symmetric cases for t2.
  auto num_zeros2 = std::count_if(
      d_2.begin(), d_2.end(), [](const auto &elem) { return elem == T{0}; });
  if (num_zeros2) {
    using triangle_vertex_distance_pair =
        std::pair<typename triangle3<T>::point_type, T>;
    std::array<triangle_vertex_distance_pair, 3> vert_dist_arr = {
        std::make_pair(t2.a, d_2[0]), std::make_pair(t2.b, d_2[1]),
        std::make_pair(t2.c, d_2[2])};
    std::sort(vert_dist_arr.begin(), vert_dist_arr.end(),
              [](const auto &p_first, const auto &p_second) {
                return std::abs(p_first.second) < std::abs(p_second.second);
              });
    auto max_index = pi1.normal().max_component().first;
    auto t1_flat = t1.project_coord(max_index);

    switch (num_zeros2) {
    case 1: {
      auto proj_first = vert_dist_arr[0].first.project_coord(max_index);
      if (t1_flat.point_in_triangle(proj_first))
        return true;
      if (!are_same_sign(vert_dist_arr[1].second, vert_dist_arr[2].second))
        return t1_flat.point_in_triangle(
            pi1.segment_intersection(
                   {vert_dist_arr[1].first, vert_dist_arr[2].first})
                .value()
                .project_coord(max_index));
      return false;
    }

    case 2: {
      return t1_flat.point_in_triangle(
                 vert_dist_arr[0].first.project_coord(max_index)) ||
             t1_flat.point_in_triangle(
                 vert_dist_arr[1].first.project_coord(max_index));
    }

    case 3:
      return t1_flat.intersect(t2.project_coord(max_index));
    default:
      throw std::runtime_error{"Something has gone terribly wrong."};
    }
  }

  // 8. If we get here than all early rejection tests failed and 2 triangles
  // intersect the plane of the other. In this case 2 distances are of one sign
  // and the other is of another sign. And none of the distances are equal to 0.
  auto d = cross(pi1.normal(), pi2.normal());
  auto index = d.max_component().first;

  auto [canon_t1, canon_dist_1] = detail::canonical_triangle(t1, d_1);
  auto [canon_t2, canon_dist_2] = detail::canonical_triangle(t2, d_2);

  T p_1_a = canon_t1.a[index];
  T p_1_b = canon_t1.b[index];
  T p_1_c = canon_t1.c[index];

  T p_2_a = canon_t2.a[index];
  T p_2_b = canon_t2.b[index];
  T p_2_c = canon_t2.c[index];

  auto v1 = p_1_a + (p_1_b - p_1_a) * (canon_dist_1[0]) /
                        (canon_dist_1[0] - canon_dist_1[1]);
  auto v2 = p_1_c + (p_1_b - p_1_c) * (canon_dist_1[2]) /
                        (canon_dist_1[2] - canon_dist_1[1]);

  auto q1 = p_2_a + (p_2_b - p_2_a) * (canon_dist_2[0]) /
                        (canon_dist_2[0] - canon_dist_2[1]);
  auto q2 = p_2_c + (p_2_b - p_2_c) * (canon_dist_2[2]) /
                        (canon_dist_2[2] - canon_dist_2[1]);

  return segment1<T>{v1, v2}.intersect(segment1<T>{q1, q2});
}

} // namespace detail

} // namespace geometry
} // namespace throttle
