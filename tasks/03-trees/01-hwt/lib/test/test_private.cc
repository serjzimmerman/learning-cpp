#include <cstddef>
#include <cstdlib>
#include <functional>
#include <gtest/gtest.h>
#include <set>
#include <string>

#define private public
#define protected public
#include "order_statistic_set.hpp"
#undef private
#undef protected

// Implicit instantiation for testing puproses
template class throttle::detail::rb_tree_ranged_<int, std::less<int>>;
template class throttle::detail::rb_tree_ranged_<std::string,
                                                 std::less<std::string>>;

using namespace throttle::detail;

namespace {
using base_node_ptr = rb_tree_ranged_node_base_ *;
using base_node = rb_tree_ranged_node_base_;

std::pair<std::size_t, bool> validate_red_black_helper(base_node_ptr p_base) {
  if (!p_base) {
    return {1, true};
  }

  if (base_node::get_color_(p_base) == k_red_ &&
      (base_node::get_color_(p_base->m_left_) == k_red_ ||
       base_node::get_color_(p_base->m_right_) == k_red_)) {
    return {0, false};
  }

  auto valid_left = validate_red_black_helper(p_base->m_left_);
  auto valid_right = validate_red_black_helper(p_base->m_right_);

  if (!valid_left.second || !valid_right.second ||
      (valid_left.first != valid_right.first)) {
    return {0, false};
  } else {
    return {valid_right.first +
                (base_node::get_color_(p_base) == k_black_ ? 1 : 0),
            true};
  }
}

bool validate_size_helper(base_node_ptr p_base) {
  if (!p_base) {
    return {true};
  }

  if (base_node::size(p_base) != base_node::size(p_base->m_left_) +
                                     base_node::size(p_base->m_right_) + 1) {
    return false;
  }

  bool valid_left = validate_size_helper(p_base->m_left_);
  bool valid_right = validate_size_helper(p_base->m_right_);

  return valid_left && valid_right;
}
} // namespace

TEST(test_rb_tree_private, test_1) {
  rb_tree_ranged_<int, std::less<int>> t;

  EXPECT_NO_THROW(for (int i = 0; i < 256000; i++) { t.insert(i); });
  EXPECT_EQ(validate_red_black_helper(t.m_root_).second, true);
  EXPECT_NO_THROW(for (int i = 0; i < 100000; i++) { t.erase(i); });
  EXPECT_EQ(validate_red_black_helper(t.m_root_).second, true);
  EXPECT_EQ(t.size(), 256000 - 100000);
  EXPECT_EQ(validate_size_helper(t.m_root_), true);
}

TEST(test_rb_tree_private, test_2) {
  rb_tree_ranged_<int, std::less<int>> t;
  bool thrown = false;

  EXPECT_NO_THROW(for (int i = 0; i < 65536; i++) {
    int temp = std::rand();
    if (!t.contains(temp)) {
      t.insert(temp);
    }
  });

  EXPECT_EQ(validate_size_helper(t.m_root_), true);
}

TEST(test_rb_tree_private, test_3) {
  rb_tree_ranged_<int, std::less<int>> t;
  for (int i = 0; i < 1024; i++) {
    t.insert(i);
  }

  EXPECT_EQ(validate_size_helper(t.m_root_), true);

  for (int i = 0; i < 1024; i++) {
    ASSERT_TRUE(t.contains(i));
  }

  t.erase(666);
  ASSERT_FALSE(t.contains(666));

  EXPECT_EQ(validate_size_helper(t.m_root_), true);
  ASSERT_THROW(t.erase(666), std::out_of_range);

  EXPECT_EQ(validate_size_helper(t.m_root_), true);
}

TEST(test_rb_tree_private, test_4) {
  rb_tree_ranged_<int, std::less<int>> t;
  for (int i = 1; i < 131072; i++) {
    t.insert(i);
  }

  ASSERT_EQ(t.select_rank(1), 1);

  for (int i = 1; i < 131072; i++) {
    ASSERT_EQ(t.select_rank(i), i);
  }

  for (int i = 1; i < 4096; i++) {
    t.erase(i);
  }

  EXPECT_EQ(validate_size_helper(t.m_root_), true);
}

TEST(test_rb_tree_private, test_5) {
  rb_tree_ranged_<int, std::less<int>> t;

  t.insert(1);
  t.insert(5);
  t.insert(10);
  t.insert(12);
  t.insert(14);
  t.insert(18);
  t.insert(21);
  t.insert(276);

  ASSERT_EQ(t.select_rank(1), 1);
  ASSERT_EQ(t.select_rank(4), 12);
  ASSERT_EQ(t.select_rank(8), 276);
  ASSERT_EQ(t.select_rank(5), 14);
  ASSERT_EQ(t.select_rank(7), 21);
}

TEST(test_rb_tree_private, test_6) {
  rb_tree_ranged_<int, std::less<int>> t;

  t.insert(1);
  t.insert(5);
  t.insert(10);
  t.insert(12);
  t.insert(14);
  t.insert(18);
  t.insert(21);
  t.insert(276);

  ASSERT_EQ(t.get_rank_of(1), 1);
  ASSERT_EQ(t.get_rank_of(5), 2);
  ASSERT_EQ(t.get_rank_of(10), 3);
  ASSERT_EQ(t.get_rank_of(12), 4);
  ASSERT_EQ(t.get_rank_of(14), 5);
  ASSERT_EQ(t.get_rank_of(18), 6);
  ASSERT_EQ(t.get_rank_of(21), 7);
  ASSERT_EQ(t.get_rank_of(276), 8);
}

TEST(test_rb_tree_private, test_7) {
  rb_tree_ranged_<int, std::less<int>> t;

  t.insert(1);
  t.insert(5);
  t.insert(10);
  t.insert(12);
  t.insert(14);
  t.insert(18);
  t.insert(21);
  t.insert(276);

  ASSERT_EQ(t.closest_right(4), 5);
  ASSERT_EQ(t.closest_right(1), 5);
  ASSERT_EQ(t.closest_right(14), 18);
  ASSERT_EQ(t.closest_right(17), 18);
  ASSERT_EQ(t.closest_right(42), 276);

  ASSERT_THROW(t.closest_right(276), std::out_of_range);
}

TEST(test_rb_tree_private, test_8) {
  rb_tree_ranged_<int, std::less<int>> t;

  t.insert(1);
  t.insert(5);
  t.insert(10);
  t.insert(12);
  t.insert(14);
  t.insert(18);
  t.insert(21);
  t.insert(276);

  ASSERT_EQ(t.closest_left(4), 1);
  ASSERT_EQ(t.closest_left(1), 1);
  ASSERT_EQ(t.closest_left(5), 5);
  ASSERT_EQ(t.closest_left(7), 5);
  ASSERT_EQ(t.closest_left(276), 276);
  ASSERT_EQ(t.closest_left(1000), 276);
  ASSERT_EQ(t.closest_left(20), 18);

  ASSERT_EQ(t.get_rank_of(t.closest_left(15)), 5);
}

TEST(test_rb_tree_private, test_9) {
  rb_tree_ranged_<int, std::less<int>> t;

  for (int i = 0; i < 128; i++) {
    t.insert(i);
  }

  rb_tree_ranged_<int, std::less<int>> c;
  c.insert(0);

  c = t;
  ASSERT_EQ(c.size(), t.size());

  for (int i = 0; i < 128; i++) {
    ASSERT_TRUE(c.contains(i));
  }
}

TEST(test_rb_tree_private, test_10) {
  rb_tree_ranged_<int, std::less<int>> t;

  for (int i = 0; i < 128; i++) {
    t.insert(i);
  }
  t.clear();

  rb_tree_ranged_<int, std::less<int>> c;
  c.insert(0);

  c = t;
  ASSERT_EQ(c.size(), 0);

  for (int i = 0; i < 128; i++) {
    c.insert(i);
  }

  ASSERT_EQ(c.size(), 128);
  c.clear();
}

TEST(test_rb_tree_private, test_11) {
  rb_tree_ranged_<int, std::less<int>> t;

  for (int i = 0; i < 128; i++) {
    t.insert(i);
  }

  rb_tree_ranged_<int, std::less<int>> c;
  c.insert(0);
  c.insert(1);
  c.insert(-1);
  c.insert(5);
  c.insert(10);

  ASSERT_EQ(c.size(), 5);

  t = std::move(c);
  ASSERT_EQ(t.size(), 5);

  ASSERT_TRUE(t.contains(0));
  ASSERT_TRUE(t.contains(1));
  ASSERT_TRUE(t.contains(-1));
  ASSERT_TRUE(t.contains(5));
  ASSERT_TRUE(t.contains(10));

  t = std::move(t);

  ASSERT_TRUE(t.contains(0));
  ASSERT_TRUE(t.contains(1));
  ASSERT_TRUE(t.contains(-1));
  ASSERT_TRUE(t.contains(5));
  ASSERT_TRUE(t.contains(10));
}

TEST(test_rb_tree_private, test_12) {
  throttle::order_statistic_set<int> t{1, 2, 3, 4, 5};

  EXPECT_EQ(t.size(), 5);
  for (int i = 1; i <= 5; ++i) {
    EXPECT_TRUE(t.contains(i));
  }
}

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
