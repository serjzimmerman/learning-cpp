/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "detail/rb_tree_ranged.hpp"
#include <functional>
#include <initializer_list>

namespace throttle {
template <typename T, typename t_comp = std::less<T>>
class order_statistic_set : public detail::rb_tree_ranged_<T, t_comp> {
public:
  order_statistic_set() : detail::rb_tree_ranged_<T, t_comp>{} {}

  order_statistic_set(std::initializer_list<T> p_list)
      : detail::rb_tree_ranged_<T, t_comp>{} {
    this->insert_range(p_list.begin(), p_list.end());
  }
};
} // namespace throttle
