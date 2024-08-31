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
#include <cassert>

// This file implements part of the red black order statistic tree respondisble
// for rebalaincing and ensuring that red-black and size invariants are kept
// after insert and erase operations. For more information consult "Introduction
// to Algorithms" by T.H. Cormen or Wiki:
// https://en.wikipedia.org/wiki/Order_statistic_tree.

namespace throttle {
namespace detail {
void rb_tree_ranged_impl_::rotate_left_(base_ptr_ p_n) noexcept {
  assert(p_n);
  assert(p_n->m_right_);

  base_ptr_ root = p_n, rchild = p_n->m_right_;
  root->m_right_ = rchild->m_left_;
  if (rchild->m_left_) {
    rchild->m_left_->m_parent_ = root;
  }

  rchild->m_parent_ = root->m_parent_;

  if (!root->m_parent_) {
    m_root_ = rchild;
  } else if (root->is_left_child_()) {
    root->m_parent_->m_left_ = rchild;
  } else {
    root->m_parent_->m_right_ = rchild;
  }

  rchild->m_left_ = root;
  root->m_parent_ = rchild;

  rchild->m_size_ = root->m_size_;
  root->m_size_ =
      link_type_::size(root->m_left_) + link_type_::size(root->m_right_) + 1;
}

void rb_tree_ranged_impl_::rotate_right_(base_ptr_ p_n) noexcept {
  assert(p_n);
  assert(p_n->m_left_);

  base_ptr_ root = p_n, lchild = p_n->m_left_;
  root->m_left_ = lchild->m_right_;
  if (lchild->m_right_) {
    lchild->m_right_->m_parent_ = root;
  }

  lchild->m_parent_ = root->m_parent_;

  if (!root->m_parent_) {
    m_root_ = lchild;
  } else if (root->is_right_child_()) {
    root->m_parent_->m_right_ = lchild;
  } else {
    root->m_parent_->m_left_ = lchild;
  }

  lchild->m_right_ = root;
  root->m_parent_ = lchild;

  lchild->m_size_ = root->m_size_;
  root->m_size_ =
      link_type_::size(root->m_left_) + link_type_::size(root->m_right_) + 1;
}

void rb_tree_ranged_impl_::rotate_to_parent_(base_ptr_ p_n) noexcept {
  if (p_n->is_left_child_()) {
    rotate_right_(p_n->m_parent_);
  } else {
    rotate_left_(p_n->m_parent_);
  }
}

void rb_tree_ranged_impl_::rebalance_after_insert_(base_ptr_ p_node) noexcept {
  if (!p_node->m_parent_ || !p_node->m_parent_->m_parent_) {
    return;
  }

  while (link_type_::get_color_(p_node->m_parent_) == k_red_) {
    if (p_node == m_root_ ||
        (p_node->m_parent_ && p_node->m_parent_->m_color_ == k_black_)) {
      break;
    }

    base_ptr_ uncle = p_node->get_uncle_();
    if (link_type_::get_color_(uncle) == k_red_) {
      p_node->m_parent_->m_color_ = k_black_;
      uncle->m_color_ = k_black_;
      p_node->m_parent_->m_parent_->m_color_ = k_red_;
      p_node = p_node->m_parent_->m_parent_;
    } else {
      if (!p_node->is_linear()) {
        base_ptr_ old = p_node->m_parent_;
        rotate_to_parent_(p_node);
        p_node = old;
      }
      p_node->m_parent_->m_color_ = k_black_;
      p_node->m_parent_->m_parent_->m_color_ = k_red_;
      rotate_to_parent_(p_node->m_parent_);
    }
  }

  m_root_->m_color_ = k_black_;
}

void rb_tree_ranged_impl_::rebalance_after_erase_(base_ptr_ p_leaf) noexcept {
  while (link_type_::get_color_(p_leaf) != k_red_) {
    if (!p_leaf->m_parent_) {
      break;
    }

    base_ptr_ sibling = p_leaf->get_sibling_();
    if (link_type_::get_color_(sibling) == k_red_) {
      p_leaf->m_parent_->m_color_ = k_red_;
      sibling->m_color_ = k_black_;
      rotate_to_parent_(sibling);
      continue;
    }

    // Nephew is the right child of a sibling
    // Niece is the left child of a sibling
    base_ptr_ nephew = (sibling ? sibling->m_right_ : nullptr);
    if (link_type_::get_color_(nephew) == k_red_) {
      sibling->m_color_ = p_leaf->m_parent_->m_color_;
      p_leaf->m_parent_->m_color_ = k_black_;
      nephew->m_color_ = k_black_;
      rotate_to_parent_(sibling);
      p_leaf = m_root_;
      break;
    }

    base_ptr_ niece = (sibling ? sibling->m_left_ : nullptr);
    if (link_type_::get_color_(niece) == k_red_) {
      niece->m_color_ = k_black_;
      sibling->m_color_ = k_red_;
      rotate_to_parent_(niece);
    } else {
      if (sibling)
        sibling->m_color_ = k_red_;
      p_leaf = p_leaf->m_parent_;
    }
  }

  p_leaf->m_color_ = k_black_;
}

rb_tree_ranged_impl_::base_ptr_
rb_tree_ranged_impl_::successor_for_erase_(base_ptr_ p_node) noexcept {
  p_node->m_size_--;
  p_node = p_node->m_right_;
  while (p_node->m_left_) {
    p_node->m_size_--;
    p_node = p_node->m_left_;
  }
  return p_node;
}

rb_tree_ranged_impl_::base_ptr_
rb_tree_ranged_impl_::predecessor_for_erase_(base_ptr_ p_node) noexcept {
  p_node->m_size_--;
  p_node = p_node->m_left_;
  while (p_node->m_right_) {
    p_node->m_size_--;
    p_node = p_node->m_right_;
  }
  return p_node;
}

} // namespace detail
} // namespace throttle
