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
#include "geometry/vec3.hpp"

using vec = throttle::geometry::vec3<float>;
using namespace throttle::geometry;

template struct throttle::geometry::vec3<float>;

TEST(test_vec3, test_1) {
  vec a = vec::axis_i(), b = vec::axis_j();
  EXPECT_TRUE(is_roughly_equal(cross(a, b), vec::axis_k()));
  vec c{1.5, 2, 1};
  EXPECT_TRUE(is_roughly_equal(c.project(vec::axis_i()), 1.5f * vec::axis_i()));
  EXPECT_TRUE(is_roughly_equal(c.project(vec::axis_j()), 2.0f * vec::axis_j()));
  EXPECT_TRUE(is_roughly_equal(c.project(vec::axis_k()), 1.0f * vec::axis_k()));
}

TEST(test_vec3, test_2) {
  vec a = vec::axis_i(), b = vec::axis_j();
  EXPECT_TRUE(is_roughly_equal(dot(a, b), 0.0f));
}

TEST(test_vec3, test_colinear) {
  vec a{1, 2, 3};
  vec b{-2, -4, -6};

  EXPECT_TRUE(colinear(a, b));
}

TEST(test_vec3, test_project_normal) {
  vec a{1, 1, 0.5};
  vec b{0.5, -1, 1};

  auto a_pr_b = a.project(b);
  EXPECT_EQ(a_pr_b, vec::zero());
}

TEST(test_vec3, test_project) {
  vec a{1, 2, 4};
  vec b{3, 2, 1};

  EXPECT_TRUE(is_roughly_equal(dot(a, b), 11.0f));
}

TEST(test_vec3, test_len_zero) {
  auto a = vec::zero();
  EXPECT_TRUE(is_roughly_equal(a.length(), 0.0f));
}

TEST(test_vec3, test_norm_zero) {
  auto a = vec::zero();
  auto b = a.norm();
  EXPECT_EQ(a, b);
}

TEST(test_vec3, test_project_on_zero) {
  vec a{1, 2, 3};
  auto zero = vec::zero();
  auto proj = a.project(zero);

  EXPECT_EQ(proj, zero);
}

TEST(test_vec3, test_co_directional) {
  vec a{1, 2, 3};
  vec b{1, -2, 3};
  vec c{-1, -2, -3};
  vec d{2, 4, 6};

  EXPECT_FALSE(co_directional(a, b));
  EXPECT_FALSE(co_directional(a, c));
  EXPECT_TRUE(co_directional(a, d));
}
