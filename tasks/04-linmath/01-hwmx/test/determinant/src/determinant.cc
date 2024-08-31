#include <boost/lexical_cast/bad_lexical_cast.hpp>
#include <chrono>
#include <cmath>
#include <iostream>
#include <set>
#include <string>

#include "contiguous_matrix.hpp"
#include "matrix.hpp"
#include "vector.hpp"

#include <concepts>
#include <optional>
#include <string>

#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/option.hpp>

namespace po = boost::program_options;

template <typename T>
bool main_loop_determinant(unsigned n, bool measure = false) {
  throttle::linmath::matrix<T> m{n, n};

  for (unsigned i = 0; i < n * n; ++i) {
    T temp;
    if (!(std::cin >> temp)) {
      std::cout << "Can't read " << i << "-th element";
      return false;
    }
    m[i / n][i % n] = temp;
  }

  auto start = std::chrono::high_resolution_clock::now();
  auto det = m.determinant();
  auto finish = std::chrono::high_resolution_clock::now();
  auto elapsed = std::chrono::duration<double, std::milli>(finish - start);

  if constexpr (std::is_floating_point_v<T>) {
    std::cout << std::fixed;
  }

  std::cout << det << "\n";

  if (measure) {
    std::cout << "determinant calculation took " << elapsed.count()
              << "ms to run\n";
  }

  return true;
}

int main(int argc, char *argv[]) {
  bool measure = false;

  std::string opt;
  po::options_description desc("Available options");
  desc.add_options()("help,h", "Print this help message")(
      "measure,m", "Print perfomance metrics")(
      "type,t", po::value<std::string>(&opt)->default_value("double"),
      "Type for matrix element (int, long, float, double)");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  measure = vm.count("measure");

  std::string n_str;

  if (!(std::cin >> n_str)) {
    std::cout << "Invalid matrix size\n";
    return 1;
  }

  int n;
  try {
    n = boost::lexical_cast<int>(n_str);
  } catch (boost::bad_lexical_cast &) {
    std::cout << "Invalid matrix size\n";
    return 1;
  }

  if (n <= 0) {
    std::cout << "Invalid matrix size\n";
    return 1;
  }

  if (opt == "int") {
    if (!main_loop_determinant<int>(n, measure))
      return 1;
  } else if (opt == "long") {
    if (!main_loop_determinant<long>(n, measure))
      return 1;
  } else if (opt == "float") {
    if (!main_loop_determinant<float>(n, measure))
      return 1;
  } else if (opt == "double") {
    if (!main_loop_determinant<double>(n, measure))
      return 1;
  }
}
