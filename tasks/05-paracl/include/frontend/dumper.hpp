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

#include "ast/ast_nodes/i_ast_node.hpp"

#include <iostream>
#include <string>

namespace paracl::frontend::ast {

std::string ast_dump_str(const i_ast_node *node);

inline void ast_dump(i_ast_node *node, std::ostream &os) {
  os << ast_dump_str(node);
}

} // namespace paracl::frontend::ast
