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

#include <array>
#include <cstddef>
#include <utility>

#include <range/v3/action.hpp>
#include <range/v3/algorithm.hpp>
#include <range/v3/iterator.hpp>
#include <range/v3/numeric.hpp>
#include <range/v3/view.hpp>

namespace throttle {

template <typename T, std::size_t N> struct point_n {
private:
  std::array<T, N> arr;

public:
  using value_type = T;

  static constexpr std::size_t dimension = N;
  static point_n origin() { return point_n{}; }

  T &operator[](std::size_t index) { return arr[index]; }
  const T &operator[](std::size_t index) const { return arr[index]; }

  auto begin() { return arr.begin(); }
  auto end() { return arr.end(); }
  auto begin() const { return arr.begin(); }
  auto end() const { return arr.end(); }
  auto cbegin() const { return arr.cbegin(); }
  auto cend() const { return arr.cend(); }

  bool operator==(const point_n &other) const {
    return ranges::equal(arr, other.arr);
  }
  bool operator!=(const point_n &other) const { return !(*this == other); }
};

template <typename T, std::size_t N>
T distance_sq(const point_n<T, N> &lhs, const point_n<T, N> &rhs) {
  auto dist_range = ranges::views::transform(lhs, rhs, std::minus<T>{});
  return ranges::inner_product(dist_range, dist_range, T{0});
}

template <typename T, std::size_t N, typename t_stream>
t_stream &operator>>(t_stream &istream, point_n<T, N> &rhs) {
  std::copy_n(std::istream_iterator<T>{istream}, N, rhs.begin());
  return istream;
}

template <typename T, std::size_t N>
auto compute_closest(auto &&point_range, const point_n<T, N> &point) {
  auto closest =
      ranges::min_element(point_range, std::less{}, [point](auto &&elem) {
        return distance_sq(point, elem);
      });
  return std::make_pair(distance_sq(*closest, point), *closest);
}

} // namespace throttle
