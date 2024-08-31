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

#include "ezvis/ezvis.hpp"
#include "frontend/ast/ast_nodes/i_ast_node.hpp"

#include <cassert>

namespace paracl::frontend::ast {

class ast_container;

class ast_copier
    : public ezvis::visitor_base<const i_ast_node, ast_copier, i_ast_node &> {
  using to_visit = tuple_all_nodes;
  ast_container &m_container;

public:
  ast_copier(ast_container &container) : m_container{container} {}

  EZVIS_VISIT_CT(to_visit)

  assignment_statement &copy(const assignment_statement &);
  binary_expression &copy(const binary_expression &);
  constant_expression &copy(const constant_expression &);
  if_statement &copy(const if_statement &);
  print_statement &copy(const print_statement &);
  read_expression &copy(const read_expression &);
  statement_block &copy(const statement_block &);
  unary_expression &copy(const unary_expression &);
  variable_expression &copy(const variable_expression &);
  while_statement &copy(const while_statement &);
  error_node &copy(const error_node &);
  function_definition &copy(const function_definition &);
  return_statement &copy(const return_statement &);
  function_call &copy(const function_call &);
  function_definition_to_ptr_conv &
  copy(const function_definition_to_ptr_conv &);

  EZVIS_VISIT_INVOKER(copy);

  template <typename T>
  i_expression &copy_expr(const T &expr)
    requires std::convertible_to<T, i_expression>
  {
    return static_cast<i_expression &>(apply(expr));
  }
};

inline i_ast_node *ast_copy(i_ast_node *node, ast_container &container) {
  if (!node)
    return nullptr; // In case the ast is empty. nullptr is a valid parameter
  ast_copier copier = {container};
  return &copier.apply(*node);
}

} // namespace paracl::frontend::ast

#include "ast_container.hpp"
