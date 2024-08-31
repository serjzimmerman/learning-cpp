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
#include "geometry/primitives/segment2.hpp"
#include "geometry/vec2.hpp"

using segment_type = typename throttle::geometry::segment2<float>;
using point = typename throttle::geometry::point2<float>;
using vec = typename throttle::geometry::vec2<float>;

using namespace throttle::geometry;

template struct throttle::geometry::segment2<float>;

TEST(test_segment2, test_contain_point) {
  segment_type a({0, 0}, {0, 10});
  point p1{0, 5};
  point p2{0, 11};
  EXPECT_TRUE(a.contains(p1));
  EXPECT_FALSE(a.contains(p2));
}

TEST(test_segment2, test_segment_segment_1) {
  segment_type a({0, 0}, {1, 1});
  segment_type b({0, 1}, {1, 0});
  segment_type c({0, 1}, {0, 2});

  EXPECT_TRUE(a.intersect(b));
  EXPECT_FALSE(a.intersect(c));
}

TEST(test_segment2, test_segment_segment_2) {
  segment_type a({0, 0}, {1, 1});
  segment_type b({0.5, 0.5}, {2, 2});

  EXPECT_TRUE(a.intersect(b));
}

TEST(test_segment2, test_segment_segment_3) {
  segment_type a({0, 0}, {1, 1});
  segment_type b({0.5, 0.5}, {0.5, 0.5});

  EXPECT_TRUE(a.intersect(b));
}

TEST(test_segment2, test_segment_segment_4) {
  segment_type a({1, 1}, {1, 1});
  segment_type b({1, 1}, {1, 1});

  EXPECT_TRUE(a.intersect(b));
}

TEST(test_segment2, test_segment_segment_5) {
  segment_type a({0, 0}, {0, 1});
  segment_type b({1, 0}, {1, 1});

  EXPECT_FALSE(a.intersect(b));
}

TEST(test_segment2, test_segment_segment_6) {
  segment_type a({0, 0}, {0, 1});
  segment_type b({0, 1}, {0, 2});

  EXPECT_TRUE(a.intersect(b));
}

TEST(test_segment2, test_segment_segment_7) {
  segment_type a({0, 0}, {0, 1});
  segment_type b({0, 2}, {0, 3});

  EXPECT_FALSE(a.intersect(b));
}

TEST(test_segment2, test_segment_segment_8) {
  segment_type a({0, 0}, {1, 0});
  segment_type b({1, 2}, {2, -3});

  EXPECT_FALSE(a.intersect(b));
}

TEST(test_segment2, test_segment_segment_9) {
  segment_type a({0, 0}, {0, 0});
  segment_type b({2, 2}, {2, 2});

  EXPECT_FALSE(a.intersect(b));
}

TEST(test_segment2, test_segment_segment_10) {
  segment_type a({0, 0}, {1, 0});
  segment_type b({2, 2}, {-2, -0.5});

  EXPECT_FALSE(a.intersect(b));
}

TEST(test_segment2, test_segment_segment_11) {
  segment_type a({-1, 0}, {1, 0});
  segment_type b({0, 1}, {2, -1});

  EXPECT_TRUE(a.intersect(b));

  segment_type c{{0, -1}, {0, 1}};
  segment_type d{{-1, -1}, {1, -1}};

  EXPECT_TRUE(c.intersect(d));
}

TEST(test_segment2, test_segment_segment_12) {
  segment_type a({-1, 0}, {1, 0});
  segment_type b({-0.5, 0}, {0.5, 0});

  EXPECT_TRUE(a.intersect(b));

  segment_type c{{-0.66, -0.33}, {2, 1}};
  segment_type d{{0.2, 0.1}, {4, 2}};

  EXPECT_TRUE(c.intersect(d));
}
