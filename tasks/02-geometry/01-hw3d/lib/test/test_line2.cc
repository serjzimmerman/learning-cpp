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
#include "geometry/primitives/line2.hpp"

using line = throttle::geometry::line2<float>;
using throttle::geometry::is_roughly_equal;

template struct throttle::geometry::line2<float>;

TEST(test_line2, test_1) {
  line l = line::line_x(1);
  EXPECT_TRUE(is_roughly_equal(l.distance_origin(), 1.0f));
  EXPECT_TRUE(is_roughly_equal(l.distance(line::point_type{-5, 4}), 3.0f));
}

TEST(test_line2, test_2) {
  line l{line::point_type{1, 1}, line::vec_type{1, 1}};
  EXPECT_TRUE(is_roughly_equal(l.distance_origin(), 0.0f));
  EXPECT_TRUE(l.signed_distance(line::point_type{4, 8}) *
                  signed_distance_from_line2(l, line::point_type{1, -2}) <
              0);
}
