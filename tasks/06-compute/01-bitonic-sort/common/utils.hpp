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

#include "opencl_include.hpp"

#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace clutils {

template <typename T> auto create_random_number_generator(T lower, T upper) {
  std::random_device rnd_device;
  std::mt19937 mersenne_engine{rnd_device()};

  if constexpr (std::is_floating_point_v<T>) {
    std::uniform_real_distribution<T> dist{lower, upper};
    return [dist, mersenne_engine](auto &vec) mutable {
      std::generate(vec.begin(), vec.end(),
                    [&]() { return dist(mersenne_engine); });
    };
  } else {
    std::uniform_int_distribution<T> dist{lower, upper};
    return [dist, mersenne_engine](auto &vec) mutable {
      std::generate(vec.begin(), vec.end(),
                    [&]() { return dist(mersenne_engine); });
    };
  }
}

template <typename... Ts>
std::string kernel_define(std::string symbol, Ts... values) {
  std::stringstream ss;
  ss << "#define " << symbol << " ";
  ((ss << " " << values), ...);
  ss << "\n";
  return ss.str();
}

template <class T> inline std::size_t sizeof_container(const T &container) {
  return sizeof(typename T::value_type) * container.size();
}

struct profiling_info {
  std::chrono::milliseconds pure, wall;
};

} // namespace clutils
