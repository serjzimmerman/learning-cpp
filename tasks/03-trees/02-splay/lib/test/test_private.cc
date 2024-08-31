#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <gtest/gtest.h>
#include <iterator>
#include <numeric>
#include <set>
#include <string>

#define private public
#define protected public
#include "detail/splay_order_tree.hpp"
#include "splay_order_set.hpp"
#undef private
#undef protected

// Implicit instantiation for testing puproses
template class throttle::detail::splay_order_tree<int, std::less<int>, int>;
template class throttle::detail::splay_order_tree<
    std::string, std::less<std::string>, std::string>;

using namespace throttle::detail;

namespace {
using base_node_ptr = bst_order_node_base *;
using base_node = bst_order_node_base;

bool validate_size_helper(base_node_ptr p_base) {
  if (!p_base) {
    return true;
  }

  if (base_node::size(p_base) !=
      base_node::size(p_base->m_left) + base_node::size(p_base->m_right) + 1) {
    return false;
  }

  bool valid_left = validate_size_helper(p_base->m_left);
  bool valid_right = validate_size_helper(p_base->m_right);

  return valid_left && valid_right;
}
} // namespace

TEST(splay_order_test, test_1) {
  splay_order_tree<int, std::less<int>, int> t;

  EXPECT_NO_THROW(for (int i = 0; i < 256000; i++) { t.insert(i); });
  EXPECT_NO_THROW(for (int i = 0; i < 100000; i++) { t.erase(i); });
  EXPECT_EQ(t.size(), 156000);
  EXPECT_EQ(validate_size_helper(t.m_root), true);

  size_t count = 0;
  for (const auto &v : t) {
    count++;
  }

  EXPECT_EQ(count, 156000);
}

TEST(splay_order_test, test_2) {
  throttle::splay_order_set<int> t{};

  EXPECT_NO_THROW(for (int i = 0; i < 65536; i++) {
    int temp = std::rand();
    if (!t.contains(temp))
      t.insert(temp);
  });

  EXPECT_EQ(validate_size_helper(t.m_tree_impl.m_root), true);
}

TEST(splay_order_test, test_3) {
  throttle::splay_order_set<int> t{};
  for (int i = 0; i < 1024; i++) {
    t.insert(i);
  }

  EXPECT_EQ(validate_size_helper(t.m_tree_impl.m_root), true);

  for (int i = 0; i < 1024; i++) {
    ASSERT_TRUE(t.contains(i));
  }

  t.erase(666);
  ASSERT_FALSE(t.contains(666));
  EXPECT_EQ(validate_size_helper(t.m_tree_impl.m_root), true);
  ASSERT_THROW(t.erase(666), std::out_of_range);
  EXPECT_EQ(validate_size_helper(t.m_tree_impl.m_root), true);

  t.erase(t.find(667), t.end());
  ASSERT_EQ(validate_size_helper(t.m_tree_impl.m_root), true);
  ASSERT_EQ(*t.max(), 665);
  t.erase(t.begin(), t.find(100));
  ASSERT_EQ(validate_size_helper(t.m_tree_impl.m_root), true);
  ASSERT_EQ(*t.min(), 100);
}

TEST(splay_order_test, test_4) {
  throttle::splay_order_set<int> t{};
  for (int i = 1; i < 131072; i++) {
    t.insert(i);
  }

  ASSERT_EQ(*t.select_rank(1), 1);
  for (int i = 1; i < 131072; i++) {
    ASSERT_EQ(*t.select_rank(i), i);
  }

  for (int i = 1; i < 4096; i++) {
    t.erase(i);
  }
  EXPECT_EQ(validate_size_helper(t.m_tree_impl.m_root), true);
}

TEST(splay_order_test, test_5) {
  throttle::splay_order_set<int> t{};
  for (int i = 1; i < 131072; i++) {
    t.insert(i);
  }

  auto sum = std::count_if(t.begin(), t.end(), [](int i) { return i < 1000; });
  EXPECT_EQ(sum, 999);
}

TEST(splay_order_test, test_6) {
  throttle::splay_order_set<int> t{};

  t.insert(1);
  t.insert(5);
  t.insert(10);
  t.insert(12);
  t.insert(14);
  t.insert(18);
  t.insert(21);
  t.insert(276);

  ASSERT_EQ(*t.select_rank(1), 1);
  ASSERT_EQ(*t.select_rank(4), 12);
  ASSERT_EQ(*t.select_rank(8), 276);
  ASSERT_EQ(*t.select_rank(5), 14);
  ASSERT_EQ(*t.select_rank(7), 21);
  ASSERT_EQ(t.select_rank(200), t.end());
}

TEST(splay_order_test, test_7) {
  throttle::splay_order_set<int> t{};

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

TEST(splay_order_test, test_8) {
  throttle::splay_order_set<int> t{};

  t.insert(1);
  t.insert(5);
  t.insert(10);
  t.insert(12);
  t.insert(14);
  t.insert(18);
  t.insert(21);
  t.insert(276);

  std::set<int> s{};
  std::copy(t.begin(), t.end(), std::inserter(s, s.end()));

  for (const auto v : t) {
    EXPECT_NE(s.find(v), s.end());
  }

  EXPECT_EQ(s.size(), t.size());
}

TEST(splay_order_test, test_9) {
  throttle::splay_order_set<int> t{};
  std::set<int> s{};

  for (int i = 0; i < 262144; i += 2) {
    t.insert(i);
    s.insert(i);
  }

  for (int i = -262144; i < 262144; i++) {
    auto upper_1 = t.upper_bound(i);
    auto upper_2 = s.upper_bound(i);
    ASSERT_FALSE((upper_1 == t.end() && upper_2 != s.end()) ||
                 (upper_1 != t.end() && upper_2 == s.end()));
    EXPECT_TRUE((upper_1 == t.end() && upper_2 == s.end()) ||
                *upper_1 == *upper_2);
  }

  for (int i = -262144; i < 262144; i++) {
    auto lower_1 = t.lower_bound(i);
    auto lower_2 = s.lower_bound(i);
    ASSERT_FALSE((lower_1 == t.end() && lower_2 != s.end()) ||
                 (lower_1 != t.end() && lower_2 == s.end()));
    EXPECT_TRUE((lower_1 == t.end() && lower_2 == s.end()) ||
                *lower_1 == *lower_2);
  }
}

TEST(splay_order_test, test_10) {
  throttle::splay_order_set<int> t{};

  for (int i = 0; i < 262144; i++) {
    int temp = rand();
    if (!t.contains(temp))
      t.insert(temp);
  }

  ASSERT_TRUE(std::is_sorted(t.begin(), t.end()));

  t.erase(*t.max());
  t.erase(*t.min());

  ASSERT_TRUE(std::is_sorted(t.begin(), t.end()));
  EXPECT_EQ(validate_size_helper(t.m_tree_impl.m_root), true);
  auto it1 = t.min(), next = it1;
  for (int i = 0; i < 1000; ++i) {
    next = std::next(it1);
    t.erase(*it1++);
    EXPECT_EQ(next, t.min());
  }

  auto it2 = t.max(), prev = it2;
  for (int i = 0; i < 1000; ++i) {
    prev = std::prev(it2);
    t.erase(*it2--);
    EXPECT_EQ(prev, t.max());
  }

  for (int i = 0; i < 1000; ++i) {
    t.erase(*t.select_rank(rand() % t.size()));
  }

  EXPECT_EQ(validate_size_helper(t.m_tree_impl.m_root), true);
}

TEST(splay_order_test, test_11) {
  throttle::splay_order_set<int> t{};

  for (int i = 0; i < 65536; i++) {
    int temp = rand();
    if (!t.contains(temp))
      t.insert(temp);
  }

  t.erase(*t.max());
  t.erase(*t.min());

  EXPECT_EQ(validate_size_helper(t.m_tree_impl.m_root), true);

  throttle::splay_order_set<int> c{t};

  ASSERT_EQ(t.size(), c.size());
  for (auto const &v : t) {
    EXPECT_TRUE(c.contains(v));
  }

  auto vs = t.min();
  std::advance(vs, 1024);
  t.erase(vs, t.end());

  EXPECT_EQ(t.size(), 1024);
  EXPECT_EQ(validate_size_helper(t.m_tree_impl.m_root), true);
  EXPECT_EQ(validate_size_helper(c.m_tree_impl.m_root), true);

  c = t;
  ASSERT_EQ(t.size(), c.size());
  for (auto const &v : t) {
    EXPECT_TRUE(c.contains(v));
  }

  EXPECT_EQ(validate_size_helper(c.m_tree_impl.m_root), true);
}

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
