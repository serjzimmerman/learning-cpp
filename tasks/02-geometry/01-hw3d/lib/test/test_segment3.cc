/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <gtest/gtest.h>
#include <numeric>

#include "geometry/equal.hpp"
#include "geometry/point3.hpp"
#include "geometry/primitives/segment3.hpp"
#include "geometry/vec3.hpp"

using segment_type = typename throttle::geometry::segment3<float>;
using point = typename throttle::geometry::point3<float>;
using vec = typename throttle::geometry::vec3<float>;

using namespace throttle::geometry;

template struct throttle::geometry::segment3<float>;

TEST(test_segment3, test_contain_point) {
  segment_type a({0, 0, 0}, {0, 0, 10});
  point p1{0, 0, 5};
  point p2{0, 0, 11};
  EXPECT_TRUE(a.contains(p1));
  EXPECT_FALSE(a.contains(p2));
}

TEST(test_segment3, test_intersect_1) {
  segment_type a{{0, 0, 0}, {0, 0, 1}};
  segment_type b{{0, 0, 0}, {0, 1, 0}};
  EXPECT_TRUE(a.intersect(b));
  std::swap(b.a, b.b);
  EXPECT_TRUE(a.intersect(b));
  std::swap(a.a, a.b);
  EXPECT_TRUE(a.intersect(b));
}

TEST(test_segment3, test_intersect_2) {
  segment_type a{{0, 0, 0}, {1, 1, 1}};
  segment_type b{{0, 0, 1}, {1, 1, 0}};
  EXPECT_TRUE(a.intersect(b));
  std::swap(b.a, b.b);
  EXPECT_TRUE(a.intersect(b));
  std::swap(a.a, a.b);
  EXPECT_TRUE(a.intersect(b));
}

TEST(test_segment3, test_intersect_3) {
  segment_type a{{0, 0, 0}, {1, 1, 1}};
  segment_type b{{1, 0, 0}, {0, 1, 0}};
  EXPECT_FALSE(a.intersect(b));
  std::swap(b.a, b.b);
  EXPECT_FALSE(a.intersect(b));
  std::swap(a.a, a.b);
  EXPECT_FALSE(a.intersect(b));
}

TEST(test_segment3, test_intersect_4) {
  segment_type a{{0, 0, 0}, {1, 1, 1}};
  segment_type b{{1, 0, 0.5}, {0, 1, 0.5}};
  EXPECT_TRUE(a.intersect(b));
  std::swap(b.a, b.b);
  EXPECT_TRUE(a.intersect(b));
  std::swap(a.a, a.b);
  EXPECT_TRUE(a.intersect(b));
}

TEST(test_segment3, test_intersect_5) {
  segment_type a{{0, 0, 0}, {1, 1, 1}};
  segment_type b{{1, 0.5, 1}, {0, 0.5, 0}};
  EXPECT_TRUE(a.intersect(b));
  std::swap(b.a, b.b);
  EXPECT_TRUE(a.intersect(b));
  std::swap(a.a, a.b);
  EXPECT_TRUE(a.intersect(b));
}

TEST(test_segment3, test_intersect_6) {
  segment_type a{{-1, -1, -1}, {1, 1, 1}};
  segment_type b{{0.5, 0.5, 0.5}, {-0.5, -0.5, -0.5}};
  EXPECT_TRUE(a.intersect(b));
  std::swap(b.a, b.b);
  EXPECT_TRUE(a.intersect(b));
  std::swap(a.a, a.b);
  EXPECT_TRUE(a.intersect(b));
}

TEST(test_segment3, test_intersect_7) {
  segment_type a{{1, 1, 1}, {1, 1, 1}};
  segment_type b{{2, 2, 2}, {-0.5, -0.5, -0.5}};
  EXPECT_TRUE(a.intersect(b));
  std::swap(b.a, b.b);
  EXPECT_TRUE(a.intersect(b));
  std::swap(a.a, a.b);
  EXPECT_TRUE(a.intersect(b));
}

TEST(test_segment3, test_intersect_8) {
  segment_type a{{0, 0, 0}, {1, 1, 1}};
  segment_type b{{1, 0.5, 10}, {0, 0.5, 10}};
  EXPECT_FALSE(a.intersect(b));
  std::swap(b.a, b.b);
  EXPECT_FALSE(a.intersect(b));
  std::swap(a.a, a.b);
  EXPECT_FALSE(a.intersect(b));
}

TEST(test_segment3, test_intersect_9) {
  segment_type a{{1, 1, 1}, {1, 1, 1}};
  segment_type b{{1, 1, 1}, {1, 1, 1}};
  EXPECT_TRUE(a.intersect(b));
  std::swap(b.a, b.b);
  EXPECT_TRUE(a.intersect(b));
  std::swap(a.a, a.b);
  EXPECT_TRUE(a.intersect(b));
}
