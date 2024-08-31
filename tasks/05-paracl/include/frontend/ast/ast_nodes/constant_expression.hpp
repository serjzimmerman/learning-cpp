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

class constant_expression
    : public i_expression { // Inherit from i_expression but dont pass trivial
                            // builtin type by shared ptr. Should fix in the
                            // future
  int m_val;

  EZVIS_VISITABLE();

public:
  constant_expression(int p_val, location l)
      : i_expression{l, types::type_builtin::type_int}, m_val{p_val} {}
  int value() const { return m_val; }
};

} // namespace paracl::frontend::ast
