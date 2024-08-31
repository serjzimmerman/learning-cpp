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

#include "geometry/segment1.hpp"

using segment = typename throttle::geometry::segment1<float>;
using namespace throttle::geometry;

template struct throttle::geometry::segment1<float>;

TEST(test_segment1, test_intersect) {
  segment a(1, 5);
  segment b(2, 3);
  segment c(0, 1);
  segment d(2, 30);

  EXPECT_TRUE(a.intersect(b));
  EXPECT_TRUE(a.intersect(c));
  EXPECT_TRUE(a.intersect(d));
  EXPECT_TRUE(b.intersect(d));
}

TEST(test_segment1, test_contains) {
  segment a(1, 5);
  segment b(2, 3);
  segment c(1, 5);

  EXPECT_TRUE(a.contains(b));
  EXPECT_TRUE(a.contains(c));
}
