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
#include <optional>
#include <stdexcept>
#include <tuple>
#include <type_traits>

namespace throttle {
namespace detail {
enum rb_tree_ranged_color_ { k_black_, k_red_ };

struct rb_tree_ranged_node_base_ {
  using base_ptr_ = rb_tree_ranged_node_base_ *;
  using const_base_ptr_ = const rb_tree_ranged_node_base_ *;
  using size_type = std::size_t;

  rb_tree_ranged_color_ m_color_;
  size_type m_size_;
  base_ptr_ m_left_;
  base_ptr_ m_right_;
  base_ptr_ m_parent_;

  static rb_tree_ranged_color_ get_color_(const_base_ptr_ p_x) {
    return (p_x ? p_x->m_color_ : k_black_);
  }

  static size_type size(base_ptr_ p_x) noexcept {
    return (p_x ? p_x->m_size_ : 0); // By definitions size of "NIL" node is 0;
  }

  base_ptr_ minimum_() noexcept {
    base_ptr_ curr = this;
    while (curr->m_left_)
      curr = curr->m_left_;
    return curr;
  }

  base_ptr_ maximum_() noexcept {
    base_ptr_ curr = this;
    while (curr->m_right_)
      curr = curr->m_right_;
    return curr;
  }

  base_ptr_ successor_() noexcept {
    if (m_right_)
      return (m_right_)->minimum_();
    return nullptr;
  }

  base_ptr_ predecessor_() noexcept {
    if (m_left_)
      return (m_left_)->maximum_();
    return nullptr;
  }

  base_ptr_ get_sibling_() noexcept {
    if (is_left_child_())
      return m_parent_->m_right_;
    else
      return m_parent_->m_left_;
  }

  base_ptr_ get_uncle_() noexcept {
    if (m_parent_->is_right_child_())
      return m_parent_->m_parent_->m_left_;
    else
      return m_parent_->m_parent_->m_right_;
  }

  bool is_left_child_() const noexcept { return (this == m_parent_->m_left_); }

  bool is_right_child_() const noexcept {
    return (this == m_parent_->m_right_);
  }

  bool is_linear() noexcept {
    return ((is_left_child_() && m_parent_->is_left_child_()) ||
            (is_right_child_() && m_parent_->is_right_child_()));
  }
};

template <typename t_value_type_>
struct rb_tree_ranged_node_ : public rb_tree_ranged_node_base_ {
  using node_type_ = rb_tree_ranged_node_<t_value_type_>;
  using node_ptr_ = node_type_ *;
  using const_node_ptr_ = const node_type_ *;

  t_value_type_ m_value_;

  rb_tree_ranged_node_(const t_value_type_ &p_key)
      : rb_tree_ranged_node_base_{k_red_, 1, nullptr, nullptr, nullptr},
        m_value_{p_key} {};
};

class rb_tree_ranged_impl_ {
public:
  using base_ptr_ = rb_tree_ranged_node_base_::base_ptr_;
  using const_base_ptr_ = rb_tree_ranged_node_base_ ::const_base_ptr_;
  using link_type_ = rb_tree_ranged_node_base_;
  using size_type = rb_tree_ranged_node_base_::size_type;

protected:
  base_ptr_ m_root_;

  void rotate_left_(base_ptr_) noexcept;
  void rotate_right_(base_ptr_) noexcept;
  void rotate_to_parent_(base_ptr_) noexcept;

  void rebalance_after_insert_(base_ptr_) noexcept;
  void rebalance_after_erase_(base_ptr_) noexcept;

  base_ptr_ successor_for_erase_(base_ptr_) noexcept;
  base_ptr_ predecessor_for_erase_(base_ptr_) noexcept;

  rb_tree_ranged_impl_() : m_root_{} {}
};

template <typename t_value_type, typename t_comp>
class rb_tree_ranged_ : public rb_tree_ranged_impl_ {
private:
  static_assert(std::is_swappable_v<t_value_type>,
                "t_value_type must be swappable");

  using node_type_ = rb_tree_ranged_node_<t_value_type>;
  using node_ptr_ = typename node_type_::node_ptr_;
  using const_node_ptr_ = typename node_type_::const_node_ptr_;

  using self_type_ = rb_tree_ranged_<t_value_type, t_comp>;
  using const_self_type_ = const self_type_;

public:
  bool empty() const noexcept { return !m_root_; }
  size_type size() const noexcept { return (m_root_ ? m_root_->m_size_ : 0); }
  bool contains(const t_value_type &p_key) const noexcept {
    return bst_lookup(p_key);
  }

private:
  void prune_leaf(node_ptr_ p_n) {
    if (!p_n->m_parent_) {
      m_root_ = nullptr;
    } else if (p_n->is_left_child_()) {
      p_n->m_parent_->m_left_ = nullptr;
    } else {
      p_n->m_parent_->m_right_ = nullptr;
    }

    delete p_n;
  }

  template <typename F>
  std::tuple<node_ptr_, node_ptr_, bool>
  traverse_binary_search(node_ptr_ p_r, const t_value_type &p_key, F p_f) {
    if (!p_r) {
      return std::make_tuple(nullptr, nullptr, false);
    }

    node_ptr_ prev = nullptr;
    bool is_less_than_key{};

    while (p_r && (p_r->m_value_ != p_key)) {
      is_less_than_key = t_comp{}(p_r->m_value_, p_key);
      p_f(*p_r);
      prev = p_r;
      if (is_less_than_key) {
        p_r = static_cast<node_ptr_>(p_r->m_right_);
      } else {
        p_r = static_cast<node_ptr_>(p_r->m_left_);
      }
    }

    return std::make_tuple(p_r, prev, is_less_than_key);
  }

  template <typename F>
  std::tuple<const_node_ptr_, const_node_ptr_, bool>
  traverse_binary_search(const_node_ptr_ p_r, const t_value_type &p_key,
                         F p_f) const {
    if (!p_r) {
      return std::make_tuple(nullptr, nullptr, false);
    }

    const_node_ptr_ prev = nullptr;
    bool is_less_than_key{};

    while (p_r && (p_r->m_value_ != p_key)) {
      is_less_than_key = t_comp{}(p_r->m_value_, p_key);
      p_f(*p_r);
      prev = p_r;
      if (is_less_than_key) {
        p_r = static_cast<node_ptr_>(p_r->m_right_);
      } else {
        p_r = static_cast<node_ptr_>(p_r->m_left_);
      }
    }

    return std::make_tuple(p_r, prev, is_less_than_key);
  }

  template <typename F> void traverse_postorder(base_ptr_ p_r, F p_f) {
    base_ptr_ prev{};

    while (p_r) {
      base_ptr_ parent{p_r->m_parent_}, left{p_r->m_left_},
          right{p_r->m_right_};

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

  template <typename F>
  void traverse_postorder(const_base_ptr_ p_r, F p_f) const {
    const_base_ptr_ prev{};

    while (p_r) {
      const_base_ptr_ parent{p_r->m_parent_}, left{p_r->m_left_},
          right{p_r->m_right_};

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

  node_ptr_ bst_lookup(const t_value_type &p_key) noexcept {
    auto [found, prev, is_prev_less] = traverse_binary_search(
        static_cast<node_ptr_>(m_root_), p_key, [](node_type_ &) {});
    return found;
  }

  const_node_ptr_ bst_lookup(const t_value_type &p_key) const noexcept {
    auto [found, prev, is_prev_less] = traverse_binary_search(
        static_cast<node_ptr_>(m_root_), p_key, [](const node_type_ &) {});
    return found;
  }

  node_ptr_ bst_insert(const t_value_type &p_key) {
    node_ptr_ to_insert = new node_type_{p_key};

    if (empty()) {
      to_insert->m_color_ = k_black_;
      m_root_ = to_insert;
      return to_insert;
    }

    auto [found, prev, is_prev_less] =
        traverse_binary_search(static_cast<node_ptr_>(m_root_), p_key,
                               [](node_type_ &p_node) { p_node.m_size_++; });

    if (found) {
      traverse_binary_search(static_cast<node_ptr_>(m_root_), p_key,
                             [](node_type_ &p_node) { p_node.m_size_--; });
      delete to_insert;
      throw std::out_of_range("Double insert");
    }

    to_insert->m_parent_ = prev;
    if (is_prev_less) {
      prev->m_right_ = to_insert;
    } else {
      prev->m_left_ = to_insert;
    }

    return to_insert;
  }

  node_ptr_ move_to_leaf_for_erase(node_ptr_ p_node) noexcept(
      std::is_nothrow_swappable_v<t_value_type>) {
    node_ptr_ node = p_node;

    while (node->m_right_ || node->m_left_) {
      node_ptr_ next =
          static_cast<node_ptr_>(node->m_right_ ? successor_for_erase_(node)
                                                : predecessor_for_erase_(node));
      std::swap(node->m_value_, next->m_value_);
      node = next;
    }

    return node;
  }

  template <typename t_iter>
  void insert_range(t_iter p_start, t_iter p_finish) {
    for (t_iter its = p_start, ite = p_finish; its != ite; ++its) {
      insert(*its);
    }
  }

public:
  using value_type = t_value_type;
  using comp = t_comp;
  using size_type = typename node_type_::size_type;

  void insert(const t_value_type &p_key) {
    node_ptr_ leaf = bst_insert(p_key);
    rebalance_after_insert_(leaf);
  }

  void erase(const t_value_type &p_key) {
    auto [node, prev, pred] =
        traverse_binary_search(static_cast<node_ptr_>(m_root_), p_key,
                               [](node_type_ &p_node) { p_node.m_size_--; });
    if (!node) {
      traverse_binary_search(static_cast<node_ptr_>(m_root_), p_key,
                             [](node_type_ &p_node) { p_node.m_size_++; });
      throw std::out_of_range("Can't erase element a non-present element");
    }

    node_ptr_ leaf = move_to_leaf_for_erase(node);
    leaf->m_size_ = 0;
    rebalance_after_erase_(leaf);

    prune_leaf(leaf);
  }

  void clear() noexcept {
    // A more optimized version of iterative approach for postorder traversal.
    // In this case node pointers can be used to indicate which nodes have
    // already been visited.
    base_ptr_ curr = m_root_;
    while (curr) {
      if (curr->m_left_)
        curr = curr->m_left_;
      else if (curr->m_right_)
        curr = curr->m_right_;

      else {
        base_ptr_ parent = curr->m_parent_;
        if (parent) {
          if (curr->is_left_child_())
            parent->m_left_ = nullptr;
          else
            parent->m_right_ = nullptr;
        }
        delete curr;
        curr = parent;
      }
    }
    m_root_ = nullptr;
  }

  const t_value_type &closest_left(const t_value_type &p_key) const {
    base_ptr_ curr = m_root_, bound = nullptr;

    while (curr) {
      bool is_key_less =
          t_comp{}(p_key, static_cast<node_ptr_>(curr)->m_value_);
      if (is_key_less) {
        curr = curr->m_left_;
      } else {
        bound = curr;
        curr = curr->m_right_;
      }
    }

    if (!bound)
      throw std::out_of_range("Leftmost element has no predecessor");
    return static_cast<node_ptr_>(bound)->m_value_;
  }

  const t_value_type &closest_right(const t_value_type &p_key) const {
    base_ptr_ curr = m_root_, bound = nullptr;

    while (curr) {
      bool is_key_less =
          t_comp{}(p_key, static_cast<node_ptr_>(curr)->m_value_);
      if (is_key_less) {
        bound = curr;
        curr = curr->m_left_;
      } else {
        curr = curr->m_right_;
      }
    }

    if (!bound)
      throw std::out_of_range("Rightmost element has no successor");
    return static_cast<node_ptr_>(bound)->m_value_;
  }

  // Rank operations that constiture the juice of this whole ordeal.
  const t_value_type &select_rank(size_type p_rank) const {
    if (p_rank > size() || !(p_rank > 0))
      throw std::out_of_range("Rank is greater than size or is zero");

    const_base_ptr_ curr = m_root_;
    size_type r = link_type_::size(curr->m_left_) + 1;
    while (r != p_rank) {
      if (p_rank < r) {
        curr = curr->m_left_;
      } else {
        curr = curr->m_right_;
        p_rank -= r;
      }
      r = link_type_::size(curr->m_left_) + 1;
    }

    return {static_cast<const_node_ptr_>(curr)->m_value_};
  }

  size_type get_rank_of(const t_value_type &p_elem) const {
    const_base_ptr_ node = bst_lookup(p_elem);
    if (!node)
      throw std::out_of_range("Element not present");

    size_type rank = link_type_::size(node->m_left_) + 1;
    while (node != m_root_) {
      if (node->is_right_child_())
        rank += link_type_::size(node->m_parent_->m_left_) + 1;
      node = node->m_parent_;
    }

    return rank;
  }

  const t_value_type &min() const {
    if (!m_root_)
      throw std::out_of_range("Container is empty");
    return static_cast<node_ptr_>(m_root_->minimum_())->m_value_;
  }

  const t_value_type &max() const {
    if (!m_root_)
      throw std::out_of_range("Container is empty");
    return static_cast<node_ptr_>(m_root_->maximum_())->m_value_;
  }

  rb_tree_ranged_() : rb_tree_ranged_impl_{} {}
  ~rb_tree_ranged_() { clear(); }

  rb_tree_ranged_(const_self_type_ &p_rhs) {
    self_type_ temp{};
    traverse_postorder(
        static_cast<const_node_ptr_>(p_rhs.m_root_), [&](const_base_ptr_ p_n) {
          temp.insert(static_cast<const_node_ptr_>(p_n)->m_value_);
        });
    *this = std::move(temp);
  }

  self_type_ &operator=(const_self_type_ &p_rhs) {
    if (this != &p_rhs) {
      self_type_ temp{p_rhs};
      *this = std::move(temp);
    }
    return *this;
  }

  rb_tree_ranged_(self_type_ &&p_rhs) noexcept {
    m_root_ = p_rhs.m_root_;
    p_rhs.m_root_ = nullptr;
  }

  self_type_ &operator=(self_type_ &&p_rhs) noexcept {
    if (this != &p_rhs) {
      std::swap(m_root_, p_rhs.m_root_);
    }
    return *this;
  }
};

} // namespace detail
} // namespace throttle
