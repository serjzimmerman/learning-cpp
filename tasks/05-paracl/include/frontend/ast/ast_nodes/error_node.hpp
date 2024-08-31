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
#include "location.hpp"

#include <string>

namespace paracl::frontend::ast {

class error_node : public i_expression {
private:
  std::string m_error_message;

  EZVIS_VISITABLE();

public:
  error_node(std::string_view msg, location l)
      : i_expression{l}, m_error_message{msg} {};
  std::string error_msg() const { return m_error_message; }
};

} // namespace paracl::frontend::ast
