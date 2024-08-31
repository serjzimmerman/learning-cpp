/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

/* This is an optimized version of disjoint map forest without an unordered_map
 * and elements are addressed with the index that they have been inserted at
 * with append_set(). This optimization cuts run time for offline_rmq in half
 * and drastically reduces memory usage.
 *
 */

#pragma once

#include <cstddef>
#include <functional>
#include <stack>
#include <unordered_map>
#include <utility>
#include <vector>

#include "disjoint_map_forest.hpp"
namespace throttle {

template <typename t_value_type> class indexed_disjoint_map final {
  using node_type = typename detail::disjoint_map_forest_node<t_value_type>;

public:
  using value_type = t_value_type;
  using size_type = typename node_type::size_type;
  using key_type = size_type;

private:
  std::vector<node_type> m_node_vec;
  std::stack<size_type> m_path_stack;

public:
  indexed_disjoint_map() = default;

  class individual_set_proxy {
    friend class indexed_disjoint_map;

    using pointer = value_type *;
    using reference = value_type &;

    size_type m_curr_index;
    indexed_disjoint_map *m_map;

  public:
    individual_set_proxy(size_type m_node, indexed_disjoint_map *p_map)
        : m_curr_index{m_node}, m_map{p_map} {}
    reference operator*() { return m_map->at_index(m_curr_index).m_val; }

    pointer operator->() { return &(m_map->at_index(m_curr_index).m_val); }
  };

  void append_set(const value_type &p_val) {
    size_type index = m_node_vec.size();
    m_node_vec.emplace_back(index, p_val);
  }

private:
  node_type &at_index(size_type p_index) { return m_node_vec.at(p_index); }

  size_type &parent_index(size_type p_node) {
    return at_index(p_node).m_parent_index;
  }

  size_type find_set_impl(size_type p_node) {
#ifndef RECURSIVE_FIND_SET
    size_type &parent = parent_index(p_node);

    while (parent != p_node) {
      m_path_stack.push(p_node);
      p_node = parent;
      parent = parent_index(p_node);
    }

    while (!m_path_stack.empty()) {
      parent_index(m_path_stack.top()) = p_node;
      m_path_stack.pop();
    }

#else
    size_type &parent = parent_index(p_node);
    if (parent != p_node)
      parent = find_set_impl(parent);
#endif

    return parent;
  }

  void link(size_type p_left, size_type p_right) {
    node_type &left = at_index(p_left), right = at_index(p_right);
    if (left.m_rank > right.m_rank)
      right.m_parent_index = p_left;

    else {
      left.m_parent_index = p_right;
      if (left.m_rank == right.m_rank)
        ++right.m_rank;
    }
  }

public:
  individual_set_proxy find_set(const key_type p_key) {
    return individual_set_proxy{find_set_impl(p_key), this};
  }

  void union_set(const key_type p_left, const key_type p_right) {
    size_type left = find_set_impl(p_left), right = find_set_impl(p_right);
    if (p_left != p_right)
      link(left, right);
  }

  template <typename t_stream> void dump(t_stream &p_ostream) {
    p_ostream << "digraph {\n";
    for (size_type i = 0; i < m_node_vec.size(); ++i) {
      p_ostream << "\tnode_" << i << " [label = \"" << i << ":"
                << at_index(i).m_val << "\"];\n";
      p_ostream << "\tnode_" << i << " -> node_" << at_index(i).m_parent_index
                << ";\n";
    }
    p_ostream << "}\n";
  }
};

template <typename t_stream, typename t_value_type>
t_stream &operator<<(t_stream &p_ostream,
                     indexed_disjoint_map<t_value_type> &p_set) {
  p_set.dump(p_ostream);
  return p_ostream;
}

} // namespace throttle
