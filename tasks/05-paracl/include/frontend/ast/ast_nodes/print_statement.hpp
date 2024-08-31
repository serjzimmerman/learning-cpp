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

#include "i_ast_node.hpp"

namespace paracl::frontend::ast {

class print_statement : public i_ast_node {
  i_expression *m_expr;

  EZVIS_VISITABLE();

public:
  print_statement(i_expression &p_expr, location l)
      : i_ast_node{l}, m_expr{&p_expr} {}
  i_expression &expr() const { return *m_expr; }
};

} // namespace paracl::frontend::ast
