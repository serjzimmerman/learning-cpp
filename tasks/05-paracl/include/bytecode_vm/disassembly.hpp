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

#include "bytecode_vm/decl_vm.hpp"
#include "utils/files.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
#include <iomanip>
#include <iterator>
#include <optional>
#include <stdexcept>

namespace paracl::bytecode_vm::decl_vm::disassembly {

class constant_pool_disassembler {
public:
  auto &operator()(auto &os, std::input_iterator auto start,
                   std::input_iterator auto finish) const {
    os << ".constant_pool\n";

    for (constant_pool_type::size_type i = 0; start != finish; ++start, ++i) {
      utils::padded_hex_printer(os, i) << " = { "
                                       << "0x" << std::hex << *start << " }\n";
    }

    return os;
  }
};

template <typename t_instr_set> class chunk_binary_disassembler {
public:
  t_instr_set instruction_set;
  chunk_binary_disassembler(const t_instr_set &isa) : instruction_set{isa} {}

public:
  void operator()(auto &os, std::forward_iterator auto &first,
                  std::forward_iterator auto last) const {
    if (first == last) {
      throw std::runtime_error{"Unexpectedly reached the end of range"};
    }

    auto current_instruction =
        instruction_set.instruction_lookup_table[*first++];
    // clang-format off
    std::visit(::utils::visitors{
      [&](std::monostate) {
        throw std::runtime_error{"Unknown opcode"};},
      [&](auto&& instr) {
        auto attr = instr->decode(first, last).attributes;
        instr->pretty_print(os, attr); }},
      current_instruction);
    // clang-format on
  }

public:
  auto &operator()(auto &os, const chunk &ch) const {
    os << ".code\n";

    auto start = ch.binary_begin();
    for (auto first = ch.binary_begin(), last = ch.binary_end();
         first != last;) {
      utils::padded_hex_printer(os, std::distance(start, first)) << " ";
      operator()(os, first, last);
      os << "\n";
    }

    return os;
  }
};

template <typename t_instr_set> class chunk_complete_disassembler {
  chunk_binary_disassembler<t_instr_set> binary_disas;

public:
  chunk_complete_disassembler(t_instr_set isa) : binary_disas{isa} {}

  auto &operator()(auto &os, const chunk &chk) const {
    constant_pool_disassembler{}(os, chk.constants_begin(),
                                 chk.constants_end());
    os << "\n";
    binary_disas(os, chk);
    return os;
  }
};

} // namespace paracl::bytecode_vm::decl_vm::disassembly
