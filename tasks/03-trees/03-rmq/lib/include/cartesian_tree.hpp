/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

/* This file contains implementation of base class for Cartesian tree. It's
 * derived from in cartesian_set.hpp to implement a sort of set with
 * pseudoiterators for traversing the tree. Append method constructs the tree in
 * iterative manner with O(n) complexity. This tree preserves the inorder
 * traversal to be same as the order that elements have been appended, but it
 * doesn't actually store all the keys, as that's a huge memory overhead. All
 * that is needed to construct the tree is a stack for the rightmost branch
 * (path that is taken from rightmost element to the root). When an element is
 * appended, this branch is traversed upwards while popping the keys from the
 * stack until an element less than inserted key is found. Then the branches are
 * "rotated" to preserve Cartesian tree property.
 *
 */

#pragma once

#include <cstddef>
#include <functional>
#include <stack>
#include <unordered_map>
#include <utility>
#include <vector>

namespace throttle {

namespace detail {
struct cartesian_tree_node {
  using size_type = unsigned;

  size_type m_parent;
  size_type m_left;
  size_type m_right;

  cartesian_tree_node() : m_parent{0}, m_left{0}, m_right{0} {}
};
} // namespace detail

template <typename t_value_type, typename t_comp = std::less<t_value_type>>
class cartesian_tree {
protected:
  using node_type = typename detail::cartesian_tree_node;

public:
  using size_type = node_type::size_type;
  using value_type = t_value_type;
  using comp = t_comp;

protected:
  std::vector<node_type> m_tree_vec;

  // The tree only has to keep around just a small number of keys at one time.
  // Specifically the rightmost branch. When appending an element the stack is
  // popped from while traversing to the root.
  std::stack<t_value_type> m_key_stack;

  size_type m_rightmost;
  size_type m_root;

  t_comp m_comp;

  node_type &at_index(size_type p_index) { return m_tree_vec.at(p_index); }

  const node_type &at_index(size_type p_index) const {
    return m_tree_vec.at(p_index);
  }

public:
  cartesian_tree()
      : m_tree_vec{}, m_key_stack{}, m_rightmost{0}, m_root{0}, m_comp{} {
    m_tree_vec.emplace_back(); // Sentinel value that is used to indicate that
                               // there is no parent, left or right node. It's
                               // otherwise unreachable and contains garbage;
  }

  bool empty() const { return !size(); }

  size_type size() const { return m_tree_vec.size() - 1; }

protected:
  void append_impl(const t_value_type &p_key) {
    bool is_empty = empty();
    m_tree_vec.emplace_back();

    size_type new_index = size();
    if (is_empty) {
      m_root = m_rightmost = new_index;
      m_key_stack.push(p_key);
      return;
    }

    size_type curr = m_rightmost;
    bool is_key_less = m_comp(p_key, m_key_stack.top());
    while (curr && is_key_less) {
      curr = at_index(curr).m_parent;
      if (curr) {
        m_key_stack.pop();
        auto val = m_key_stack.top();
        is_key_less = m_comp(p_key, val);
      }
    }

    m_key_stack.push(p_key);
    m_rightmost = new_index;
    node_type &rightmost_node = m_tree_vec.back();
    if (!curr) {
      rightmost_node.m_left = m_root;
      at_index(m_root).m_parent = new_index;
      m_root = m_rightmost;
      return;
    }

    node_type &curr_node = at_index(curr);
    rightmost_node.m_left = curr_node.m_right;
    at_index(curr_node.m_right).m_parent = m_rightmost;
    curr_node.m_right = m_rightmost;
    rightmost_node.m_parent = curr;
  }

public:
  template <typename t_stream> void dump(t_stream &p_ostream) const {
    p_ostream << "digraph {\n";
    for (size_type i = 1; i < m_tree_vec.size(); ++i) {
      p_ostream << "\tnode_" << i << " [label = \"" << i << "\"];\n";
      const node_type &node = at_index(i);

      if (node.m_left) {
        p_ostream << "\tnode_" << i << " -> node_" << node.m_left << ";\n";
      } else {
        p_ostream << "\tnode_" << i << " -> node_0_" << i << ";\n";
        p_ostream << "\tnode_0_" << i << " [label = \"NIL\"];\n";
      }

      if (node.m_right) {
        p_ostream << "\tnode_" << i << " -> node_" << node.m_right << ";\n";
      } else {
        p_ostream << "\tnode_" << i << " -> node_0__" << i << ";\n";
        p_ostream << "\tnode_0__" << i << " [label = \"NIL\"];\n";
      }
    }
    p_ostream << "}\n";
  }
};

template <typename t_stream, typename t_value_type, typename t_comp>
t_stream &operator<<(t_stream &p_ostream,
                     cartesian_tree<t_value_type, t_comp> &p_tree) {
  p_tree.dump(p_ostream);
  return p_ostream;
}

} // namespace throttle
