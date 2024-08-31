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

#include "equal.hpp"

namespace throttle {
namespace geometry {

template <typename T> struct segment1 {
private:
  T a;
  T b;

public:
  segment1(T p_a, T p_b) : a{std::min(p_a, p_b)}, b{std::max(p_a, p_b)} {}
  static segment1 unity() { return segment1{0, 1}; }

  T len() const { return b - a; }

  bool intersect(const segment1<T> &other) const {
    if (is_roughly_greater_eq(a, other.a) && is_roughly_less_eq(a, other.b))
      return true;
    if (is_roughly_greater_eq(other.a, a) && is_roughly_less_eq(other.a, b))
      return true;
    return false;
  }

  bool contains(const segment1<T> &other) const {
    return is_roughly_less_eq(a, other.a) && is_roughly_greater_eq(b, other.b);
  }

  bool contains(T point) const {
    return is_roughly_greater_eq(point, a) && is_roughly_less_eq(point, b);
  }

  T left() const { return a; }
  T right() const { return b; }
};

} // namespace geometry
} // namespace throttle
