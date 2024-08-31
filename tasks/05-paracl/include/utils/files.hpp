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

#include "utils/algorithm.hpp"
#include "utils/concepts.hpp"

#include <fmt/format.h>

#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <optional>
#include <span>
#include <sstream>

namespace utils {

namespace detail {

inline auto get_iterator_endian(std::span<char> raw_bytes) {
  static_assert(std::endian::native == std::endian::little ||
                    std::endian::native == std::endian::big,
                "Mixed endian, bailing out");
  if constexpr (std::endian::native == std::endian::little) {
    return raw_bytes.begin();
  } else {
    return raw_bytes.rbegin();
  }
}

} // namespace detail

template <integral_or_floating T, std::input_iterator iter>
std::pair<std::optional<T>, iter> read_little_endian(iter first, iter last) {
  std::array<char, sizeof(T)> raw_bytes;

  const std::input_iterator auto input_iter =
      detail::get_iterator_endian(raw_bytes);
  auto size = sizeof(T);
  first = copy_while(first, last, input_iter,
                     [&size](auto) { return size && size--; });

  if (size != 0)
    return std::pair{std::nullopt, first};
  return std::pair{std::bit_cast<T>(raw_bytes), first};
}

template <integral_or_floating T>
void write_little_endian(T val, std::output_iterator<char> auto oput) {
  std::array<char, sizeof(T)> raw_bytes =
      std::bit_cast<decltype(raw_bytes)>(val);
  const std::input_iterator auto input_iter =
      detail::get_iterator_endian(raw_bytes);
  auto size = sizeof(T);
  std::copy_n(input_iter, size, oput);
}

struct padded_hex {
  auto &operator()(auto &os, std::integral auto val,
                   std::size_t padding = 8) const {
    const auto format_string = fmt::format("{{:#0{}x}}", padding + 2);
    return os << fmt::vformat(format_string, fmt::make_format_args(val));
  }
};

constexpr padded_hex padded_hex_printer;

void try_open_file(is_fstream auto &file, const std::filesystem::path &path,
                   std::ios_base::openmode mode) {
  file.exceptions(file.exceptions() | std::ios::badbit | std::ios::failbit);
  try {
    file.open(path, mode);
  } catch (std::exception &e) {
    throw std::runtime_error{
        fmt::format("Could not open file `{}`: ", path.string(), e.what())};
  }
}

inline std::string read_file(const std::filesystem::path &input_path) {
  std::ifstream ifs;
  try_open_file(ifs, input_path, std::ios::in);
  std::stringstream ss;
  ss << ifs.rdbuf();
  return ss.str();
}

} // namespace utils
