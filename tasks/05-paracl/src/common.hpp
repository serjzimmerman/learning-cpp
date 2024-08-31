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

#include "bytecode_vm/bytecode_builder.hpp"
#include "bytecode_vm/decl_vm.hpp"
#include "bytecode_vm/disassembly.hpp"
#include "bytecode_vm/opcodes.hpp"
#include "bytecode_vm/virtual_machine.hpp"

#include "utils/files.hpp"

#include <fmt/core.h>
#include <fmt/format.h>

#include <popl/popl.hpp>

#include <memory>
#include <optional>
#include <string>

namespace {

namespace bytecode_vm = paracl::bytecode_vm;
namespace decl_vm = bytecode_vm::decl_vm;
namespace disassembly = decl_vm::disassembly;
namespace instruction_set = bytecode_vm::instruction_set;

constexpr int k_exit_success = 0;
constexpr int k_exit_failure = 1;

[[maybe_unused]] void disassemble_chunk(const decl_vm::chunk &ch) {
  using disassembly::chunk_complete_disassembler;
  chunk_complete_disassembler disas{instruction_set::paracl_isa};
  disas(std::cout, ch);
}

[[maybe_unused]] void execute_chunk(const decl_vm::chunk &ch) {
  auto vm = bytecode_vm::create_paracl_vm();
  vm.set_program_code(std::move(ch));
  vm.execute();
}

[[maybe_unused]] std::optional<std::string>
read_input_file(const popl::Implicit<std::string> &option,
                const popl::OptionParser &op) {
  if (option.is_set())
    return option.value(); // Case 1. Option set
  if (op.non_option_args().size() == 1)
    return op.non_option_args().front();            // Case 2. Unnamed args
  fmt::println(stderr, "Input file not specified"); // Case 3. Error
  return std::nullopt;
}

} // namespace
