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

#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
#include <iterator>
#include <optional>
#include <variant>

namespace utils {

template <typename... Ts> struct visitors : Ts... {
  using Ts::operator()...;
};

template <typename... Ts> visitors(Ts...) -> visitors<Ts...>;

template <typename t_tuple> struct variant_from_tuple;
template <typename... Ts> struct variant_from_tuple<std::tuple<Ts...>> {
  using type = std::variant<Ts...>;
};

template <typename t_tuple>
using variant_from_tuple_t = typename variant_from_tuple<t_tuple>::type;

template <typename t_tuple, typename... t_types> struct tuple_add_types {};
template <typename... t_tuple_types, typename... t_types>
struct tuple_add_types<std::tuple<t_tuple_types...>, t_types...> {
  using type = std::tuple<t_tuple_types..., t_types...>;
};

template <typename t_tuple, typename... t_types>
using tuple_add_types_t = typename tuple_add_types<t_tuple, t_types...>::type;

auto pointer_to_uintptr(auto *pointer) {
  return std::bit_cast<uintptr_t>(pointer);
}

} // namespace utils
