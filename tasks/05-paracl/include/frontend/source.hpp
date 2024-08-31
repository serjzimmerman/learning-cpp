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

#include "utils/files.hpp"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace paracl::frontend {

class source_input {
private:
  std::unique_ptr<std::string> m_filename; // Name of the source file
  // Unique pointer to keep the underlying pointer the same, even when the
  // object is moved from.

private:
  std::string m_file_source; // Raw file representation as a string
  using line_vec = std::vector<std::string>;
  line_vec m_file_lines; // Vector of lines split

private:
  void fill_lines() {
    std::istringstream iss{m_file_source};
    for (std::string line; std::getline(iss, line);) {
      m_file_lines.push_back(line);
    }
  }

public:
  source_input(const std::filesystem::path &input_path)
      : m_filename{std::make_unique<std::string>(input_path.string())},
        m_file_source{utils::read_file(input_path)} {
    fill_lines();
  }

  std::string_view getline(unsigned i) const & {
    assert(i != 0 && "Line number can't be equal to 1");
    return m_file_lines.at(
        i - 1); /* Bison lines start with 1, so we have to subtrack */
  }

  // Can't make this const qualified, because bison location requires it be a
  // modifiable pointer for whatever reason. Note that the string is allocated
  // on the heap to avoid issues with default constructors.
  std::string *filename() & { return m_filename.get(); }
  std::istringstream iss() const & { return std::istringstream{m_file_source}; }
};

} // namespace paracl::frontend
