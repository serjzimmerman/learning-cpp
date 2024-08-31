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

#include "point2.hpp"
#include <cmath>
#include <stdexcept>
#include <type_traits>

namespace throttle {
namespace geometry {

template <typename T> struct point3 {
  T x;
  T y;
  T z;

  using point_flat_type = point2<T>;
  static point3 origin() { return {0, 0, 0}; }
  std::pair<unsigned, T> max_component() const {
    return (*this - origin()).max_component();
  }

  T &operator[](unsigned index) {
    switch (index) {
    case 0:
      return x;
    case 1:
      return y;
    case 2:
      return z;
    default:
      throw std::out_of_range("Incorrect coordinate of vec3 was requested.");
    }
  }

  const T &operator[](unsigned index) const {
    switch (index) {
    case 0:
      return x;
    case 1:
      return y;
    case 2:
      return z;
    default:
      throw std::out_of_range("Incorrect coordinate of vec3 was requested.");
    }
  }

  point_flat_type project_coord(unsigned axis) const {
    switch (axis) {
    case 0:
      return point_flat_type{y, z}; // Project onto yz plane
    case 1:
      return point_flat_type{x, z}; // Project onto xz plane
    case 2:
      return point_flat_type{x, y}; // Project onto xy plane
    default:
      throw std::out_of_range(
          "Axis index for point projection is out of range.");
    }
  }

  bool operator==(const point3 &p_other) const {
    return (x == p_other.x && y == p_other.y && z == p_other.z);
  }
  bool operator!=(const point3 &p_other) const { return !(*this == p_other); }
};

} // namespace geometry
} // namespace throttle

#include "vec3.hpp"

namespace throttle {
namespace geometry {

template <typename T, typename... Ts,
          typename = std::enable_if_t<
              std::conjunction_v<std::is_same<point3<T>, Ts>...>>>
point3<T> barycentric_average(Ts... points) {
  return point3<T>{(... + points.x) / T{sizeof...(Ts)},
                   (... + points.y) / T{sizeof...(Ts)},
                   (... + points.z) / T{sizeof...(Ts)}};
}

template <typename T>
vec3<T> operator-(const point3<T> &lhs, const point3<T> &rhs) {
  return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

template <typename T>
point3<T> operator+(const point3<T> &lhs, const vec3<T> &rhs) {
  return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

template <typename T>
point3<T> operator+(const vec3<T> &lhs, const point3<T> &rhs) {
  return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
bool is_roughly_equal(point3<T> p_first, point3<T> p_second,
                      T p_precision = default_precision<T>::m_prec) {
  return is_roughly_equal(p_first.x, p_second.x, p_precision) &&
         is_roughly_equal(p_first.y, p_second.y, p_precision) &&
         is_roughly_equal(p_first.z, p_second.z, p_precision);
};

} // namespace geometry
} // namespace throttle
