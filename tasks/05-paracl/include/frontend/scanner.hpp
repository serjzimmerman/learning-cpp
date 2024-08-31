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

#if !defined(yyFlexLexerOnce)
#undef yyFlexLexer
#define yyFlexLexer paracl_FlexLexer
#include <FlexLexer.h>
#endif

#undef YY_DECL
#define YY_DECL                                                                \
  paracl::frontend::parser::symbol_type                                        \
  paracl::frontend::scanner::get_next_token()

#include "bison_paracl_parser.hpp"
#include "location.hpp"

namespace paracl::frontend {
class parser_driver;

class scanner final : public yyFlexLexer {
private:
  parser_driver &m_driver;
  position m_pos;

private:
  auto symbol_length() const { return yyleng; }

public:
  scanner(parser_driver &driver, std::string *filename = nullptr)
      : m_driver{driver}, m_pos{filename} {}
  paracl::frontend::parser::symbol_type get_next_token();

  location update_loc() {
    auto old_pos = m_pos;
    auto new_pos = (m_pos += symbol_length());
    return location{old_pos, new_pos};
  }
};

} // namespace paracl::frontend

#include "frontend_driver.hpp"
