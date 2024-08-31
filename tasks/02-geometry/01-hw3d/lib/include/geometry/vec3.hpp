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
template <typename T> struct vec3 {
  T x;
  T y;
  T z;

  // Static pseudoconstructors.
  static vec3 zero() { return {0, 0, 0}; }
  static vec3 axis_i() { return {1, 0, 0}; }
  static vec3 axis_j() { return {0, 1, 0}; }
  static vec3 axis_k() { return {0, 0, 1}; }

  vec3 neg() const { return vec3{x * -1, y * -1, z * -1}; }
  vec3 norm() const { T length = vec3::length(); return (length ? vec3{x / length, y / length, z / length} : *this); }

  T length_sq() const { return dot(*this); }
  T length() const { return std::sqrt(length_sq()); }

  T dot(const vec3 &rhs) const { return x * rhs.x + y * rhs.y + z * rhs.z; }
  vec3 cross(const vec3 rhs) const { return {y * rhs.z - z * rhs.y, 
                                            -(x * rhs.z - z * rhs.x), 
                                            x * rhs.y - y * rhs.x}; }

  vec3 project(const vec3 &p_axis) { T length = p_axis.length_sq(); return ( length ? dot(p_axis) / length * p_axis : zero()); }

  std::pair<unsigned, T> max_component() const {
    unsigned index = 0; T max = std::abs(x);
    if (std::abs(y) > max) { max = std::abs(y); index = 1; }
    if (std::abs(z) > max) { max = std::abs(z); index = 2; }
    return std::make_pair(index, max);
  }

  T &operator[](unsigned index) {
    switch (index) {
    case 0: return x;
    case 1: return y;
    case 2: return z;
    default: throw std::out_of_range("Incorrect coordinate of vec3 was requested.");
    }
  }

  const T &operator[](unsigned index) const {
    switch (index) {
    case 0: return x;
    case 1: return y;
    case 2: return z;
    default: throw std::out_of_range("Incorrect coordinate of vec3 was requested.");
    }
  }

  vec3 &operator+=(const vec3 &rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
  vec3 &operator-=(const vec3 &rhs) { return *this += rhs.neg(); }
  vec3 operator-() const { return neg(); }
  bool operator==(const vec3 &rhs) const { return (x == rhs.x && y == rhs.y && z == rhs.z); }
  bool operator!=(const vec3 &rhs) const { return !(*this == rhs); }
};
// clang-format on

} // namespace geometry
} // namespace throttle

#include "equal.hpp"
#include "point3.hpp"

namespace throttle {
namespace geometry {

template <typename T> T dot(vec3<T> lhs, vec3<T> rhs) { return lhs.dot(rhs); }
template <typename T> vec3<T> cross(vec3<T> lhs, vec3<T> rhs) {
  return lhs.cross(rhs);
}
template <typename T> T triple_product(vec3<T> a, vec3<T> b, vec3<T> c) {
  return dot(a, cross(b, c));
}

template <typename T>
vec3<T> operator+(const vec3<T> &lhs, const vec3<T> &rhs) {
  return vec3<T>{lhs} += rhs;
}
template <typename T>
vec3<T> operator-(const vec3<T> &lhs, const vec3<T> &rhs) {
  return vec3<T>{lhs} -= rhs;
}

template <typename T> vec3<T> operator*(const vec3<T> &lhs, T rhs) {
  return {lhs.x * rhs, lhs.y * rhs, lhs.z * rhs};
}
template <typename T> vec3<T> operator*(T lhs, const vec3<T> &rhs) {
  return {rhs.x * lhs, rhs.y * lhs, rhs.z * lhs};
}
template <typename T> vec3<T> operator/(const vec3<T> &lhs, T rhs) {
  return {lhs.x / rhs, lhs.y / rhs, lhs.z / rhs};
}

template <typename T>
bool colinear(
    const vec3<T> &lhs, const vec3<T> &rhs,
    T p_tolerance = ::throttle::geometry::default_precision<T>::m_prec) {
  return is_roughly_equal(cross(lhs, rhs), vec3<T>::zero(), p_tolerance);
}

template <typename T>
bool co_directional(
    const vec3<T> &lhs, const vec3<T> &rhs,
    T p_tolerance = ::throttle::geometry::default_precision<T>::m_prec) {
  return (colinear(lhs, rhs, p_tolerance) && dot(lhs, rhs) > 0);
}

template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
bool is_roughly_equal(vec3<T> p_first, vec3<T> p_second,
                      T p_precision = default_precision<T>::m_prec) {
  return is_roughly_equal(p_first.x, p_second.x, p_precision) &&
         is_roughly_equal(p_first.y, p_second.y, p_precision) &&
         is_roughly_equal(p_first.z, p_second.z, p_precision);
};

} // namespace geometry
} // namespace throttle
