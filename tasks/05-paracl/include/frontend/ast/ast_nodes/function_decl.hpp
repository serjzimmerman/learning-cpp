/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "frontend/symtab.hpp"
#include "frontend/types/types.hpp"

#include "i_ast_node.hpp"
#include "variable_expression.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace paracl::frontend::ast {

class function_definition : public i_ast_node,
                            private std::vector<variable_expression> {
public:
  // An optional function name. Those functions that don't have a name will be
  // called anonymous functions
  std::optional<std::string> name;
  symtab param_stab;

private:
  i_ast_node *m_block;

private:
  types::type_composite_function
  make_func_type(const types::generic_type &return_type) {
    std::vector<types::generic_type> arg_types;

    for (auto &v : *this) {
      if (!v.type)
        v.type = types::type_builtin::type_int;
      arg_types.push_back(v.type);
      param_stab.declare(v.name(), &v);
    }

    return types::type_composite_function{arg_types, return_type};
  }

  EZVIS_VISITABLE();

public:
  types::type_composite_function type;

  function_definition(std::optional<std::string> p_name, i_ast_node &body,
                      location l, std::vector<variable_expression> vars = {},
                      types::generic_type return_type = {})
      : i_ast_node{l}, vector{std::move(vars)}, name{p_name}, m_block{&body},
        type{make_func_type(return_type)} {}

public:
  using vector::begin;
  using vector::cbegin;
  using vector::cend;
  using vector::crbegin;
  using vector::crend;
  using vector::end;
  using vector::size;

  symtab &param_symtab() { return param_stab; }
  i_ast_node &body() const { return *m_block; }
};

class function_definition_to_ptr_conv final : public i_expression {
  function_definition *m_definition;

  EZVIS_VISITABLE();

public:
  function_definition_to_ptr_conv(location l, function_definition &def)
      : i_expression{l}, m_definition{&def} {}
  function_definition &definition() const { return *m_definition; }
};

} // namespace paracl::frontend::ast
