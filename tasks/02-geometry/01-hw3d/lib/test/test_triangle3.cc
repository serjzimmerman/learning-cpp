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
#include "geometry/primitives/plane.hpp"
#include "geometry/primitives/triangle3.hpp"

using plane = throttle::geometry::plane<float>;
using triangle3 = throttle::geometry::triangle3<float>;
using throttle::geometry::is_roughly_equal;

template struct throttle::geometry::triangle3<float>;

TEST(test_triangle3, test_1) {
  triangle3 t{{1, 2, 3}, {-1, 2, 3}, {5, 8, -1}};
  EXPECT_FALSE(t.lies_on_one_side(plane::plane_xy()));
  EXPECT_FALSE(t.lies_on_one_side(plane::plane_yz()));
  EXPECT_TRUE(t.lies_on_one_side(plane::plane_xz()));
}

TEST(test_triangle3, test_canonical) {
  triangle3 t1{{1, 2, 3}, {-1, 2, 3}, {5, 8, -1}};
  plane p{{1, 1, 1}, {0, 0, 1}};

  std::array<float, 3> dist;
  dist[0] = p.signed_distance(t1.a);
  dist[1] = p.signed_distance(t1.b);
  dist[2] = p.signed_distance(t1.c);

  auto c = throttle::geometry::detail::canonical_triangle(t1, dist);
  EXPECT_EQ(c.first.b, t1.c);

  triangle3 t2{{1, 2, 8}, {-1, 2, -4}, {5, 8, -1}};
  dist[0] = p.signed_distance(t2.a);
  dist[1] = p.signed_distance(t2.b);
  dist[2] = p.signed_distance(t2.c);

  c = throttle::geometry::detail::canonical_triangle(t2, dist);
  EXPECT_EQ(c.first.b, t2.a);
}

TEST(test_triangle3, test_intersect_1) {
  triangle3 a{{0, 0, 0}, {1, 1, 1}, {3, 5, 2}};
  triangle3 b{{3, 4, 1}, {2, 1, 4}, {-4, 3, 0}};
  EXPECT_TRUE(a.intersect(b));
}

TEST(test_triangle3, test_intersect_2) {
  triangle3 a{{0, 0, 0}, {1, 1, 1}, {1, 2, 3}};
  triangle3 b{{3, 4, 1}, {2, 1, 4}, {-4, 3, 0}};
  EXPECT_TRUE(a.intersect(b));
}

TEST(test_triangle3, test_intersect_3) {
  triangle3 a{{0, 0, 0}, {1, 1, 0}, {1, 2, 0}};
  triangle3 b{{3, 4, 1}, {2, 1, 1}, {-4, 3, 0}};
  EXPECT_FALSE(a.intersect(b));
}

TEST(test_triangle3, test_intersect_4) {
  triangle3 a{{0, 0, 0}, {1, 1, 0}, {1, 2, 0}};
  triangle3 b{{3, 4, 1}, {2, 1, 1}, {-4, 3, 1}};
  EXPECT_FALSE(a.intersect(b));
}

TEST(test_triangle3, test_intersect_5) {
  triangle3 a{{0, 0, 0}, {1, 1, 0}, {1, 2, 0}};
  triangle3 b{{-1, 4, 5}, {1, 4, 1}, {0, 4, 0}};
  EXPECT_FALSE(a.intersect(b));
}

TEST(test_triangle3, test_intersect_6) {
  triangle3 a{{-3, 1, 0}, {1, 1, 0}, {1, -5, 0}};
  triangle3 b{{-1, 0, 0}, {0, -1, 0}, {0, -0.5, 0}};
  EXPECT_TRUE(a.intersect(b));
}

TEST(test_triangle3, test_intersect_7) {
  triangle3 a{{-3, 1, 0}, {1, 1, 0}, {1, -5, 0}};
  triangle3 b{{1, -6, 0}, {1, 2, 1}, {1, 5, -4}};
  EXPECT_TRUE(a.intersect(b));
}

TEST(test_triangle3, test_intersect_8) {
  triangle3 a{{-3, 1, 0}, {1, 1, 0}, {1, -5, 0}};
  triangle3 b{{1, -5, 0}, {3, 2, 1}, {1, 5, -4}};
  EXPECT_TRUE(a.intersect(b));
}

TEST(test_triangle3, test_intersect_9) {
  triangle3 a{{5, 0, 0}, {0, 5, 0}, {0, 0, 0}};
  triangle3 b{{0, 0, 0}, {0, 5, 0}, {0, 0, 5}};
  EXPECT_TRUE(a.intersect(b));
}
