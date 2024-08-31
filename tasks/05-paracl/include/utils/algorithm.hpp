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
#include <iterator>

namespace utils {

template <std::input_iterator input_it>
input_it copy_while(
    input_it first, input_it last,
    std::output_iterator<
        typename std::iterator_traits<input_it>::value_type> auto o_first,
    std::invocable<typename std::iterator_traits<input_it>::value_type> auto
        pred) {
  for (; first != last; ++first) {
    if (!pred(*first))
      break;
    *o_first = *first;
    ++o_first;
  }
  return first;
}

} // namespace utils
