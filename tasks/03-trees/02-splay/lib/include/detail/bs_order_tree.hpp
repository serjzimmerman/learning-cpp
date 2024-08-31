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

#include <cassert>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <memory>
#include <tuple>
#include <type_traits>

namespace throttle {
namespace detail {

struct bst_order_node_base {
  using base_ptr = bst_order_node_base *;
  using const_base_ptr = const bst_order_node_base *;
  using size_type = std::size_t;

  size_type m_size;
  base_ptr m_left;
  base_ptr m_right;
  base_ptr m_parent;

  static size_type size(base_ptr p_x) noexcept {
    return (p_x ? p_x->m_size : 0);
  }

  base_ptr minimum() noexcept {
    base_ptr curr = this;
    while (curr->m_left)
      curr = curr->m_left;
    return curr;
  }

  base_ptr maximum() noexcept {
    base_ptr curr = this;
    while (curr->m_right)
      curr = curr->m_right;
    return curr;
  }

  base_ptr successor() noexcept {
    if (m_right)
      return (m_right)->minimum();
    return nullptr;
  }

  base_ptr predecessor() noexcept {
    if (m_left)
      return (m_left)->maximum();
    return nullptr;
  }

  base_ptr inorder_successor() noexcept {
    base_ptr curr = this;

    if (curr->m_right) {
      curr = curr->successor();
    }

    else {
      base_ptr parent = curr->m_parent;
      while (parent && curr->is_right_child()) {
        curr = parent;
        parent = parent->m_parent;
      }
      curr = parent;
    }

    return curr;
  }

  base_ptr inorder_predecessor() noexcept {
    base_ptr curr = this;

    if (curr->m_left) {
      curr = curr->predecessor();
    }

    else {
      base_ptr parent = curr->m_parent;
      while (parent && curr->is_left_child()) {
        curr = parent;
        parent = parent->m_parent;
      }
      curr = parent;
    }

    return curr;
  }

  base_ptr get_sibling() noexcept {
    return (is_left_child() ? m_parent->m_right : m_parent->m_left);
  }

  base_ptr get_uncle() noexcept {
    return (m_parent->is_right_child() ? m_parent->m_parent->m_left
                                       : m_parent->m_parent->m_right);
  }

  bool is_left_child() const noexcept { return (this == m_parent->m_left); }

  bool is_right_child() const noexcept { return (this == m_parent->m_right); }

  bool is_linear() const noexcept {
    return ((is_left_child() && m_parent->is_left_child()) ||
            (is_right_child() && m_parent->is_right_child()));
  }
};

template <typename t_value_type>
struct bst_order_node : public bst_order_node_base {
  using node_type = bst_order_node<t_value_type>;
  using node_ptr = node_type *;
  using const_node_ptr = const node_type *;

  t_value_type m_value;

  bst_order_node() : bst_order_node_base{}, m_value{} {}
  bst_order_node(const t_value_type &p_key, size_type p_size = 1)
      : bst_order_node_base{p_size, nullptr, nullptr, nullptr}, m_value{p_key} {
  }
};

class bs_order_tree_impl {
protected:
  using base_ptr = bst_order_node_base::base_ptr;
  using const_base_ptr = bst_order_node_base::const_base_ptr;
  using link_type = bst_order_node_base;
  using size_type = bst_order_node_base::size_type;

protected:
  mutable base_ptr m_root;
  // Note that rotations don't change min and max values, only erase and insert
  // does, thus pointers don't change as well.
  base_ptr m_leftmost;
  base_ptr m_rightmost;

  void rotate_left(base_ptr) const noexcept;
  void rotate_right(base_ptr) const noexcept;
  void rotate_to_parent(base_ptr) const noexcept;

  bs_order_tree_impl() : m_root{}, m_leftmost{}, m_rightmost{} {}
};

template <typename t_value_type, typename t_comp,
          typename t_key_type = t_value_type>
class bs_order_tree : public bs_order_tree_impl {
protected:
  using node_type = bst_order_node<t_value_type>;
  using node_ptr = typename node_type::node_ptr;
  using const_node_ptr = typename node_type::const_node_ptr;
  using size_type = typename node_type::size_type;
  using self = bs_order_tree;

public:
  struct iterator {
    const self *m_tree;
    base_ptr m_curr;

    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = ptrdiff_t;
    using value_type = t_value_type;
    using pointer = t_value_type *;
    using reference = t_value_type &;

    iterator() : m_tree{}, m_curr{} {}
    iterator(base_ptr p_node, const self *p_tree)
        : m_tree{p_tree}, m_curr{p_node} {}

    pointer get() { return &(static_cast<node_ptr>(m_curr)->m_value); }

    reference operator*() const {
      return static_cast<node_ptr>(m_curr)->m_value;
    }

    pointer operator->() { return get(); }

    iterator &operator++() {
      m_curr = m_curr->inorder_successor();
      return *this;
    }

    iterator operator++(int) {
      iterator temp{*this};
      ++(*this);
      return temp;
    }

    iterator &operator--() {
      m_curr = (m_curr ? m_curr->inorder_predecessor() : m_tree->m_rightmost);
      return *this;
    }

    iterator operator--(int) {
      iterator temp{*this};
      --(*this);
      return temp;
    }

    bool operator==(const iterator &p_rhs) const {
      return (m_curr == p_rhs.m_curr);
    }

    bool operator!=(const iterator &p_rhs) const {
      return (m_curr != p_rhs.m_curr);
    }
  };

  // Helper selectors
  iterator begin() const noexcept { return iterator{m_leftmost, this}; }

  iterator end() const noexcept { return iterator{nullptr, this}; }

  bool empty() const noexcept { return !m_root; }

  size_type size() const noexcept { return (m_root ? m_root->m_size : 0); }

public: // Modifiers
  void clear() {
    base_ptr curr = m_root;
    while (curr) {
      if (curr->m_left)
        curr = curr->m_left;
      else if (curr->m_right)
        curr = curr->m_right;

      else {
        base_ptr parent = curr->m_parent;
        if (parent) {
          if (curr->is_left_child())
            parent->m_left = nullptr;
          else
            parent->m_right = nullptr;
        }
        delete static_cast<node_ptr>(curr);
        curr = parent;
      }
    }
    m_root = nullptr;
  }

  void dump(std::ostream &p_ostream) const {
    struct dumper {
      size_type m_curr_index = 0;
      void dump_helper(base_ptr p_root, std::ostream &p_ostream) {
        size_type curr_index = m_curr_index;

        if (!p_root) {
          p_ostream << "\tnode_" << m_curr_index++ << " [label = \"NIL:0\"];\n";
          return;
        }

        p_ostream << "\tnode_" << m_curr_index++ << " [label = \""
                  << static_cast<node_ptr>(p_root)->m_value << ":"
                  << p_root->m_size << "\"];\n";
        p_ostream << "\tnode_" << curr_index << " -> node_" << curr_index + 1
                  << ";\n";
        dump_helper(p_root->m_left, p_ostream);

        p_ostream << "\tnode_" << curr_index << " -> node_" << m_curr_index
                  << ";\n";
        dump_helper(p_root->m_right, p_ostream);
      }
    };

    p_ostream << "digraph BST {\n";
    dumper{}.dump_helper(m_root, p_ostream);
    p_ostream << "}";
  }

protected:
  // clang-format off
  template <typename F>
  std::tuple<node_ptr, node_ptr, bool> traverse_bs(node_ptr p_r, const t_key_type &p_k, F p_f) const {
    if (empty()) {
      return std::make_tuple(nullptr, nullptr, false);
    }

    node_ptr prev = nullptr, curr = p_r;
    bool is_less_than_key{};

    while (curr && (curr->m_value != p_k)) {
      is_less_than_key = t_comp{}(curr->m_value, p_k);
      p_f(*curr);
      prev = curr;
      if (is_less_than_key) {
        curr = static_cast<node_ptr>(curr->m_right);
      } else {
        curr = static_cast<node_ptr>(curr->m_left);
      }
    }

    return std::make_tuple(curr, prev, is_less_than_key);
  }

  template <typename F> void traverse_postorder(const_base_ptr p_r, F p_f) const {
    const_base_ptr prev{};

    while (p_r) {
      const_base_ptr parent{p_r->m_parent}, left{p_r->m_left}, right{p_r->m_right};

      if (prev == parent) {
        prev = p_r;
        if (left) {
          p_r = left;
        } else if (right) {
          p_r = right;
        } else {
          p_f(p_r);
          p_r = parent;
        }
      }

      else if (prev == left) {
        prev = p_r;
        if (right) {
          p_r = right;
        } else {
          p_f(p_r);
          p_r = parent;
        }
      }

      else {
        prev = p_r;
        p_f(p_r);
        p_r = parent;
      }
    }
  }
  // clang-format on

  std::pair<node_ptr, node_ptr> bst_lookup(const t_key_type &p_key) const {
    auto [found, prev, is_prev_less] = traverse_bs(
        static_cast<node_ptr>(m_root), p_key, [](const node_type &) {});
    return {found, prev};
  }

  std::pair<node_ptr, node_ptr> bst_insert(const t_value_type &p_key) {
    std::unique_ptr<node_type> to_insert = std::make_unique<node_type>(p_key);

    if (empty()) {
      m_root = m_leftmost = m_rightmost = to_insert.get();
      return {to_insert.release(), nullptr};
    }

    auto [found, prev, is_prev_less] =
        traverse_bs(static_cast<node_ptr>(m_root), p_key,
                    [](node_type &p_node) { p_node.m_size++; });

    if (found) {
      traverse_bs(static_cast<node_ptr>(m_root), p_key,
                  [](node_type &p_node) { p_node.m_size--; });
      return {nullptr, prev}; // Double insert
    }

    to_insert->m_parent = prev;
    if (is_prev_less) {
      prev->m_right = to_insert.get();
    } else {
      prev->m_left = to_insert.get();
    }

    if (t_comp{}(p_key, static_cast<node_ptr>(m_leftmost)->m_value)) {
      m_leftmost = to_insert.get();
    } else if (t_comp{}(static_cast<node_ptr>(m_rightmost)->m_value, p_key)) {
      m_rightmost = to_insert.get();
    }

    return {to_insert.release(), prev};
  }

  // Constructors
  bs_order_tree() : bs_order_tree_impl{} {}

  ~bs_order_tree() { clear(); }

  // Copy constructor and assigment should be defined in a derived class.
  bs_order_tree(const self &p_other) = delete;
  self &operator=(const self &p_other) = delete;

  bs_order_tree(self &&p_other) noexcept {
    std::swap(m_root, p_other.m_root);
    std::swap(m_leftmost, p_other.m_leftmost);
    std::swap(m_rightmost, p_other.m_rightmost);
  }

  self &operator=(self &&p_other) noexcept {
    if (this != &p_other) {
      std::swap(m_root, p_other.m_root);
      std::swap(m_leftmost, p_other.m_leftmost);
      std::swap(m_rightmost, p_other.m_rightmost);
    }
    return *this;
  }
};

} // namespace detail
} // namespace throttle
