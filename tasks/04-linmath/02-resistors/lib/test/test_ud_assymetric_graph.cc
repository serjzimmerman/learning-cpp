/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "datastructures/ud_asymmetric_graph.hpp"

#include <algorithm>
#include <gtest/gtest.h>
#include <iostream>
#include <numeric>
#include <vector>

using graph = throttle::containers::ud_asymmetric_graph<int, float>;

bool contains_all_edges(auto g, auto start, auto finish) {
  for (; start != finish; ++start) {
    if (!g.contains_edge(*start))
      return false;
  }
}

TEST(test_ud_asymmetric_graph, test_basics) {
  graph g;

  g.insert_edge({1, 2}, 10, -10);
  g.insert_edge({2, 3}, 2, -2);
  g.insert_edge({4, 5}, 5, -5);
  g.insert_vertex(6);

  auto components = g.connected_components();

  EXPECT_EQ(components.size(), 3);
  EXPECT_EQ(std::accumulate(components.begin(), components.end(), 0,
                            [](auto a, auto b) { return a + b.edges(); }),
            3);
  EXPECT_EQ(std::accumulate(components.begin(), components.end(), 0,
                            [](auto a, auto b) { return a + b.vertices(); }),
            6);
}
