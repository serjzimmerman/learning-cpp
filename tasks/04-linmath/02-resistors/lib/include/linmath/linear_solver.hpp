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

#include "datastructures/vector.hpp"
#include "linmath/matrix.hpp"

#include <algorithm>
#include <concepts>
#include <initializer_list>
#include <optional>

namespace throttle::linmath {

template <std::floating_point T> struct linear_equation final {
  using value_type = T;
  using size_type = std::size_t;

  std::vector<value_type> m_coefs;

  linear_equation(size_type variables) { m_coefs.resize(variables + 1); }
  linear_equation(const std::vector<value_type> &coefs)
      : m_coefs{coefs} {} // if coefs includes free coef
  linear_equation(std::initializer_list<value_type> list)
      : linear_equation{list.begin(), list.end()} {}

  template <std::input_iterator it> linear_equation(it start, it finish) {
    m_coefs.reserve(static_cast<size_type>(std::distance(start, finish)));
    std::copy(start, finish, std::back_inserter(m_coefs));
  }

  auto &operator[](size_type i) { return m_coefs[i]; }
  const auto &operator[](size_type i) const { return m_coefs[i]; }

  auto &free_coeff() { return m_coefs.back(); }
  const auto &free_coeff() const { return m_coefs.back(); }

  void reset() { std::fill(m_coefs.begin(), m_coefs.end(), 0); }

  size_type size() const { return m_coefs.size(); }
  size_type vars() const { return size() - 1; }

  auto begin() { return m_coefs.begin(); }
  auto end() { return m_coefs.end(); }
  auto begin() const { return m_coefs.cbegin(); }
  auto end() const { return m_coefs.cend(); }
  auto cbegin() const { return m_coefs.cbegin(); }
  auto cend() const { return m_coefs.cend(); }
};

template <std::floating_point T> class linear_equation_system final {
public:
  using value_type = T;
  using size_type = std::size_t;
  using equation_type = linear_equation<value_type>;

private:
  std::vector<equation_type> m_equations;

  mutable matrix<T> m_matrix;
  mutable bool m_changed_flag = true;

public:
  linear_equation_system() = default;
  linear_equation_system(const size_type size) { m_equations.reserve(size); }
  linear_equation_system(const std::vector<equation_type> m_equations)
      : m_equations{m_equations} {}
  linear_equation_system(std::initializer_list<equation_type> list)
      : linear_equation_system{list.begin(), list.end()} {}

  template <std::input_iterator it>
  linear_equation_system(it start, it finish) {
    m_equations.reserve(static_cast<size_type>(std::distance(start, finish)));
    std::copy(start, finish, std::back_inserter(m_equations));
  }

  void push(const equation_type &eq) {
    m_equations.push_back(eq);
    m_changed_flag = true;
  }

  auto operator[](size_type i) const { return m_equations[i]; }

  auto begin() const { return m_equations.cbegin(); }
  auto end() const { return m_equations.cend(); }
  auto cbegin() const { return m_equations.cbegin(); }
  auto cend() const { return m_equations.cend(); }

  size_type size() const { return m_equations.size(); }
  size_type vars() const {
    auto comp = [](auto a, auto b) { return a.size() < b.size(); };
    return std::max_element(m_equations.begin(), m_equations.end(), comp)
        ->vars();
  }

  matrix<value_type> get_xtnd_matrix() const {
    if (!m_changed_flag)
      return m_matrix;

    auto rows = size(), cols = vars() + 1;
    matrix<value_type> xtnd_matrix{rows, cols};

    for (unsigned i = 0; i < rows; ++i) {
      std::copy(m_equations[i].begin(), m_equations[i].end(),
                xtnd_matrix[i].begin());
    }

    m_changed_flag = false;
    return (m_matrix = xtnd_matrix);
  }

  std::optional<matrix<value_type>> solve() const {
    auto xtnd_matrix = get_xtnd_matrix();
    auto cols = xtnd_matrix.cols();
    auto rows = xtnd_matrix.rows();

    xtnd_matrix.convert_to_row_echelon();

    matrix<value_type> res{cols - 1, 1};
    for (size_type i = 0; i < cols - 1; i++) {
      if (is_roughly_equal(xtnd_matrix[i][i], 0.0))
        return std::nullopt;
      res[i][0] = xtnd_matrix[i][cols - 1] / xtnd_matrix[i][i];
    }

    for (size_type i = cols - 1; i < rows; i++) {
      if (!is_roughly_equal(xtnd_matrix[i][cols - 1], 0.0))
        return std::nullopt;
    }

    return res;
  }
};

} // namespace throttle::linmath
