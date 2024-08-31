/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "bs_order_tree.hpp"
#include <stdexcept>
#include <tuple>

namespace throttle {
namespace detail {
template <typename t_value_type, typename t_comp, typename t_key_type>
class splay_order_tree
    : public bs_order_tree<t_value_type, t_comp, t_key_type> {
  using base_tree = bs_order_tree<t_value_type, t_comp, t_key_type>;
  using typename base_tree::base_ptr;
  using typename base_tree::const_base_ptr;
  using typename base_tree::const_node_ptr;
  using typename base_tree::link_type;
  using typename base_tree::node_ptr;
  using self = splay_order_tree;

public:
  using typename base_tree::iterator;
  using typename base_tree::size_type;

protected:
  void splay_to_root(base_ptr p_node) const {
    while (p_node->m_parent) {

      if (!p_node->m_parent->m_parent) { // Case 1. Zig
        this->rotate_to_parent(p_node);
      }

      else if (p_node->is_linear()) { // Case 2. Zig-zig
        this->rotate_to_parent(p_node->m_parent);
        this->rotate_to_parent(p_node);
      }

      else { // Case 3. Zig-zag
        this->rotate_to_parent(p_node);
        this->rotate_to_parent(p_node);
      }
    }
  }

  void join(base_ptr p_left, base_ptr p_right) {
    assert(p_left);
    assert(p_right);

    base_ptr max_left = p_left->maximum();

    splay_to_root(max_left);
    max_left->m_right = p_right;
    p_right->m_parent = max_left;
    this->m_root = max_left;

    max_left->m_size = link_type::size(max_left->m_left) +
                       link_type::size(max_left->m_right) + 1;
  }

  void erase(base_ptr p_node) {
    base_ptr to_erase = p_node;
    assert(to_erase);

    if (this->size() == 1) {
      this->m_root = this->m_leftmost = this->m_rightmost = nullptr;
      delete static_cast<node_ptr>(to_erase);
      return;
    }

    // Move the node to the root. Depending on the number of children of the new
    // root we should modify leftmost and rightmost pointers.
    splay_to_root(to_erase);
    if (!to_erase->m_left) {
      this->m_leftmost = to_erase->successor();
      this->m_root = to_erase->m_right;
      to_erase->m_right->m_parent = nullptr;
    }

    else if (!to_erase->m_right) {
      this->m_rightmost = to_erase->predecessor();
      this->m_root = to_erase->m_left;
      to_erase->m_left->m_parent = nullptr;
    }

    else {
      to_erase->m_right->m_parent = nullptr;
      to_erase->m_left->m_parent = nullptr;
      join(to_erase->m_left, to_erase->m_right);
    }

    delete static_cast<node_ptr>(to_erase);
  }

  size_type get_rank_of(base_ptr p_node) const {
    base_ptr node = p_node;
    assert(node);

    size_type rank = link_type::size(node->m_left) + 1;
    while (node != this->m_root) {
      if (node->is_right_child())
        rank += link_type::size(node->m_parent->m_left) + 1;
      node = node->m_parent;
    }

    splay_to_root(node);
    return rank;
  }

public:
  void insert(const t_value_type &p_val) {
    auto [inserted, prev] = this->bst_insert(p_val);
    splay_to_root(inserted ? inserted : prev);
    if (!inserted)
      throw std::out_of_range("Double insert");
  }

  void erase(const t_key_type &p_key) {
    auto [to_erase, prev] = this->bst_lookup(p_key);
    if (!to_erase) {
      if (prev)
        splay_to_root(prev);
      throw std::out_of_range(
          "Trying to erase element not present in the tree");
    }
    erase(to_erase);
  }

  void erase(iterator p_pos) { erase(p_pos.m_curr); }

  size_type get_rank_of(const t_key_type &p_elem) const {
    base_ptr node, prev;
    std::tie(node, prev) = this->bst_lookup(p_elem);
    if (!node) {
      if (prev)
        splay_to_root(prev);
      throw std::out_of_range("Element not present");
    }

    return get_rank_of(node);
  }

  size_type get_rank_of(iterator p_pos) const {
    return get_rank_of(p_pos.m_curr);
  }

  iterator select_rank(size_type p_rank) const {
    if (p_rank > this->size() || !(p_rank > 0))
      return this->end();

    base_ptr curr = this->m_root;
    size_type r = link_type::size(curr->m_left) + 1;
    while (r != p_rank) {
      if (p_rank < r) {
        curr = curr->m_left;
      } else {
        curr = curr->m_right;
        p_rank -= r;
      }
      r = link_type::size(curr->m_left) + 1;
    }

    splay_to_root(curr);
    return iterator{curr, this};
  }

  iterator lower_bound(const t_key_type &p_key) const {
    base_ptr curr = this->m_root, prev = nullptr, bound = nullptr;

    while (curr) {
      prev = curr;
      bool is_curr_less = t_comp{}(static_cast<node_ptr>(curr)->m_value, p_key);
      if (is_curr_less) {
        curr = curr->m_right;
      } else {
        bound = curr;
        curr = curr->m_left;
      }
    }

    if (!bound) {
      if (prev)
        splay_to_root(prev);
      return this->end();
    }

    splay_to_root(bound);
    return iterator{bound, this};
  }

  iterator upper_bound(const t_key_type &p_key) const {
    base_ptr curr = this->m_root, prev = nullptr, bound = nullptr;

    while (curr) {
      prev = curr;
      bool is_key_less = t_comp{}(p_key, static_cast<node_ptr>(curr)->m_value);
      if (is_key_less) {
        bound = curr;
        curr = curr->m_left;
      } else {
        curr = curr->m_right;
      }
    }

    if (!bound) {
      if (prev)
        splay_to_root(prev);
      return this->end();
    }

    splay_to_root(bound);
    return iterator{bound, this};
  }

  iterator find(const t_key_type &p_key) const {
    auto [found, prev] = this->bst_lookup(p_key);
    if (!found) {
      if (prev)
        splay_to_root(prev);
      return this->end();
    }
    splay_to_root(found);
    return iterator{found, this};
  }

  // Use default constructor, destructor and move constructor, assigment from
  // base class.
  splay_order_tree() = default;
  ~splay_order_tree() = default;
  splay_order_tree(self &&) = default;
  self &operator=(self &&) = default;

  // Traverse in post-order to improve perfomance. If a were to traverse in
  // inorder, then inserted elements would be sorted and the copy would be a
  // degenerate linked list.
  splay_order_tree(const self &p_other) : base_tree{} {
    self temp{};
    this->traverse_postorder(
        static_cast<const_node_ptr>(p_other.m_root), [&](const_base_ptr p_n) {
          temp.insert(static_cast<const_node_ptr>(p_n)->m_value);
        });
    *this = std::move(temp);
  }

  // Define copy assignment in terms of copy constructor.
  self &operator=(const self &p_other) {
    if (this != &p_other) {
      self temp{p_other};
      *this = std::move(temp);
    }
    return *this;
  }

public:
};
} // namespace detail
} // namespace throttle
