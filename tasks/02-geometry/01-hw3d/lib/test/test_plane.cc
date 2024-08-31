/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <gtest/gtest.h>
#include <numeric>

#include <cmath>

#include "geometry/equal.hpp"
#include "geometry/primitives/plane.hpp"

using plane = throttle::geometry::plane<float>;
using throttle::geometry::is_roughly_equal;

template struct throttle::geometry::plane<float>;

TEST(test_plane, test_1) {
  plane p{plane::point_type{1, 2, 2}, plane::vec_type{1, 0, 0},
          plane::vec_type{0, 1, 0}};
  EXPECT_TRUE(is_roughly_equal(p.distance_origin(), 2.0f));
}

TEST(test_plane, test_2) {
  using throttle::geometry::colinear;

  plane p{plane::point_type{1, 0, 0}, plane::point_type{0, 1, 0},
          plane::point_type{0, 0, 1}};
  EXPECT_TRUE(is_roughly_equal(p.distance_origin(), 1.0f / std::sqrt(3.0f)));
  EXPECT_TRUE(colinear(p.normal(), plane::vec_type{1, 1, 1}.norm()));

  plane c{plane::point_type{0, -1.0f, 0}, plane::point_type{-0.5f, 0, 0},
          plane::point_type{0, 0, -1.0f / 3.0f}};
  EXPECT_TRUE(
      is_roughly_equal(std::abs(c.distance_origin()), 1.0f / std::sqrt(14.0f)));
  EXPECT_TRUE(colinear(c.normal(), (-1.0f) * plane::vec_type{2, 1, 3}.norm()));

  EXPECT_TRUE(is_roughly_equal(
      throttle::geometry::distance_from_plane(c, {128.0f, 11.0f, 5.0f}),
      283.0f / std::sqrt(14.0f)));
}

TEST(test_plane, test_3) {
  plane p{plane::point_type{1, 0, 0}, plane::point_type{0, 1, 0},
          plane::point_type{0, 0, 1}};
  EXPECT_TRUE(
      is_roughly_equal(throttle::geometry::distance_from_plane(p, {-5, 0, 0}),
                       2 * std::sqrt(3.0f)));
}

TEST(test_plane, test_4) {
  plane p = plane::plane_xy();
  using point = plane::point_type;
  EXPECT_FALSE(lie_on_the_same_side(p, point{0, 0, 1}, point{0, 2, -1}));
  EXPECT_TRUE(lie_on_the_same_side(p, point{5, -1, 2.2}, point{-8, 5, 0.1}));
}

TEST(test_plane, test_5) {
  using segment = plane::segment_type;
  plane p = plane::plane_xy();

  segment s1{{0, 0, -5}, {0, 0, 3}};
  auto i1 = p.segment_intersection(s1);
  EXPECT_TRUE(i1);
  EXPECT_TRUE(is_roughly_equal(i1.value(), plane::point_type{0, 0, 0}));

  segment s2{{-2, -2, -4}, {2, 2, 4}};
  auto i2 = p.segment_intersection(s2);
  EXPECT_TRUE(i2);
  EXPECT_TRUE(is_roughly_equal(i2.value(), plane::point_type{0, 0, 0}));

  segment s3{{1, 2, -3}, {4, 5, 6}};
  auto i3 = p.segment_intersection(s3);
  EXPECT_TRUE(i3);
  EXPECT_TRUE(is_roughly_equal(i3.value(), plane::point_type{2, 3, 0}));
}
