/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "matrix.hpp"

#include <gtest/gtest.h>
#include <iostream>
#include <vector>

using matrix = typename throttle::linmath::matrix<float>;

TEST(test_matrix, test_ctor_1) {
  matrix A = matrix::zero(3, 4);
  for (unsigned i = 0; i < 3; i++)
    for (unsigned j = 0; j < 4; j++)
      EXPECT_EQ(A[i][j], 0.0f);
}

TEST(test_matrix, test_ctor_2) {
  matrix A = matrix::unity(10);
  for (unsigned i = 0; i < 10; i++)
    EXPECT_EQ(A[i][i], 1.0f);
}

TEST(test_matrix, test_ctor_3) {
  std::vector vec{1, 2, 3, 4};
  matrix A = matrix::diag(3, vec.begin(), vec.end());
  matrix B{3, 3, {1, 0, 0, 0, 2, 0, 0, 0, 3}};
  EXPECT_TRUE(A == B);
}

TEST(test_matrix, test_ctor_4) {
  std::vector vec{1, 2};
  matrix A = matrix::diag(3, vec.begin(), vec.end());
  matrix B{3, 3, {1, 0, 0, 0, 2, 0, 0, 0, 0}};
  EXPECT_TRUE(A == B);
}

TEST(test_matrix, test_transponse) {
  std::vector vals{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

  matrix a(4, 3, vals.begin(), vals.end());

  auto it = vals.begin();

  for (unsigned i = 0; i < 4; i++)
    for (unsigned j = 0; j < 3; j++) {
      EXPECT_EQ(a[i][j], *it);
      ++it;
    }

  matrix b{3, 4, {1, 4, 7, 10, 2, 5, 8, 11, 3, 6, 9, 12}};
  auto c = transpose(a);

  EXPECT_TRUE(c == b);
  EXPECT_TRUE(c != a);
}

TEST(test_matrix, test_sum_1) {
  matrix A{3, 3, {-1, 2, 3, -3, 1, 4, 5, 3, 2}};
  matrix B{3, 3, {1, -2, -3, 3, -1, -4, -5, -3, -2}};
  matrix C = A + B;
  EXPECT_EQ(C, matrix::zero(3, 3));
}

TEST(test_matrix, test_sum_2) {
  matrix A{3, 3, {-1, 2, 3, -3, 1, 4, 5, 3, 2}};
  matrix B{3, 2, {1, -2, -3, 3, -1, -4}};
  EXPECT_THROW(A + B, std::runtime_error);
}

TEST(test_matrix, test_dif_1) {
  matrix A{3, 3, {-1, 2, 3, -3, 1, 4, 5, 3, 2}};
  matrix B{3, 3, {-1, 2, 3, -3, 1, 4, 5, 3, 2}};
  matrix C = A - B;
  EXPECT_EQ(C, matrix::zero(3, 3));
}

TEST(test_matrix, test_multiplication_1) {
  matrix A(2, 2, {2, -3, 4, -6});
  matrix B(2, 2, {9, -6, 6, -4});

  A *= B;

  EXPECT_TRUE(matrix::zero(2, 2) == matrix::zero(2, 2));
}

TEST(test_matrix, test_multiplication_2) {
  matrix A{3, 3, {5, 8, -4, 6, 9, -5, 4, 7, -3}};
  matrix B{3, 1, {2, -3, 1}};

  A *= B;
  EXPECT_TRUE(A == matrix(3, 1, {-18, -20, -16}));
}

TEST(test_matrix, test_multiplication_3) {
  matrix A{3, 3, {5, 8, -4, 6, 9, -5, 4, 7, -3}};
  matrix B{3, 1, {2, -3, 1}};

  auto C = A * B;
  EXPECT_TRUE(C == matrix(3, 1, {-18, -20, -16}));
  EXPECT_TRUE(A != C);
}

TEST(test_matrix, test_multiplication_4) {
  matrix A{2, 3, {1, 2, 3, 4, 5, 6}};
  matrix B{3, 2, {7, 8, 9, 10, 11, 12}};

  auto C = A * B;
  EXPECT_EQ(C, matrix(2, 2, {58, 64, 139, 154}));
}

TEST(test_matrix, test_max_in_row) {
  std::vector vals{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
  matrix a(4, 3, vals.begin(), vals.end());
  EXPECT_EQ(a.max_in_col(2).first, 3);
  EXPECT_EQ(a.max_in_col(2).second, 12);
}

TEST(test_matrix, test_gauss_jordan_elimination_1) {
  matrix A{3, 2, {1, 2, 3, 4, 5, 6}};
  A.convert_to_row_echelon();

  matrix B{3, 2, {5, 0, 0, 0.8, 0, 0}};
  EXPECT_TRUE(A == B);
}

TEST(test_matrix, test_determinant_for_fields_1) {
  matrix A{2, 2, {1, 0, 0, 1}};
  EXPECT_EQ(A.determinant(), 1);
}

TEST(test_matrix, test_determinant_for_fields_2) {
  matrix A{3, 3, {1, 3, 2, -3, -1, -3, 2, 3, 1}};
  EXPECT_EQ(A.determinant(), -15);
}

TEST(test_matrix, test_determinant_for_fields_3) {
  matrix A{3, 4, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}};
  EXPECT_THROW(A.determinant(), std::runtime_error);
}
