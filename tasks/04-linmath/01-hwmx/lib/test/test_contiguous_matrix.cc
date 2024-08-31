#include "contiguous_matrix.hpp"

#include <gtest/gtest.h>
#include <iostream>
#include <vector>

using matrix = throttle::linmath::contiguous_matrix<float>;

TEST(test_contiguous_matrix, test_zero) {
  matrix a = matrix::zero(9, 8);
  for (int i = 0; i < 9; i++)
    for (int j = 0; j < 8; j++)
      EXPECT_EQ(a[i][j], 0.0);
}

TEST(test_contiguous_matrix, test_unity) {
  const matrix a = matrix::unity(10);
  for (int i = 0; i < 10; i++)
    EXPECT_EQ(a[i][i], 1.0);
}

TEST(test_contiguous_matrix, test_sq_braces_1) {
  const matrix a = matrix::unity(10);
  EXPECT_EQ(a[0][0], 1);
}

TEST(test_contiguous_matrix, test_arithmetic) {
  const matrix a = matrix::unity(2);
  const matrix b = {2, 2, {1, 2, 3, 4}};
  EXPECT_EQ(a + b, matrix(2, 2, {2, 2, 3, 5}));
  EXPECT_EQ(b - a, matrix(2, 2, {0, 2, 3, 3}));
  EXPECT_EQ(a / 2.0f, matrix(2, 2, {0.5, 0, 0, 0.5}));
}

TEST(test_contiguous_matrix, test_mult_eq) {
  matrix a = matrix::unity(10);
  a *= 666;
  for (int i = 0; i < 10; i++)
    EXPECT_EQ(a[i][i], 666);
}

TEST(test_contiguous_matrix, test_dev_eq) {
  matrix a = matrix::unity(10);
  a *= 100;
  a /= 5;
  for (int i = 0; i < 10; i++)
    EXPECT_EQ(a[i][i], 20);
}

TEST(test_contiguous_matrix, test_multiplication) {
  matrix a = matrix::unity(10);
  matrix b = a * 666.0f;
  for (int i = 0; i < 10; i++)
    EXPECT_EQ(b[i][i], 666);
}

TEST(test_contiguous_matrix, test_devision) {
  matrix a = matrix::unity(10);
  a *= 100;
  matrix b = a / 5.0f;
  for (int i = 0; i < 10; i++)
    EXPECT_EQ(b[i][i], 20);
}

TEST(test_contiguous_matrix, test_transponse_method_1) {
  matrix a(4, 7, 0);
  a.transpose();
  EXPECT_EQ(a.cols(), 4);
  EXPECT_EQ(a.rows(), 7);
}

TEST(test_contiguous_matrix, test_transponse_method_2) {
  std::vector vals{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

  matrix a(4, 3, vals.begin(), vals.end());

  auto it = vals.begin();

  for (unsigned i = 0; i < 4; i++)
    for (unsigned j = 0; j < 3; j++) {
      EXPECT_EQ(a[i][j], *it);
      ++it;
    }

  matrix b(3, 4, {1, 4, 7, 10, 2, 5, 8, 11, 3, 6, 9, 12});
  a.transpose();

  EXPECT_TRUE(a == b);
}

TEST(test_contiguous_matrix, test_transponse) {
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

TEST(test_contiguous_matrix, test_multiplication_1) {
  matrix A(2, 2, {2, -3, 4, -6});
  matrix B(2, 2, {9, -6, 6, -4});

  EXPECT_TRUE(A * B == matrix::zero(2, 2));
}

TEST(test_contiguous_matrix, test_multiplication_2) {
  matrix A{3, 3, {5, 8, -4, 6, 9, -5, 4, 7, -3}};
  matrix B{3, 1, {2, -3, 1}};

  EXPECT_TRUE(A * B == matrix(3, 1, {-18, -20, -16}));
}

TEST(test_contiguous_matrix, test_multiplication_3) {
  matrix A{3, 3, {5, 8, -4, 6, 9, -5, 4, 7, -3}};
  matrix B{3, 1, {2, -3, 1}};

  auto C = A * B;
  EXPECT_TRUE(C == matrix(3, 1, {-18, -20, -16}));
  EXPECT_TRUE(A != C);
}
