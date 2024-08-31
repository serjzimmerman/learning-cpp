/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "detail/bs_order_tree.hpp"
#include <cassert>

namespace throttle {
namespace detail {

void bs_order_tree_impl::rotate_left(base_ptr p_n) const noexcept {
  assert(p_n);
  assert(p_n->m_right);

  base_ptr root = p_n, rchild = p_n->m_right;
  root->m_right = rchild->m_left;
  if (rchild->m_left) {
    rchild->m_left->m_parent = root;
  }

  rchild->m_parent = root->m_parent;

  if (!root->m_parent) {
    m_root = rchild;
  } else if (root->is_left_child()) {
    root->m_parent->m_left = rchild;
  } else {
    root->m_parent->m_right = rchild;
  }

  rchild->m_left = root;
  root->m_parent = rchild;

  rchild->m_size = root->m_size;
  root->m_size =
      link_type::size(root->m_left) + link_type::size(root->m_right) + 1;
}

void bs_order_tree_impl::rotate_right(base_ptr p_n) const noexcept {
  assert(p_n);
  assert(p_n->m_left);

  base_ptr root = p_n, lchild = p_n->m_left;
  root->m_left = lchild->m_right;
  if (lchild->m_right) {
    lchild->m_right->m_parent = root;
  }

  lchild->m_parent = root->m_parent;

  if (!root->m_parent) {
    m_root = lchild;
  } else if (root->is_right_child()) {
    root->m_parent->m_right = lchild;
  } else {
    root->m_parent->m_left = lchild;
  }

  lchild->m_right = root;
  root->m_parent = lchild;

  lchild->m_size = root->m_size;
  root->m_size =
      link_type::size(root->m_left) + link_type::size(root->m_right) + 1;
}

void bs_order_tree_impl::rotate_to_parent(base_ptr p_n) const noexcept {
  if (p_n->is_left_child()) {
    rotate_right(p_n->m_parent);
  } else {
    rotate_left(p_n->m_parent);
  }
}

} // namespace detail
} // namespace throttle
