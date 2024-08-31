/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

%{
#include <iostream>
#include <numeric>

#include "bison_network_parser.hpp"
#include "scanner.hpp"

#define yyterminate() 

using namespace circuits;
%}

%option noyywrap nounput noinput nodefault 
%option c++
%option prefix="network_"
%option yyclass="scanner"

number      (0|[1-9][0-9]*)
float       [+-]?({number}|{number}[.]{number}?)

%%

";"             { return parser::make_SEMICOL(); }
","             { return parser::make_COMMA(); }
"--"            { return parser::make_LINE(); }
"V"             { return parser::make_VOLTAGE(); }

{number}        { return parser::make_UNSIGNED(std::stoi(yytext)); }
{float}         { return parser::make_DOUBLE(std::stod(yytext)); }

[ \t\n\r]       { }

.               { throw parser::syntax_error{"Unexpected symbol"}; }
<<EOF>>         { return circuits::parser::make_EOF(); }

%%
