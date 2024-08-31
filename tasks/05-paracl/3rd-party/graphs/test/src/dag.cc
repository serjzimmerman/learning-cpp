/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "graphs/dag.hpp"

#include <gtest/gtest.h>

using dag = graphs::dag<int>;

TEST(test_dag, test_insert) {
  dag A;

  A.insert(1);
  A.insert(2);

  EXPECT_TRUE(A.insert(1, 2));
  EXPECT_TRUE(A.insert(1, 3));
  EXPECT_TRUE(A.insert(1, 4));
  EXPECT_TRUE(A.insert(2, 4));
  EXPECT_TRUE(A.insert(4, 3));

  EXPECT_EQ(A.edges(), 5);
  EXPECT_EQ(A.vertices(), 4);

  EXPECT_TRUE(A.contains(1));
  EXPECT_TRUE(A.contains(2));
  EXPECT_TRUE(A.contains(3));
  EXPECT_TRUE(A.contains(4));
  EXPECT_FALSE(A.contains(11));

  EXPECT_TRUE(A.connected(1, 2));
  EXPECT_TRUE(A.connected(1, 3));
  EXPECT_TRUE(A.connected(1, 4));
  EXPECT_FALSE(A.connected(3, 2));
}
