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

#include "broadphase_structure.hpp"
#include "geometry/narrowphase/collision_shape.hpp"

#include <algorithm>
#include <set>
#include <vector>

namespace throttle {
namespace geometry {

template <typename T, typename t_shape = collision_shape<T>,
          typename =
              std::enable_if_t<std::is_base_of_v<collision_shape<T>, t_shape>>>
class bruteforce
    : public broadphase_structure<bruteforce<T, t_shape>, t_shape> {
  using shape_ptr = t_shape *;
  std::vector<t_shape> m_stored_shapes;

public:
  using shape_type = t_shape;

  bruteforce() = default;
  bruteforce(unsigned number_hint) { m_stored_shapes.reserve(number_hint); }

  void add_collision_shape(const shape_type &shape) {
    m_stored_shapes.push_back(shape);
  }
  void rebuid() { return; }

  std::vector<shape_ptr> many_to_many() {
    std::set<unsigned> in_collision;
    unsigned size = m_stored_shapes.size();

    for (unsigned i = 0; i < size; ++i) {
      for (unsigned j = i + 1; j < size; ++j) {
        if (m_stored_shapes[i].collide(m_stored_shapes[j])) {
          in_collision.insert(i);
          in_collision.insert(j);
        }
      }
    }

    std::vector<shape_ptr> result;
    std::transform(in_collision.begin(), in_collision.end(),
                   std::back_inserter(result), [&](const auto &elem) {
                     return std::addressof(m_stored_shapes[elem]);
                   });
    return result;
  }
};

} // namespace geometry
} // namespace throttle
