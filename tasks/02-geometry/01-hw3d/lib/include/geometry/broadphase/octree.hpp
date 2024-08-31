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

#include "broadphase_structure.hpp"
#include "geometry/narrowphase/aabb.hpp"
#include "geometry/narrowphase/collision_shape.hpp"

#include "geometry/equal.hpp"
#include "geometry/point3.hpp"
#include "geometry/vec3.hpp"

#include <algorithm>
#include <array>
#include <queue>
#include <set>
#include <stack>
#include <vector>

namespace throttle {
namespace geometry {

template <typename T, typename t_shape = collision_shape<T>,
          typename =
              std::enable_if_t<std::is_base_of_v<collision_shape<T>, t_shape>>>
class octree : public broadphase_structure<octree<T, t_shape>, t_shape> {
  using shape_ptr = t_shape *;
  using point_type = point3<T>;
  using vec_type = vec3<T>;

public:
  using shape_type = t_shape;

private:
  std::vector<shape_type> m_stored_shapes;
  std::vector<t_shape> m_waiting_queue;

  unsigned m_max_depth;
  T m_min_cell_size_half;

  std::optional<T> m_max_coord, m_min_coord;
  struct octree_node {
    point_type m_center;
    T m_halfwidth;
    std::array<unsigned, 8> m_children;
    std::vector<unsigned> m_contained_shape_indexes;

    octree_node(point_type center, T halfwidth)
        : m_center{center}, m_halfwidth{halfwidth}, m_children{},
          m_contained_shape_indexes{} {}
  };

  unsigned root_index() const { return 1; }

  std::vector<octree_node> m_nodes;

  void insert_shape_impl(unsigned root_index, const shape_type &shape,
                         unsigned shape_pos) {
    unsigned index = 0;
    bool straddling = false;

    auto bbox = shape.bounding_box();
    octree_node &curr_node = m_nodes[root_index];

    for (unsigned i = 0; i < 3; ++i) {
      T delta = bbox.m_center[i] - curr_node.m_center[i];

      if (bbox.intersect_coodrinate_plane(i, curr_node.m_center[i])) {
        straddling = true;
        break;
      }

      if (delta > T{0})
        index |= (1 << i);
    }

    if (!straddling) {
      if (curr_node.m_children[index])
        return insert_shape_impl(curr_node.m_children[index], shape, shape_pos);
      curr_node.m_contained_shape_indexes.push_back(shape_pos);
      return;
    }

    else {
      curr_node.m_contained_shape_indexes.push_back(shape_pos);
    }
  }

  void insert_shape(const shape_type &shape) {
    unsigned new_shape_index = m_stored_shapes.size();
    m_stored_shapes.emplace_back(shape);
    insert_shape_impl(root_index(), shape, new_shape_index);
  }

  unsigned build_subtree(point_type center, T halfwidth, unsigned stop) {
    unsigned index = m_nodes.size();
    m_nodes.emplace_back(center, halfwidth);

    // Check if stop conditions are met.
    if (!stop || (halfwidth < m_min_cell_size_half))
      return index;

    vec_type offset = vec_type::zero();
    T step = halfwidth * T{0.5f};
    for (unsigned i = 0; i < 8; ++i) {
      offset.x = ((i & 1) ? step : -step);
      offset.y = ((i & 2) ? step : -step);
      offset.z = ((i & 4) ? step : -step);
      m_nodes[index].m_children[i] =
          build_subtree(center + offset, step, stop - 1);
    }

    return index;
  }

  void preconstruct() {
    // Clear all existing nodes and add sentinel value at index 0
    m_nodes.clear();
    m_nodes.emplace_back(point_type::origin(), T{0});

    // Guard against empty octree
    if (!m_max_coord)
      return;

    // Calculate halfwidth and center
    auto center_coord = (m_max_coord.value() + m_min_coord.value()) / T{2};
    auto center = point_type{center_coord, center_coord, center_coord};
    auto halfwidth = (m_max_coord.value() - m_min_coord.value()) / T{2};

    build_subtree(center, halfwidth, m_max_depth);
  }

  void flush_waiting() {
    while (!m_waiting_queue.empty()) {
      insert_shape(m_waiting_queue.back());
      m_waiting_queue.pop_back();
    }
  }

public:
  static constexpr auto default_min_cell_size = T{1.0e-4f};
  octree(unsigned depth, T min_cell_size = default_min_cell_size)
      : m_max_depth{depth}, m_min_cell_size_half{min_cell_size / T{2}} {
    m_nodes.emplace_back(point_type::origin(), T{0});
  }

  void add_collision_shape(const shape_type &shape) {
    auto bbox = shape.bounding_box();
    m_waiting_queue.push_back(shape);

    point_type min_point = bbox.minimum_corner(),
               max_point = bbox.maximum_corner();
    if (!m_max_coord) {
      m_min_coord = vmin(min_point.x, min_point.y, min_point.z);
      m_max_coord = vmax(max_point.x, max_point.y, max_point.z);
      return;
    }

    m_min_coord =
        vmin(m_min_coord.value(), min_point.x, min_point.y, min_point.z);
    m_max_coord =
        vmax(m_max_coord.value(), max_point.x, max_point.y, max_point.z);
  }

  void rebuid() {
    if (!m_waiting_queue.size())
      return;

    // Copy shapes from stored buffer to waiting queue.
    std::transform(m_stored_shapes.begin(), m_stored_shapes.end(),
                   std::back_inserter(m_waiting_queue),
                   [](const auto &elem) { return elem; });
    m_stored_shapes.clear();

    preconstruct();
    flush_waiting(); // Flush the waiting queue and move shapes to the stored
                     // buffer.
  }

private:
  struct many_to_many_collider {
    std::vector<unsigned> ancestor_stack;
    const octree &tree;
    std::set<unsigned> in_collision;

    many_to_many_collider(const octree &p_tree) : tree{p_tree} {
      ancestor_stack.reserve(tree.m_max_depth);
    }

    void collide(unsigned current_node) {
      ancestor_stack.push_back(current_node);

      for (unsigned n = 0; n < ancestor_stack.size(); ++n) {
        for (const auto &i_a :
             tree.m_nodes[ancestor_stack[n]].m_contained_shape_indexes) {
          for (const auto &i_b :
               tree.m_nodes[current_node].m_contained_shape_indexes) {
            if (i_a == i_b)
              break;

            if (tree.m_stored_shapes[i_a].collide(tree.m_stored_shapes[i_b])) {
              in_collision.insert(i_a);
              in_collision.insert(i_b);
            }
          }
        }
      }

      for (const auto &c : tree.m_nodes[current_node].m_children) {
        if (c)
          collide(c);
      }

      ancestor_stack.pop_back();
    }
  };

public:
  auto begin() const { return m_nodes.cbegin(); }
  auto end() const { return m_nodes.cend(); }
  auto cbegin() const { return m_nodes.cbegin(); }
  auto cend() const { return m_nodes.cend(); }

  bool empty() const {
    return m_stored_shapes.empty() && m_waiting_queue.empty();
  }

  std::vector<shape_ptr> many_to_many() {
    if (empty())
      return {};

    rebuid();

    many_to_many_collider collider{*this};
    collider.collide(root_index());

    std::vector<shape_ptr> result;
    std::transform(collider.in_collision.begin(), collider.in_collision.end(),
                   std::back_inserter(result), [&](const auto &elem) {
                     return std::addressof(m_stored_shapes[elem]);
                   });

    return result;
  }
};

} // namespace geometry
} // namespace throttle
