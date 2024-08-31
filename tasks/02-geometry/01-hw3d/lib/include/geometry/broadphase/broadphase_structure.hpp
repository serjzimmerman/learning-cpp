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

#include "geometry/narrowphase/collision_shape.hpp"
#include <type_traits>
#include <vector>

namespace throttle {
namespace geometry {

template <typename t_derived, typename shape_type> class broadphase_structure {
  using derived_ref = t_derived &;
  using shape_ptr = shape_type *;

public:
  void add_collision_shape(const shape_type &shape) {
    impl().add_collision_shape(shape);
  }
  derived_ref impl() { return static_cast<derived_ref>(*this); }
  void rebuild() { impl().rebuild(); }
  std::vector<shape_ptr> many_to_many() { return impl().many_to_many(); }
};

} // namespace geometry
} // namespace throttle
