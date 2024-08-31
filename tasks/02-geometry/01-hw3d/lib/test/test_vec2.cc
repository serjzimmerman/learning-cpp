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
#include "geometry/vec2.hpp"

using vec = throttle::geometry::vec2<float>;
using namespace throttle::geometry;

template struct throttle::geometry::vec2<float>;

TEST(test_vec2, test_colinear) {
  vec a{1, 2};
  vec b{-2.3, -4.6};

  EXPECT_TRUE(colinear(a * 1.0f, b * 8.0f));
}

TEST(test_vec2, test_project_normal) {
  vec a{1, 1};
  vec b{1, -1};

  auto a_pr_b = a.project(b);
  EXPECT_EQ(a_pr_b, vec::zero());
}

TEST(test_vec2, test_dot_1) {
  vec a{1, 2};
  vec b{3, 2};

  EXPECT_TRUE(is_roughly_equal(dot(a, b), 7.0f));
}

TEST(test_vec2, test_len_zero) {
  auto a = vec::zero();
  EXPECT_TRUE(is_roughly_equal(a.length(), 0.0f));
}

TEST(test_vec2, test_norm_zero) {
  auto a = vec::zero();
  auto b = a.norm();
  EXPECT_EQ(a, b);
}

TEST(test_vec2, test_project_on_zero) {
  vec a{1, 2};
  auto zero = vec::zero();
  auto proj = a.project(zero);

  EXPECT_EQ(proj, zero);
}

TEST(test_vec2, test_co_directional) {
  vec a{1, 2};
  vec b{1, -2};
  vec c{-1, -2};
  vec d{2, 4};

  EXPECT_FALSE(co_directional(a, b));
  EXPECT_FALSE(co_directional(a, c));
  EXPECT_TRUE(co_directional(a, d));
}
