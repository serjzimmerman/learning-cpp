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

#include <optional>

namespace circuits {

struct network_edge {
  unsigned first, second;
  double res;
  std::optional<double> emf = std::nullopt;

  network_edge(unsigned f = 0, unsigned s = 0, double r = 0,
               std::optional<double> e = std::nullopt)
      : first{f}, second{s}, res{r}, emf{e} {}
};

} // namespace circuits
