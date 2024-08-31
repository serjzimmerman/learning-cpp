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

#include <exception>
#include <stdexcept>
#include <string>

namespace ezvk {

class error : public std::exception {
  std::string m_message;

public:
  error(const std::string &msg) : m_message{msg} {}
  const char *what() const noexcept override { return m_message.c_str(); }
};

class vk_error : public error {
public:
  vk_error(const std::string &msg) : error{msg} {}
};

}; // namespace ezvk
