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

#include "frontend/ast/ast_nodes/i_ast_node.hpp"
#include "frontend/types/types.hpp"

#include <algorithm>
#include <vector>

namespace paracl::frontend::ast {

class return_statement : public i_ast_node {
  i_expression *m_expr;

  EZVIS_VISITABLE();

public:
  return_statement(i_expression *p_expr, location l)
      : i_ast_node{l}, m_expr{p_expr} {}

  bool empty() const { return !m_expr; }
  i_expression &expr() {
    if (m_expr == nullptr)
      throw std::runtime_error{"Attempt to dereference empty return statement"};
    return *m_expr;
  }

  const i_expression &expr() const {
    if (m_expr == nullptr)
      throw std::runtime_error{"Attempt to dereference empty return statement"};
    return *m_expr;
  }

  types::generic_type type() const {
    if (!m_expr)
      return types::type_builtin::type_void;
    return m_expr->type;
  }
};

class return_vector : private std::vector<return_statement *> {
public:
  using vector::begin;
  using vector::end;
  using vector::operator[];
  using vector::at;
  using vector::back;
  using vector::cbegin;
  using vector::cend;
  using vector::clear;
  using vector::crbegin;
  using vector::crend;
  using vector::empty;
  using vector::front;
  using vector::push_back;
  using vector::rbegin;
  using vector::rend;
  using vector::size;

public:
  bool are_all_void() const {
    return std::all_of(
        cbegin(), cend(),
        [&void_type = types::type_builtin::type_void](return_statement *ret) {
          assert(ret);
          return ret->type() && ret->type() == void_type;
        });
  }
};

} // namespace paracl::frontend::ast
