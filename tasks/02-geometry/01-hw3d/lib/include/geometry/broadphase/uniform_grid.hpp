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
#include "geometry/equal.hpp"
#include "geometry/narrowphase/collision_shape.hpp"
#include "geometry/point3.hpp"
#include "geometry/vec3.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <list>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <boost/functional/hash.hpp>

namespace throttle {
namespace geometry {

namespace detail {
template <typename T> vec3<int> convert_to_int_vector(const vec3<T> &vec) {
  return vec3<int>{static_cast<int>(std::floor(vec.x)),
                   static_cast<int>(std::floor(vec.y)),
                   static_cast<int>(std::floor(vec.z))};
}
} // namespace detail

template <typename T, typename t_shape = collision_shape<T>,
          typename =
              std::enable_if_t<std::is_base_of_v<collision_shape<T>, t_shape>>>
class uniform_grid
    : public broadphase_structure<uniform_grid<T, t_shape>, t_shape> {
  using shape_ptr = t_shape *;

  using int_point_type = point3<int>;
  using int_vector_type = vec3<int>;

  using point_type = point3<T>;
  using vector_type = vec3<T>;

  using index_t = unsigned;
  using cell_type = int_vector_type;

  T m_cell_size = T{0};                 // grid's cells size
  std::vector<t_shape> m_waiting_queue; // queue of shapes to insert

  // The vector of inserted elements and vectors from all the cells that the
  // element overlaps
  using stored_shapes_elem_t = typename std::pair<t_shape, cell_type>;
  std::vector<stored_shapes_elem_t> m_stored_shapes;

  using shape_idx_vec_t =
      typename std::vector<index_t>; // A list of indexes of shapes in
                                     // m_stored_shapes

  struct cell_hash {
    std::size_t operator()(const cell_type &cell) const {
      std::size_t seed{};

      boost::hash_combine(seed, cell.x);
      boost::hash_combine(seed, cell.y);
      boost::hash_combine(seed, cell.z);

      return seed;
    }
  };

  using map_t =
      typename std::unordered_map<cell_type, shape_idx_vec_t,
                                  cell_hash>; // map cell into shape_list_t
  map_t m_map;

  std::optional<T> m_min_val,
      m_max_val; // minimum and maximum values of the bounding box coordinates

public:
  using shape_type = t_shape;

  uniform_grid(index_t number_hint) { // ctor with hint about the number of
                                      // shapes to insert
    m_waiting_queue.reserve(number_hint);
    m_stored_shapes.reserve(number_hint);
  }

  void add_collision_shape(const shape_type &shape) {
    m_waiting_queue.push_back(shape);

    auto bbox = shape.bounding_box();
    auto bbox_max_corner = bbox.maximum_corner();
    auto bbox_min_corner = bbox.minimum_corner();
    auto max_width = bbox.max_width();

    // remain cell size to be large enough to fit the largest shape in any
    // rotation
    if (max_width > m_cell_size)
      m_cell_size = max_width;

    if (!m_min_val) { // first insertion
      m_min_val = vmin(bbox_min_corner.x, bbox_min_corner.y, bbox_min_corner.z);
      m_max_val = vmax(bbox_max_corner.x, bbox_max_corner.y, bbox_max_corner.z);
      return;
    }

    m_min_val = vmin(m_min_val.value(), bbox_min_corner.x, bbox_min_corner.y,
                     bbox_min_corner.z);
    m_max_val = vmax(m_max_val.value(), bbox_max_corner.x, bbox_max_corner.y,
                     bbox_max_corner.z);
  }

  std::vector<shape_ptr> many_to_many() {
    rebuild();

    many_to_many_collider collider(m_map, m_stored_shapes);
    collider.collide();

    std::vector<shape_ptr> result;
    std::transform(collider.in_collision.begin(), collider.in_collision.end(),
                   std::back_inserter(result), [&](const auto &idx) {
                     return std::addressof(m_stored_shapes[idx].first);
                   });
    return result;
  }

  void flush_waiting() {
    while (
        !m_waiting_queue.empty()) { // insert all the new shapes into the grid
      insert(m_waiting_queue.back());
      m_waiting_queue.pop_back();
    }
  }

  void rebuild() {
    m_map.clear();

    std::transform(m_stored_shapes.begin(), m_stored_shapes.end(),
                   std::back_inserter(m_waiting_queue),
                   [&](const auto &pair) { return pair.first; });
    m_stored_shapes.clear();

    flush_waiting();
  }

  T cell_size() const { return m_cell_size; }

  auto begin() const { return m_stored_shapes.cbegin(); }
  auto end() const { return m_stored_shapes.cend(); }
  auto cbegin() const { return m_stored_shapes.cbegin(); }
  auto cend() const { return m_stored_shapes.cend(); }

private:
  void insert(const shape_type
                  &shape) { // insert shape into m_stored shapes and into m_map
    auto old_stored_size = m_stored_shapes.size();
    auto cell = compute_cell(shape);

    m_stored_shapes.emplace_back(shape, cell);
    m_map[cell].push_back(old_stored_size);
  }

  cell_type compute_cell(const shape_type &shape) const {
    auto bbox = shape.bounding_box();
    auto min_corner = bbox.m_center - point_type::origin();
    return detail::convert_to_int_vector(min_corner / m_cell_size);
  }

  static constexpr std::array<int_vector_type, 27> offsets() {
    std::array<int_vector_type, 27> result{};

    std::size_t index = 0;
    for (int i = -1; i <= 1; ++i) {
      for (int j = -1; j <= 1; ++j) {
        for (int k = -1; k <= 1; ++k) {
          result[index++] = {i, j, k};
        }
      }
    }

    return result;
  }

  struct many_to_many_collider {
    std::set<index_t> in_collision;
    map_t &map;
    const std::vector<stored_shapes_elem_t> &stored_shapes;

    many_to_many_collider(
        map_t &map_a, const std::vector<stored_shapes_elem_t> &stored_shapes_a)
        : map(map_a), stored_shapes(stored_shapes_a) {}

    void collide() { // fills "in_collision" set with all intersecting shapes in
                     // the grid
      for (auto &bucket : map) { // For each cell we will test
        auto offsets_a = offsets();
        for (auto &offset : offsets_a) { // all the neighbors
          auto bucket_to_test_with_it = map.find(bucket.first + offset);
          if (bucket_to_test_with_it !=
              map.end()) // for all of the shapes they containes to intersect
            for (auto to_test_idx : bucket.second)
              for (auto to_test_with_idx : bucket_to_test_with_it->second)
                if (to_test_idx != to_test_with_idx &&
                    stored_shapes[to_test_idx].first.collide(
                        stored_shapes[to_test_with_idx].first)) {
                  in_collision.insert(to_test_idx);
                  in_collision.insert(to_test_with_idx);
                }
        }
      }
    };
  };
};

} // namespace geometry
} // namespace throttle
