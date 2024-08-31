/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>
#include <string>

#include "equal.hpp"
#include "utility.hpp"

#include <boost/program_options.hpp>
#include <boost/program_options/option.hpp>

namespace po = boost::program_options;

bool contain_same(std::string name_a, std::string name_b) {
  std::ifstream file_a{name_a};
  std::ifstream file_b{name_b};

  if (!file_a.good()) {
    std::cout << "Can't open " << name_a << " \n";
    return false;
  }

  if (!file_b.good()) {
    std::cout << "Can't open " << name_b << " \n";
    return false;
  }

  std::vector<double> vec_a, vec_b;

  std::copy(std::istream_iterator<float>(file_a),
            std::istream_iterator<float>(), std::back_inserter(vec_a));
  std::copy(std::istream_iterator<float>(file_b),
            std::istream_iterator<float>(), std::back_inserter(vec_b));

  if (vec_a.size() != vec_b.size())
    return false;

  return std::equal(vec_a.begin(), vec_a.end(), vec_b.begin(),
                    [](const auto &first, const auto &second) {
                      return throttle::is_roughly_equal(first, second, 1e-3);
                    });
}

int main(int argc, char *argv[]) {
  std::string file_a, file_b;

  po::options_description desc("Available options");
  desc.add_options()("help,h", "Print this help message")(
      "input-file", po::value<std::vector<std::string>>(),
      "File to be compared");

  po::positional_options_description p;
  p.add("input-file", -1);

  po::variables_map vm;
  po::store(
      po::command_line_parser(argc, argv).options(desc).positional(p).run(),
      vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  if (vm.count("input-file")) {
    std::vector<std::string> input_files =
        vm["input-file"].as<std::vector<std::string>>();
    if (input_files.size() < 2) {
      std::cout << "Nothing to compare\n";
      return 1;
    } else if (input_files.size() > 2) {
      std::cout << "More than 2 files to compare\n";
      return 1;
    }
    return (contain_same(input_files[0], input_files[1]) ? 0 : 1);
  }

  else {
    std::cout << "Nothing to do\n";
    return 1;
  }
}
