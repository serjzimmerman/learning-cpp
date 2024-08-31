/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#if !defined(yyFlexLexerOnce)
#undef yyFlexLexer
#define yyFlexLexer network_FlexLexer
#include <FlexLexer.h>
#endif

#undef YY_DECL
#define YY_DECL                                                                \
  circuits::parser::symbol_type circuits::scanner::get_next_token()

#include "bison_network_parser.hpp"

namespace circuits {

class scanner : public yyFlexLexer {
private:
public:
  scanner() {}
  circuits::parser::symbol_type get_next_token();
};

} // namespace circuits
