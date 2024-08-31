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

class if_statement : public i_ast_node {
public:
  symtab control_block_symtab;
  symtab true_symtab;
  symtab false_symtab;

private:
  i_expression *m_condition;
  i_ast_node *m_true_block;
  i_ast_node *m_else_block = nullptr;

  EZVIS_VISITABLE();

public:
  if_statement(i_expression &cond, i_ast_node &true_block, location l)
      : i_ast_node{l}, m_condition{&cond}, m_true_block{&true_block} {}

  if_statement(i_expression &cond, i_ast_node &true_block,
               i_ast_node &else_block, location l)
      : i_ast_node{l}, m_condition{&cond}, m_true_block{&true_block},
        m_else_block{&else_block} {}

  i_expression &cond() const { return *m_condition; }
  i_ast_node &true_block() const { return *m_true_block; }
  i_ast_node *else_block() const { return m_else_block; }
};

} // namespace paracl::frontend::ast
