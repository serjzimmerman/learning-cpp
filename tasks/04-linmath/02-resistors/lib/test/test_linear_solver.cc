#include <gtest/gtest.h>
#include <iostream>

#include "datastructures/vector.hpp"
#include "linmath/linear_solver.hpp"
#include "linmath/matrix.hpp"

namespace linmath = throttle::linmath;

using linear_equation_d = typename throttle::linmath::linear_equation<double>;
using linear_equation_system_d =
    typename throttle::linmath::linear_equation_system<double>;

TEST(test_equation_system, test_1) {
  linear_equation_d eq1{{1, -1, 7}};
  linear_equation_d eq2{{3, 2, 16}};
  linear_equation_system_d eqsys{{eq1, eq2}};

  auto res = eqsys.solve();
  EXPECT_TRUE(res.has_value());
  linmath::matrix_d sol{2, 1, {6, -1}};
  EXPECT_EQ(res.value().cols(), sol.cols());
  EXPECT_EQ(res.value(), sol);
}

TEST(test_equation_system, test_2) {
  linear_equation_d eq1{{1, 1, 1, 6}};
  linear_equation_d eq2{{0, 2, 5, -4}};
  linear_equation_d eq3{{2, 5, -1, 27}};
  linear_equation_system_d eqsys{{eq1, eq2, eq3}};
  linmath::matrix_d sol{3, 1, {5, 3, -2}};
  auto res = eqsys.solve();
  EXPECT_EQ(res.value(), sol);
}

TEST(test_equation_system, test_3) {
  linear_equation_d eq1{{1, 1, 1, 6}};
  linear_equation_d eq2{{5, 2, 0, -4}};
  linear_equation_d eq3{{-1, 5, 2, 27}};
  linear_equation_system_d eqsys{{eq1, eq2, eq3}};
  linmath::matrix_d sol{3, 1, {-2, 3, 5}};
  auto res = eqsys.solve();
  EXPECT_EQ(res.value(), sol);
}

TEST(test_equation_system, test_push) {
  linear_equation_d eq1{{1, 1, 1, 6}};
  linear_equation_d eq2{{5, 2, 0, -4}};
  linear_equation_d eq3{{-1, 5, 2, 27}};
  linear_equation_system_d eqsys{};
  eqsys.push(eq1);
  eqsys.push(eq2);
  eqsys.push(eq3);
  linmath::matrix_d sol{3, 1, {-2, 3, 5}};
  auto res = eqsys.solve();
  EXPECT_EQ(res.value(), sol);
}
