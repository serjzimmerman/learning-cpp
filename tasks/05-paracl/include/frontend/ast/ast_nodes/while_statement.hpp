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

#include "frontend/symtab.hpp"
#include "i_ast_node.hpp"
#include "statement_block.hpp"

namespace paracl::frontend::ast {

class while_statement : public i_ast_node {
public:
  symtab symbol_table;

private:
  i_expression *m_condition;
  i_ast_node *m_block;

  EZVIS_VISITABLE();

public:
  while_statement(i_expression &cond, i_ast_node &block, location l)
      : i_ast_node{l}, m_condition{&cond}, m_block{&block} {}

  i_expression &cond() const { return *m_condition; }
  i_ast_node &block() const { return *m_block; }
};

} // namespace paracl::frontend::ast
