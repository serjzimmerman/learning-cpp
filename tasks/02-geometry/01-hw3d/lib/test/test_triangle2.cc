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

#include <array>
#include <cmath>

#include "geometry/equal.hpp"
#include "geometry/primitives/triangle2.hpp"

using triangle2 = throttle::geometry::triangle2<float>;
using throttle::geometry::is_roughly_equal;

template struct throttle::geometry::triangle2<float>;

TEST(test_triangle2, test_1) {
  triangle2 t{{0, 0}, {2, 6}, {4, -1}};

  EXPECT_TRUE(t.point_in_triangle({1, 3}));
  EXPECT_TRUE(t.point_in_triangle({2, 1}));
  EXPECT_FALSE(t.point_in_triangle({0, 1}));
  EXPECT_TRUE(t.point_in_triangle({0, 0}));
  EXPECT_TRUE(t.point_in_triangle({2, 6}));
  EXPECT_TRUE(t.point_in_triangle({4, -1}));

  std::swap(t.a, t.b);

  EXPECT_TRUE(t.point_in_triangle({1, 3}));
  EXPECT_TRUE(t.point_in_triangle({2, 1}));
  EXPECT_FALSE(t.point_in_triangle({0, 1}));
  EXPECT_TRUE(t.point_in_triangle({0, 0}));
  EXPECT_TRUE(t.point_in_triangle({2, 6}));
  EXPECT_TRUE(t.point_in_triangle({4, -1}));

  std::swap(t.b, t.c);

  EXPECT_TRUE(t.point_in_triangle({1, 3}));
  EXPECT_TRUE(t.point_in_triangle({2, 1}));
  EXPECT_FALSE(t.point_in_triangle({0, 1}));
  EXPECT_TRUE(t.point_in_triangle({0, 0}));
  EXPECT_TRUE(t.point_in_triangle({2, 6}));
  EXPECT_TRUE(t.point_in_triangle({4, -1}));
  EXPECT_FALSE(t.point_in_triangle({-100, 50}));
}

TEST(triangle2, test_intersect_1) {
  triangle2 a{{-1, 1}, {1, 1}, {0, -1}};
  triangle2 b{{-1, -1}, {0, 1}, {1, -1}};

  EXPECT_TRUE(a.intersect(b));
  EXPECT_TRUE(b.intersect(a));
}

TEST(triangle2, test_intersect_2) {
  triangle2 a{{-1, 1}, {1, 1}, {0, -1}};
  triangle2 b = a; // copy

  EXPECT_TRUE(a.intersect(b));
  EXPECT_TRUE(b.intersect(a));
}

TEST(triangle2, test_intersect_3) {
  triangle2 a{{0, 0}, {1, 1}, {1, -1}};
  triangle2 b{{-1, 1}, {-1, -1}, {0, 0}};

  EXPECT_TRUE(a.intersect(b));
  EXPECT_TRUE(b.intersect(a));
}

TEST(triangle2, test_intersect_4) {
  triangle2 a{{-1, 0}, {0, 1}, {0, -1}};
  triangle2 b{{0, -1}, {0, 1}, {1, 0}};

  EXPECT_TRUE(a.intersect(b));
  EXPECT_TRUE(b.intersect(a));
}

TEST(triangle2, test_intersect_5) {
  triangle2 a{{-2, 0}, {2, 2}, {2, -2}};
  triangle2 b{{-1, 0}, {1, 1}, {1, -1}};

  EXPECT_TRUE(a.intersect(b));
  EXPECT_TRUE(b.intersect(a));
}

TEST(triangle2, test_intersect_6) {
  triangle2 a{{-1, 1}, {-3, 2}, {-1, 4}};
  triangle2 b{{1, -1}, {3, -2}, {1, -4}};

  EXPECT_FALSE(a.intersect(b));
  EXPECT_FALSE(b.intersect(a));
}

TEST(triangle2, test_intersect_7) {
  triangle2 a{{1, 5}, {1, 1}, {5, 1}};
  triangle2 b{{2, 6}, {2, 2}, {6, 2}};

  EXPECT_TRUE(a.intersect(b));
  EXPECT_TRUE(b.intersect(a));
}

TEST(triangle2, test_intersect_8) {
  triangle2 a{{-3, 1}, {1, 1}, {1, -5}};
  triangle2 b{{0, 0}, {0, 2}, {2, 0}};

  EXPECT_TRUE(a.intersect(b));
  EXPECT_TRUE(b.intersect(a));
}

TEST(triangle2, test_intersect_9) {
  triangle2 a{{-3, 1}, {1, 1}, {1, -5}};
  triangle2 b{{-1, 0}, {0, -1}, {0, -1}};

  EXPECT_TRUE(a.intersect(b));
  EXPECT_TRUE(b.intersect(a));
}

TEST(triangle2, test_intersect_10) {
  triangle2 a{{-3, 1}, {1, 1}, {1, -5}};
  triangle2 b{{0, 3}, {1, 3}, {2, 0}};

  EXPECT_FALSE(a.intersect(b));
  EXPECT_FALSE(b.intersect(a));
}
