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
#include "geometry/point2.hpp"
#include "geometry/vec2.hpp"

using point = throttle::geometry::point2<float>;
using vec = throttle::geometry::vec2<float>;
using namespace throttle::geometry;

template struct throttle::geometry::point2<float>;

TEST(TestPoint2, TestSubscriptOperator_1) {
  point a{1.0, 2.0};

  EXPECT_TRUE(is_roughly_equal(a[0], 1.0f));
  EXPECT_TRUE(is_roughly_equal(a[1], 2.0f));

  EXPECT_THROW(a[666], std::out_of_range);
}

TEST(TestPoint2, TestDifference) {
  point a{1, 2};
  point b{10, 16};
  auto res = b - a;
  vec right{9, 14};
  EXPECT_EQ(res, right);
}

TEST(TestPoint2, TestAdd) {
  point a{1, 2};
  vec b{10, 16};
  auto res = b + a;
  point right{11, 18};
  EXPECT_EQ(res, right);
}
