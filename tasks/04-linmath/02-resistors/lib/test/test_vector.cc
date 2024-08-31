/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "datastructures/vector.hpp"

#include <algorithm>
#include <gtest/gtest.h>
#include <memory>
#include <new>
#include <numeric>
#include <vector>

using vector = typename throttle::containers::vector<int>;
template class throttle::containers::vector<int>;

TEST(test_vector, test_ctor) {
  vector a;
  EXPECT_EQ(a.size(), 0);
}

TEST(test_vector, test_push_pop_back) {
  vector a;
  for (auto i = 0; i < 5; i++)
    a.push_back(i);

  EXPECT_EQ(a.size(), 5);

  for (auto i = 4; i >= 0; i--) {
    EXPECT_EQ(a.back(), i);
    a.pop_back();
  }

  EXPECT_EQ(a.size(), 0);
}

TEST(test_vector, test_copy_ctor) {
  vector a;
  for (auto i = 0; i < 5; i++)
    a.push_back(i);

  vector b = a;

  EXPECT_EQ(a.size(), b.size());

  for (auto i = 4; i >= 0; i--) {
    EXPECT_EQ(b.back(), i);
    EXPECT_EQ(a.back(), i);
    b.pop_back();
    a.pop_back();
  }
}

TEST(test_vector, test_reserve_1) {
  vector a;
  a.reserve(7);
  EXPECT_GE(a.capacity(), 7);
  EXPECT_EQ(a.size(), 0);
}

TEST(test_vector, test_reserve_2) {
  vector a;
  a.reserve(7);
  EXPECT_GE(a.capacity(), 7);
  EXPECT_EQ(a.size(), 0);
}

TEST(test_vector, test_reserve_3) {
  vector a;

  for (int i = 0; i < 5; i++)
    a.push_back(i);

  EXPECT_EQ(a.size(), 5);

  a.reserve(7);

  EXPECT_GE(a.capacity(), 7);
  EXPECT_EQ(a.size(), 5);
}

TEST(test_vector, test_resize_1) {
  std::vector<int> range{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  vector a{range.begin(), range.end()};
  range.resize(5);
  vector b{range.begin(), range.end()};
  a.resize(5);
  EXPECT_TRUE(std::equal(a.begin(), a.end(), b.begin()));
}

TEST(test_vector, test_resize_2) {
  std::vector<int> range{1, 2, 3, 4, 5};
  vector a{range.begin(), range.end()};
  range.resize(8);
  vector b{range.begin(), range.end()};
  a.resize(8);
  EXPECT_TRUE(std::equal(a.begin(), a.end(), b.begin()));
}

TEST(test_vector, test_iterator_1) {
  vector a;
  for (auto i = 0; i < 4096; i++)
    a.push_back(i);

  int i = 0;
  for (auto &elem : a) {
    EXPECT_EQ(elem, i);
    i++;
  }
}

TEST(test_vector, test_iterator_2) {
  vector a;
  for (auto i = 0; i < 10; i++)
    a.push_back(i);

  int i = 9;
  for (auto it = a.end() - 1; it != a.begin(); it--) {
    EXPECT_EQ(*it, i);
    i--;
  }
}

TEST(test_vector, test_iterator_3) {
  vector a;
  for (auto i = 0; i < 10; i++)
    a.push_back(i);

  auto it = a.begin();

  EXPECT_EQ(*std::next(it, 5), 5);
}

TEST(test_vector, test_iterator_4) {
  vector a;
  for (auto i = 0; i < 10; i++)
    a.push_back(i);

  auto it = a.end();

  EXPECT_EQ(*std::prev(it, 1), 9);
  EXPECT_EQ(*std::prev(it, 5), 5);
}

#if 0
TEST(test_vector, test_exception_reserve_resize) {
  vector a(1000);

  std::iota(a.begin(), a.end(), 0);

  auto old_cap = a.capacity();

  EXPECT_EQ(a.size(), 1000);
  EXPECT_GE(old_cap, 1000);

  EXPECT_THROW(a.reserve(1ul << 60), std::bad_alloc);
  EXPECT_EQ(a.size(), 1000);
  EXPECT_EQ(a.capacity(), old_cap);
}
#endif

TEST(test_vector, test_strings_1) {
  throttle::containers::vector<std::string> a{5, "Hello "};
  a.resize(10, "World!");

  EXPECT_EQ(a.size(), 10);

  for (unsigned i = 6; i < a.size(); ++i) {
    EXPECT_EQ(a[i], "World!");
  }
}

TEST(test_vector, test_strings_2) {
  throttle::containers::vector<std::string> a{5, "Hello "};
  a.resize(2);
}

TEST(test_vector, test_vec_uptr_vec) {
  throttle::containers::vector<std::unique_ptr<std::vector<int>>> vec_vec;

  for (int i = 0; i < 500; ++i) {
    vec_vec.emplace_back(std::make_unique<std::vector<int>>(i));
    std::iota(vec_vec.back()->begin(), vec_vec.back()->end(), 0);
  }

  for (int i = 0; i < vec_vec.size(); ++i) {
    std::vector<int> test(i);
    std::iota(test.begin(), test.end(), 0);
    EXPECT_TRUE(
        std::equal(vec_vec[i]->begin(), vec_vec[i]->end(), test.begin()));
  }
}

TEST(test_vector, range_constructor) {
  std::vector<int> iota_range(1000);
  std::iota(iota_range.begin(), iota_range.end(), 0);
  vector a{iota_range.begin(), iota_range.end()};

  EXPECT_TRUE(std::equal(a.begin(), a.end(), iota_range.begin()));
}

TEST(test_vector, vec_uptr) {
  throttle::containers::vector<std::unique_ptr<int>> vec;

  for (int i = 0; i < 100000; ++i) {
    vec.push_back(std::make_unique<int>(i));
  }

  for (int i = 0; i < 100000; ++i) {
    EXPECT_EQ(*vec.at(i), i);
  }
}
