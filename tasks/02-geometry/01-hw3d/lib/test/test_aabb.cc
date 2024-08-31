/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru>, wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <gtest/gtest.h>
#include <numeric>

#include "geometry/narrowphase/aabb.hpp"

using AABB = typename throttle::geometry::axis_aligned_bb<double>;
using namespace throttle::geometry;

template struct throttle::geometry::axis_aligned_bb<float>;

TEST(TestAABB, test_intersect_1) {
  AABB a{{0, 0, 0}, 0.5, 0.5, 0.5};
  AABB b{{100, 100, 100}, 1, 2, 3};

  EXPECT_FALSE(a.intersect(b));
}

TEST(TestAABB, test_intersect_2) {
  AABB a{{0, 0, 0}, 0.5, 0.5, 0.5};
  AABB b{{0, 0, 1}, 1, 1, 1};

  EXPECT_TRUE(a.intersect(b));
}

TEST(TestAABB, test_intersect_3) {
  AABB a{{0, 0, 0}, 0.5, 0.5, 0.5};
  AABB b{{0, 0, 1}, 0.5, 0.5, 0.5};

  EXPECT_TRUE(a.intersect(b));
}

TEST(TestAABB, test_intersect_4) {
  AABB a{{0, 0, 0}, 0.5, 0.5, 0.5};
  AABB b{{0, 0.5, 0}, 0, 0.5, 0.5};

  EXPECT_TRUE(a.intersect(b));
}

TEST(TestAABB, test_intersect_5) {
  AABB a{{0, 0, 0}, 0.5, 0.5, 0.5};
  AABB b{{0, 1, 0}, 0, 0.5, 0.5};

  EXPECT_TRUE(a.intersect(b));
}

TEST(TestAABB, test_intersect_6) {
  AABB a{{0, 0, 0}, 0.5, 0, 0.5};
  AABB b{{0, 0, 0}, 0, 0.5, 0.5};

  EXPECT_TRUE(a.intersect(b));
}

TEST(TestAABB, test_intersect_7) {
  AABB a{{0, 0, 0}, 0.5, 0.5, 0.5};
  AABB b{{-0.5, 0.75, 0}, 0, 0.5, 0.5};

  EXPECT_TRUE(a.intersect(b));
}

TEST(TestAABB, test_intersect_8) {
  AABB a{{0, 0, 0}, 0, 0, 0};
  AABB b{{0, 0, 0}, 0, 0, 0};

  EXPECT_TRUE(a.intersect(b));
}

TEST(TestAABB, test_intersect_9) {
  AABB a{{0, 0.3, 0}, 0, 0.5, 0};
  AABB b{{0, 0, 0}, 0, 0.5, 0};

  EXPECT_TRUE(a.intersect(b));
}

TEST(TestAABB, test_intersect_10) {
  AABB a{{0, 0.3, 0}, 0, 0.5, 0.5};
  AABB b{{0, 0, 0}, 0, 0.5, 0.5};

  EXPECT_TRUE(a.intersect(b));
}

TEST(TestAABB, test_intersect_with_plane_1) {
  AABB a{{0, 0.3, 0}, 0.5, 0.5, 0.5};
  EXPECT_TRUE(a.intersect_xy(0));
}

TEST(TestAABB, test_intersect_with_plane_2) {
  AABB a{{0, 0.3, 0}, 0.5, 0.5, 0.5};
  EXPECT_TRUE(a.intersect_xy(0.5));
}

TEST(TestAABB, test_intersect_with_plane_3) {
  AABB a{{0, 0.3, 0}, 0.5, 0.5, 0.5};
  EXPECT_FALSE(a.intersect_xy(1.0));
}

TEST(TestAABB, test_intersect_with_plane_4) {
  AABB a{{0, 0.3, 0}, 0.5, 0.5, 0.5};
  EXPECT_FALSE(a.intersect_xy(-1.0));
}

TEST(TestAABB, test_intersect_with_plane_5) {
  AABB a{{0, 0.3, 0}, 0.5, 0.5, 0.5};
  EXPECT_TRUE(a.intersect_xy(-0.5));
}

TEST(TestAABB, test_min_max_corner) {
  using point = AABB::point_type;
  AABB a{{1.0, 2.0, 3.0}, 1.0, 2.0, 3.0};
  EXPECT_TRUE(is_roughly_equal(a.minimum_corner(), point{0, 0, 0}));
  EXPECT_TRUE(is_roughly_equal(a.maximum_corner(), point{2.0, 4.0, 6.0}));
}

TEST(TestAABB, test_variadic_constructor) {
  using point = AABB::point_type;
  AABB a{point{1, 2, 3}, point{4, 5, 6}, point{7, 8, 9}};

  EXPECT_TRUE(is_roughly_equal(a.m_center, {4, 5, 6}));
  EXPECT_TRUE(is_roughly_equal(a.m_halfwidth_x, 6.0 / 2));
  EXPECT_TRUE(is_roughly_equal(a.m_halfwidth_y, 6.0 / 2));
  EXPECT_TRUE(is_roughly_equal(a.m_halfwidth_z, 6.0 / 2));
}
