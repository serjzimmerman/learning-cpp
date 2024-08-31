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

#include <cmath>
#include <utility>

namespace throttle {
namespace geometry {

// clang-format off
template <typename T> struct vec2 {
  T x;
  T y;

  // Static pseudoconstructors.
  static vec2 zero() { return {0, 0}; }
  static vec2 axis_i() { return {1, 0}; }
  static vec2 axis_j() { return {0, 1}; }

  vec2 neg() const { return vec2{x * -1, y * -1}; }
  vec2 norm() const { T length = vec2::length(); return (length ? vec2{x / length, y / length} : *this); }
  vec2 perp() const { return vec2{-y, x}; }

  T length_sq() const { return dot(*this); }
  T length() const { return std::sqrt(length_sq()); }

  T dot(const vec2 &rhs) const { return x * rhs.x + y * rhs.y; }
  T perp_dot(const vec2 &rhs) const { return rhs.dot(perp()); }
  vec2 project(const vec2 &p_axis) { T length = p_axis.length_sq(); return ( length ? dot(p_axis) / length * p_axis : zero()); }
  std::pair<unsigned, T> max_component() const { return (std::abs(x) > std::abs(y) ? std::make_pair(0, std::abs(x)) : std::make_pair(1, std::abs(y))); }

  T &operator[](unsigned index) {
    switch (index) {
      case 0: return x;
      case 1: return y;
      default: throw std::out_of_range("Incorrect coordinate of vec2 was requested.");
    }
  }

  const T &operator[](unsigned index) const {
    switch (index) {
      case 0: return x;
      case 1: return y;
      default: throw std::out_of_range("Incorrect coordinate of vec2 was requested.");
    }
  }

  vec2 &operator+=(const vec2 &rhs) { x += rhs.x; y += rhs.y; return *this; }
  vec2 &operator-=(const vec2 &rhs) { return *this += rhs.neg(); }
  vec2 operator-() const { return neg(); }
  bool operator==(const vec2 &rhs) const { return (x == rhs.x && y == rhs.y); }
  bool operator!=(const vec2 &rhs) const { return !(*this == rhs); }
};
// clang-format on

} // namespace geometry
} // namespace throttle

#include "equal.hpp"
#include "point2.hpp"

namespace throttle {
namespace geometry {

template <typename T> T dot(vec2<T> lhs, vec2<T> rhs) { return lhs.dot(rhs); }
template <typename T> T perp_dot(vec2<T> lhs, vec2<T> rhs) {
  return lhs.perp_dot(rhs);
}

template <typename T>
vec2<T> operator+(const vec2<T> &lhs, const vec2<T> &rhs) {
  return vec2<T>{lhs} += rhs;
}
template <typename T>
vec2<T> operator-(const vec2<T> &lhs, const vec2<T> &rhs) {
  return vec2<T>{lhs} -= rhs;
}

template <typename T> vec2<T> operator*(const vec2<T> &lhs, T rhs) {
  return {lhs.x * rhs, lhs.y * rhs};
}
template <typename T> vec2<T> operator*(T lhs, const vec2<T> &rhs) {
  return {rhs.x * lhs, rhs.y * lhs};
}
template <typename T> vec2<T> operator/(const vec2<T> &lhs, T rhs) {
  return {lhs.x / rhs, lhs.y / rhs};
}

template <typename T>
bool colinear(
    const vec2<T> &lhs, const vec2<T> &rhs,
    T p_tolerance = ::throttle::geometry::default_precision<T>::m_prec) {
  return is_roughly_equal(lhs.x * rhs.y, lhs.y * rhs.x, p_tolerance);
}

template <typename T>
bool co_directional(
    const vec2<T> &lhs, const vec2<T> &rhs,
    T p_tolerance = ::throttle::geometry::default_precision<T>::m_prec) {
  return (colinear(lhs, rhs, p_tolerance) && dot(lhs, rhs) > 0);
}

template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
bool is_roughly_equal(vec2<T> p_first, vec2<T> p_second,
                      T p_precision = default_precision<T>::m_prec) {
  return is_roughly_equal(p_first.x, p_second.x, p_precision) &&
         is_roughly_equal(p_first.y, p_second.y, p_precision);
};

} // namespace geometry
} // namespace throttle
