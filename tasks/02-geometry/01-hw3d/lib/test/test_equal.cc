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

#include "geometry/equal.hpp"
using namespace throttle::geometry;

TEST(test_equal, test_1) {
  static constexpr float epsilon_flt = 1.0e-6f;
  EXPECT_FALSE(is_roughly_equal(1.0f, 1.0e-7f, epsilon_flt));
  EXPECT_TRUE(is_roughly_equal(1.0f, 1.0000000001f, epsilon_flt));
  EXPECT_FALSE(is_roughly_equal(1.0, 1.0e-11, double(epsilon_flt)));
}

TEST(test_equal, test_2) {
  EXPECT_FALSE(are_same_sign(1, 2, 3, -1));
  EXPECT_TRUE(are_same_sign(-1, -5, -1, -2));
}

TEST(test_equal, test_3) {
  EXPECT_TRUE(is_roughly_greater_eq(-1e-7, 0.));
  EXPECT_TRUE(is_roughly_greater_eq(1e7, 0.));
  EXPECT_TRUE(is_definitely_less(-1e2, 0.));
}

TEST(test_equal, test_4) {
  EXPECT_TRUE(is_roughly_less_eq(1e-7, 0.));
  EXPECT_FALSE(is_roughly_less_eq(1e7, 0.));
}
