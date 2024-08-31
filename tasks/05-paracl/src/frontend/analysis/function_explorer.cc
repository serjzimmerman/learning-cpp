/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "frontend/analysis/function_explorer.hpp"

#include "frontend/ast/ast_nodes.hpp"
#include "frontend/ast/node_identifier.hpp"

#include "utils/misc.hpp"

#include <fmt/core.h>

#include <iostream>
#include <string>

namespace paracl::frontend {

void function_explorer::explore(ast::function_definition &ref) {
  assert(ref.name.has_value() &&
         "Encountered an unnamed function. This shouldn't happen");

  auto &&name_v = ref.name.value();
  auto [attr, inserted] =
      m_analytics->named_functions.define_function(name_v, {&ref});

  if (!inserted) {
    auto report = error_report{
        {fmt::format("Redefinition of function `{}`", name_v), ref.loc()}};
    report.add_attachment(
        error_attachment{fmt::format("[Note] Previously declared here:"),
                         attr.definition->loc()});
    report_error(report);
    return;
  }

  if (!m_function_stack.empty()) {
    m_analytics->usegraph.insert(usegraph_type::value_type{name_v, &ref},
                                 m_function_stack.back());
  } else {
    m_analytics->usegraph.insert(usegraph_type::value_type{name_v, &ref});
  }

  m_function_stack.push_back({name_v, &ref});
  apply(ref.body());
  m_function_stack.pop_back();
}

void function_explorer::explore(ast::function_call &ref) {
  auto name_v = std::string{ref.name()};
  auto found = m_analytics->named_functions.lookup(name_v);

  auto *def = (found ? found->definition : nullptr);
  ref.m_def = def;

  if (found) {
    if (!m_function_stack.empty()) {
      auto &&curr_func = m_function_stack.back();
      // Do not create recursive loops. These will get handled separately.
      if (curr_func.key != ref.name()) {
        m_analytics->usegraph.insert(usegraph_type::value_type{name_v, def},
                                     m_function_stack.back());
      } else {
        m_analytics->named_functions.set_recursive(name_v);
      }
    }

    else {
      m_analytics->usegraph.insert(usegraph_type::value_type{name_v, def});
    }
  }

  for (auto *param : ref) {
    assert(param && "Encountered a nullptr in statement block");
    apply(*param);
  }
}

void function_explorer::explore(
    const ast::function_definition_to_ptr_conv &ref) {
  auto &def = ref.definition();
  auto &name = def.name;

  if (!name) {
    name = fmt::format("$anon-func-{}", m_analytics->named_functions.size());
  }

  if (!m_function_stack.empty()) {
    m_analytics->usegraph.insert(usegraph_type::value_type{name.value(), &def},
                                 m_function_stack.back());
  } else {
    m_analytics->usegraph.insert(usegraph_type::value_type{name.value(), &def});
  }

  apply(def);
}

} // namespace paracl::frontend
