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

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

namespace ezvk::utils {

inline std::vector<char> read_file(std::string filename) {
  std::ifstream file;

  file.exceptions(file.exceptions() | std::ifstream::failbit |
                  std::ifstream::badbit);
  file.open(filename, std::ios::binary);

  std::istreambuf_iterator<char> start(file), fin;
  return std::vector<char>(start, fin);
}

inline std::string trim_leading_trailing_spaces(std::string input) {
  if (input.empty())
    return {};
  auto pos_first = input.find_first_not_of(" \t\n");
  auto pos_last = input.find_last_not_of(" \t\n");
  return input.substr(pos_first != std::string::npos ? pos_first : 0,
                      (pos_last - pos_first) + 1);
}

template <typename T> auto sizeof_container(const T &container) {
  return sizeof(typename T::value_type) * container.size();
}

std::vector<const char *> to_c_strings(auto start, auto finish) {
  std::vector<const char *> result;
  for (; start != finish; ++start) {
    result.push_back(start->c_str());
  }
  return result;
}

}; // namespace ezvk::utils
