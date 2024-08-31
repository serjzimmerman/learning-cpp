/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include <cmath>
#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

namespace throttle {

template <typename T> T vmin(const T &a) { return a; }

template <
    typename T, typename... Ts,
    typename = std::enable_if_t<std::conjunction_v<std::is_same<T, Ts>...>>>
T vmin(const T &a, const T &b, Ts... args) {
  return ((a > b) ? vmin(b, args...) : vmin(a, args...));
}

template <typename T> T vmax(const T &a) { return a; }

template <
    typename T, typename... Ts,
    typename = std::enable_if_t<std::conjunction_v<std::is_same<T, Ts>...>>>
T vmax(const T &a, const T &b, Ts... args) {
  return ((a < b) ? vmax(b, args...) : vmax(a, args...));
}

// Precision to be used for floating point comparisons
template <typename T> struct default_precision {
  static constexpr T m_prec = 1.0e-6f;
};

template <typename T> bool is_roughly_equal(T p_first, T p_second, T) {
  return p_first == p_second;
};

template <std::floating_point T>
bool is_roughly_equal(T p_first, T p_second,
                      T p_precision = default_precision<T>::m_prec) {
  using std::abs;
  using std::max;
  T epsilon = p_precision;
  return (abs(p_first - p_second) <=
          epsilon * vmax(abs(p_first), abs(p_second), T{1}));
};

template <std::floating_point T>
bool is_roughly_greater_eq(T p_first, T p_second,
                           T p_precision = default_precision<T>::m_prec) {
  using std::abs;
  using std::max;
  T epsilon = p_precision;
  return (p_first - p_second >=
          -epsilon * vmax(abs(p_first), abs(p_second), T{1}));
};

template <std::floating_point T>
bool is_definitely_less(T p_first, T p_second,
                        T p_precision = default_precision<T>::m_prec) {
  return !(is_roughly_greater_eq(p_first, p_second, p_precision));
};

template <std::floating_point T>
bool is_roughly_less_eq(T p_first, T p_second,
                        T p_precision = default_precision<T>::m_prec) {
  using std::abs;
  using std::max;
  T epsilon = p_precision;
  return (p_first - p_second <=
          epsilon * vmax(abs(p_first), abs(p_second), T{1}));
};

template <std::floating_point T>
bool is_definitely_greater(T p_first, T p_second,
                           T p_precision = default_precision<T>::m_prec) {
  return !(is_roughly_less_eq(p_first, p_second, p_precision));
};

template <typename... Ts, typename = std::enable_if_t<std::conjunction_v<
                              std::is_convertible<bool, Ts>...>>>
bool are_all_true(Ts... args) {
  return (... && args);
}

template <typename... Ts> bool are_all_roughly_zero(Ts... args) {
  return are_all_true((is_roughly_equal<Ts>(args, 0))...);
}

template <typename... Ts> bool are_same_sign(Ts... args) {
  return (are_all_true(std::greater{}(args, 0)...) ||
          are_all_true(std::less{}(args, 0)...));
}

} // namespace throttle
