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

#include <concepts>
#include <fstream>
#include <string_view>

namespace utils {

// clang-format off
template <typename T>
concept is_ifstream = requires (T stream) {
  { [] <typename CharType, typename Traits> (std::basic_ifstream<CharType, Traits> &) {} (stream) };
};

template <typename T>
concept is_ofstream = requires (T stream) {
  { [] <typename CharType, typename Traits> (std::basic_ofstream<CharType, Traits> &) {} (stream) };
};

template <typename T>
concept is_fstream = requires (std::remove_cvref_t<T> stream) {
  requires is_ifstream<T> || is_ofstream<T>;
};
// clang-format on

template <typename T>
concept integral_or_floating = std::integral<T> || std::floating_point<T>;

// clang-format off
template <typename T, typename D>
concept coercible_to = requires(T object) {
  { static_cast<D>(object) };
};
// clang-format on

// clang-format off
template <typename T>
concept convertible_to_string_view = coercible_to<T, std::string_view>;

} // namespace utils