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

#include <cstddef>

namespace paracl::bytecode_vm {

// clang-format off
enum opcode : unsigned char {
  E_RETURN_NULLARY,

  E_PUSH_CONST_UNARY,
  E_POP_NULLARY,

  E_ADD_NULLARY, E_SUB_NULLARY, E_MUL_NULLARY,
  E_DIV_NULLARY, E_MOD_NULLARY, E_AND_NULLARY, E_OR_NULLARY, E_NOT_NULLARY,
  E_CMP_EQ_NULLARY, E_CMP_NE_NULLARY, E_CMP_GT_NULLARY, E_CMP_LS_NULLARY,
  E_CMP_GE_NULLARY, E_CMP_LE_NULLARY,

  E_PRINT_NULLARY, E_PUSH_READ_NULLARY, E_MOV_LOCAL_REL_UNARY,
  E_PUSH_LOCAL_REL_UNARY, E_PUSH_LOCAL_REL_TOP_UNARY,
  E_MOV_LOCAL_REL_TOP_UNARY,

  E_JMP_UNARY, E_JMP_DYNAMIC_NULLARY, E_JMP_DYNAMIC_REL_UNARY, E_JMP_FALSE_UNARY, E_JMP_TRUE_UNARY, E_SETUP_CALL_NULLARY,
  E_PUSH_SP_NULLARY, E_UPDATE_SP_UNARY, 
  
  E_LOAD_R0_NULLARY, E_STORE_R0_NULLARY, 
  E_PUSH_LOCAL_UNARY, E_MOV_LOCAL_UNARY
};
// clang-format on

} // namespace paracl::bytecode_vm
