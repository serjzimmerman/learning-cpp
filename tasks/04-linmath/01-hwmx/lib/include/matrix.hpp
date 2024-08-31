/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "contiguous_matrix.hpp"
#include "equal.hpp"
#include "utility.hpp"

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>

#include <range/v3/action.hpp>
#include <range/v3/algorithm.hpp>
#include <range/v3/view.hpp>

namespace throttle {
namespace linmath {

template <typename T>
concept models_ordered_ring = requires(T a, T b) {
  requires models_ring<T>;
  requires std::totally_ordered<T>;
};

template <typename T>
  requires models_ordered_ring<T>
class matrix {
  using value_type = T;
  using reference = T &;
  using const_reference = const T &;
  using pointer = T *;
  using const_pointer = const T *;
  using size_type = typename std::size_t;

  contiguous_matrix<T> m_contiguous_matrix;
  containers::vector<pointer> m_rows_vec;

  void update_rows_vec() {
    m_rows_vec.reserve(rows());

    // Just messing around with range-v3. Nothing to see here.
    // clang-format off
    ranges::copy(
        ranges::views::ints(0, ranges::unreachable) 
        | ranges::views::stride(cols()) 
        | ranges::views::transform([start = m_contiguous_matrix.data()](auto value) { return start + value; })
        | ranges::views::take(rows()),
        std::back_inserter(m_rows_vec)); }
  // clang-format on

public:
  matrix(size_type rows, size_type cols, value_type val = value_type{})
      : m_contiguous_matrix{rows, cols, val} {
    update_rows_vec();
  }

  template <std::input_iterator it>
  matrix(size_type rows, size_type cols, it start, it finish)
      : m_contiguous_matrix{rows, cols, start, finish} {
    update_rows_vec();
  }

  matrix(size_type rows, size_type cols, std::initializer_list<value_type> list)
      : m_contiguous_matrix{rows, cols, list} {
    update_rows_vec();
  }

  matrix(contiguous_matrix<T> &&c_matrix)
      : m_contiguous_matrix(std::move(c_matrix)) {
    update_rows_vec();
  }

  static matrix zero(size_type rows, size_type cols) {
    return matrix<T>{rows, cols};
  }
  static matrix unity(size_type size) {
    return matrix{std::move(contiguous_matrix<T>::unity(size))};
  }

  template <std::input_iterator it>
  static matrix diag(size_type size, it start, it finish) {
    matrix ret{size, size, value_type{}};

    for (size_type i = 0; (i < size) && (start != finish); i++, start++) {
      ret[i][i] = *start;
    }

    return ret;
  }

private:
  class proxy_row {
    pointer m_row, m_past_row;

  public:
    proxy_row() = default;
    proxy_row(pointer row, size_type n_cols)
        : m_row{row}, m_past_row{m_row + n_cols} {}

    using iterator = utility::contiguous_iterator<value_type>;
    using const_iterator = utility::const_contiguous_iterator<value_type>;

    reference operator[](size_type index) { return m_row[index]; }
    const_reference operator[](size_type index) const { return m_row[index]; }

    iterator begin() { return iterator{m_row}; }
    iterator end() { return iterator{m_past_row}; }

    const_iterator begin() const { return const_iterator{m_row}; }
    const_iterator end() const { return const_iterator{m_past_row}; }
    const_iterator cbegin() const { return const_iterator{m_row}; }
    const_iterator cend() const { return const_iterator{m_past_row}; }

    size_type size() const { return m_past_row - m_row; }
  };

  class const_proxy_row {
    const_pointer m_row, m_past_row;

  public:
    const_proxy_row() = default;
    const_proxy_row(const_pointer row, size_type n_cols)
        : m_row{row}, m_past_row{m_row + n_cols} {}

    using iterator = utility::const_contiguous_iterator<value_type>;
    using const_iterator = iterator;

    const_reference operator[](size_type index) const { return m_row[index]; }
    iterator begin() const { return iterator{m_row}; }
    iterator end() const { return iterator{m_past_row}; }
    const_iterator cbegin() const { return const_iterator{m_row}; }
    const_iterator cend() const { return const_iterator{m_past_row}; }

    size_type size() const { return m_past_row - m_row; }
  };

public:
  proxy_row operator[](size_type index) {
    return proxy_row{m_rows_vec[index], cols()};
  }
  const_proxy_row operator[](size_type index) const {
    return const_proxy_row{m_rows_vec[index], cols()};
  }

  size_type rows() const { return m_contiguous_matrix.rows(); }
  size_type cols() const { return m_contiguous_matrix.cols(); }
  bool square() const { return (cols() == rows()); }

  bool equal(const matrix &other,
             const value_type &precision =
                 default_precision<value_type>::m_prec) const {
    if ((rows() != other.rows()) || (cols() != other.cols()))
      return false;
    for (size_type i = 0; i < m_rows_vec.size(); i++) {
      const auto first_row = (*this)[i];
      const auto second_row = other[i];
      if (!ranges::equal(first_row, second_row,
                         [&precision](auto first, auto second) {
                           return is_roughly_equal(first, second, precision);
                         }))
        return false;
    }
    return true;
  }

  matrix &transpose() {
    matrix transposed{cols(), rows()};

    for (size_type i = 0; i < rows(); i++) {
      for (size_type j = 0; j < cols(); j++) {
        transposed[j][i] = std::move((*this)[i][j]);
      }
    }

    *this = std::move(transposed);
    return *this;
  }

  void swap_rows(size_type idx1, size_type idx2) {
    std::swap(m_rows_vec[idx1], m_rows_vec[idx2]);
  }

public:
  std::pair<size_type, value_type>
  max_in_col_greater_eq(size_type col, size_type minimum_row) {
    size_type max_row_idx = minimum_row;
    auto rows = this->rows();
    auto cmp = std::less<value_type>{};

    for (size_type row = minimum_row; row < rows; row++) {
      max_row_idx =
          (cmp(std::abs((*this)[max_row_idx][col]), std::abs((*this)[row][col]))
               ? row
               : max_row_idx);
    }
    return std::make_pair(max_row_idx, (*this)[max_row_idx][col]);
  }

  std::pair<size_type, value_type> max_in_col(size_type col) {
    return max_in_col_greater_eq(col, 0);
  }

  std::optional<std::pair<size_type, value_type>>
  first_non_zero_in_col(size_type col, size_type start_row = 0) const {
    for (size_type m = start_row; m < rows(); ++m) {
      if ((*this)[m][col] == 0)
        continue;
      return std::make_pair(m, (*this)[m][col]);
    }
    return std::nullopt;
  }

public:
  std::optional<int> convert_to_row_echelon()
    requires std::is_floating_point_v<value_type>
  {
    matrix &mat = *this;
    int sign = 1;

    for (size_type i = 0; i < rows(); i++) {
      auto [pivot_row, pivot_elem] = max_in_col_greater_eq(i, i);

      if (pivot_elem == value_type{})
        return std::nullopt;

      if (i != pivot_row) {
        swap_rows(i, pivot_row);
        sign *= -1;
      }

      for (size_type to_elim_row = 0; to_elim_row < rows(); to_elim_row++) {
        if (i == to_elim_row)
          continue;

        auto first_row = mat[to_elim_row];
        auto second_row = mat[i];

        auto coef = mat[to_elim_row][i] / pivot_elem;
        ranges::transform(first_row, second_row, first_row.begin(),
                          [coef](value_type left, value_type right) {
                            return left - coef * right;
                          });
      }
    }

    return sign;
  }

  value_type determinant() const {
    if (!square())
      throw std::runtime_error("Mismatched matrix size for determinant");

    value_type sign = 1;
    auto size = rows();
    matrix mat{*this};

    for (size_type k = 0; k < size - 1; ++k) {
      auto result = mat.first_non_zero_in_col(k, k);
      if (!result)
        return 0;
      auto [pivot_row, pivot_elem] = result.value();

      if (k != pivot_row) {
        mat.swap_rows(k, pivot_row);
        sign *= -1;
      }

      for (size_type i = k + 1; i < size; ++i) {
        for (size_type j = k + 1; j < size; ++j) {
          mat[i][j] = mat[k][k] * mat[i][j] - mat[i][k] * mat[k][j];
          if (k == 0)
            continue;
          mat[i][j] /= mat[k - 1][k - 1];
        }
      }
    }

    return sign * mat[size - 1][size - 1];
  }

  value_type determinant() const
    requires std::is_floating_point_v<value_type>
  {
    if (!square())
      throw std::runtime_error("Mismatched matrix size for determinant");

    matrix tmp{*this};
    auto res = tmp.convert_to_row_echelon();
    if (!res)
      return value_type{};

    value_type val = res.value();
    for (size_type i = 0; i < rows(); i++) {
      val *= tmp[i][i];
    }

    return val;
  }

  matrix &operator*=(value_type rhs) {
    m_contiguous_matrix *= rhs;
    return *this;
  }

  matrix &operator/=(value_type rhs) {
    m_contiguous_matrix /= rhs;
    return *this;
  }

  matrix &operator+=(const matrix &other) {
    if (rows() != other.rows() || cols() != other.cols())
      throw std::runtime_error("Mismatched matrix sizes");
    for (size_type i = 0; i < rows(); ++i) {
      auto row_first = (*this)[i];
      auto row_second = other[i];
      ranges::transform(row_first, row_second, row_first.begin(),
                        std::plus<value_type>{});
    }
    return *this;
  }

  matrix &operator-=(const matrix &other) {
    if (rows() != other.rows() || cols() != other.cols())
      throw std::runtime_error("Mismatched matrix sizes");
    for (size_type i = 0; i < rows(); ++i) {
      auto row_first = (*this)[i];
      auto row_second = other[i];
      ranges::transform(row_first, row_second, row_first.begin(),
                        std::minus<value_type>{});
    }
    return *this;
  }

  matrix &operator*=(const matrix &rhs) {
    if (cols() != rhs.rows())
      throw std::runtime_error("Mismatched matrix sizes");

    matrix res{rows(), rhs.cols()}, t_rhs = rhs;
    t_rhs.transpose();

    for (size_type i = 0; i < rows(); i++) {
      for (size_type j = 0; j < t_rhs.rows(); j++) {
        const auto range_first = (*this)[i], range_second = t_rhs[j];
        res[i][j] = ranges::accumulate(
            ranges::views::zip_with(std::multiplies<value_type>{}, range_first,
                                    range_second),
            value_type{});
      }
    }

    std::swap(*this, res);
    return *this;
  }
};

// clang-format off
template <typename T> matrix<T> operator*(const matrix<T> &lhs, T rhs) { auto res = lhs; res *= rhs; return res; }
template <typename T> matrix<T> operator*(T lhs, const matrix<T> &rhs) { auto res = rhs; res *= lhs; return res; }

template <typename T> matrix<T> operator+(const matrix<T> &lhs, const matrix<T> &rhs) { auto res = lhs; res += rhs; return res; }
template <typename T> matrix<T> operator-(const matrix<T> &lhs, const matrix<T> &rhs) { auto res = lhs; res -= rhs; return res; }

template <typename T> matrix<T> operator*(const matrix<T> &lhs, const matrix<T> &rhs) { auto res = lhs; res *= rhs; return res; }
template <typename T> matrix<T> operator/(const matrix<T> &lhs, T rhs) { auto res = lhs; res /= rhs; return res; }

template <typename T> bool operator==(const matrix<T> &lhs, const matrix<T> &rhs) { return lhs.equal(rhs); }
template <typename T> bool operator!=(const matrix<T> &lhs, const matrix<T> &rhs) { return !(lhs.equal(rhs)); }

template <typename T> matrix<T> transpose(const matrix<T> &mat) { auto res = mat; res.transpose(); return res; }

// clang-format on

} // namespace linmath
} // namespace throttle
