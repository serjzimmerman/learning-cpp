/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

%skeleton "lalr1.cc"
%require "3.5"

%defines

%define api.token.raw
%define api.parser.class { parser }
%define api.token.constructor
%define api.value.type variant
%define parse.assert
%define api.namespace { circuits }

%code requires {

#include <iostream>
#include <string>
#include <vector>
#include "edge.hpp"
#include <optional>

namespace circuits{
  class scanner;
  class driver;
}

using namespace circuits;

}

%code top
{

#include <iostream>
#include <string>

#include "driver.hpp"
#include "scanner.hpp"
#include "bison_network_parser.hpp"

static circuits::parser::symbol_type yylex(circuits::scanner &p_scanner, circuits::driver &p_driver) {
  return p_scanner.get_next_token();
}

}

%lex-param { circuits::scanner &scanner }
%lex-param { circuits::driver &driver }
%parse-param { circuits::scanner &scanner }
%parse-param { circuits::driver &driver }

%define parse.trace
%define parse.error verbose
%define api.token.prefix {TOKEN_}

/* Signle letter tokens */
%token LINE     "--"
%token SEMICOL  ";"
%token COMMA    ","
%token VOLTAGE  "V"

/* Terminals */
%token <unsigned> UNSIGNED "unsigned"
%token <double> DOUBLE "double"

%token EOF 0 "end of file"

%type <double> unsigned_or_double

%type <circuits::network_edge> trailing_edge last_edge
%type <std::vector<circuits::network_edge>> network

%start all

/* The abomination you are to see next is due to shift/reduce conflict if one was to write this grammar naively */

%%

all: network  { driver.m_parsed = std::move($1); }

network:  network trailing_edge     { $$ = std::move($1); auto first = $2.first; $2.first = $$.back().first; $$.back() = $2; $$.emplace_back(first); }
          | network last_edge       { $$ = std::move($1); $2.first = $$.back().first; $$.back() = $2; }
          | UNSIGNED LINE           { $$.emplace_back($1, 0, 0, std::nullopt); }

trailing_edge:  UNSIGNED COMMA unsigned_or_double SEMICOL unsigned_or_double VOLTAGE UNSIGNED LINE              { $$ = {$7, $1, $3, $5}; }
                | UNSIGNED COMMA unsigned_or_double SEMICOL unsigned_or_double VOLTAGE SEMICOL UNSIGNED LINE    { $$ = {$8, $1, $3, $5}; }
                | UNSIGNED COMMA unsigned_or_double SEMICOL UNSIGNED LINE                                       { $$ = {$5, $1, $3, std::nullopt}; }

last_edge:  UNSIGNED COMMA unsigned_or_double SEMICOL unsigned_or_double VOLTAGE EOF                { $$ = {0, $1, $3, $5}; }
            | UNSIGNED COMMA unsigned_or_double SEMICOL unsigned_or_double VOLTAGE SEMICOL EOF      { $$ = {0, $1, $3, $5}; }
            | UNSIGNED COMMA unsigned_or_double SEMICOL EOF                                         { $$ = {0, $1, $3}; }

unsigned_or_double: UNSIGNED  { $$ = $1; }
                    | DOUBLE  { $$ = $1; }

%%

// Bison expects us to provide implementation - otherwise linker complains
void circuits::parser::error(const std::string &message) {
  throw std::runtime_error{message};
}