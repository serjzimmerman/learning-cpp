/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "ezvis/ezvis.hpp"

#include <cstdlib>
#include <gtest/gtest.h>

namespace {

struct i_base : ezvis::visitable_base<i_base> {
  virtual ~i_base() {}
};

struct derived1 : i_base {
  static constexpr auto msg = "hello from derived1";
  EZVIS_VISITABLE();
};

struct derived2 : i_base {
  static constexpr auto msg = "hello from derived2";
  EZVIS_VISITABLE();
};

TEST(test_ezvis, test_lambda_visit) {
  std::unique_ptr<i_base> base;
  base = std::make_unique<derived1>();

  const auto get_str = [](const auto &base) {
    auto res = ezvis::visit<std::string, derived1, derived2>(
        [](const auto &type) {
          return std::remove_reference_t<decltype(type)>::msg;
        },
        *base.get());
    return res;
  };

  auto res = get_str(base);
  EXPECT_EQ(res, derived1::msg);

  base = std::make_unique<derived2>();
  res = get_str(base);
  EXPECT_EQ(res, derived2::msg);
}

} // namespace
