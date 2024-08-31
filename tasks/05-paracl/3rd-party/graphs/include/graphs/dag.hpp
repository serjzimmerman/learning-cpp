/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "directed_graph.hpp"

#include <cassert>

namespace graphs {

template <typename T, std::invocable<T> hash_t = std::hash<T>>
using dag = basic_directed_graph<T, void, void>;

} // namespace graphs
