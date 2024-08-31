#include <chrono>
#include <iostream>
#include <vector>

#ifdef BOOST_FOUND__
#include <boost/program_options.hpp>
#include <boost/program_options/option.hpp>
namespace po = boost::program_options;
#endif

#include "belady.hpp"

int main(int argc, char *argv[]) {
  std::size_t n{}, m{};

  if (!std::cin || !std::cout) {
    std::abort();
  }

#ifdef BOOST_FOUND__
  po::options_description desc("Available options");
  desc.add_options()("help,h", "Print this help message")(
      "count-time,t", "Print perfomance metrics");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }
#endif

  std::cin >> m >> n;

  if (n == 0 || m == 0) {
    std::abort();
  }

  std::vector<int> vec{};
  vec.reserve(n);

  for (unsigned i = 0; i < n; ++i) {
    if (!std::cin || !std::cout) {
      std::abort();
    }

    int temp{};
    std::cin >> temp;

    if (std::cin.fail()) {
      std::abort();
    }

    vec.push_back(temp);
  }

  auto start = std::chrono::high_resolution_clock::now();
  auto hits = caches::get_optimal_hits<int>(m, vec.begin(), vec.end());
  auto finish = std::chrono::high_resolution_clock::now();
  auto elapsed = std::chrono::duration<double, std::milli>(finish - start);

  std::cout << hits << "\n";

#ifdef BOOST_FOUND__
  if (vm.count("count-time")) {
    std::cout << "Time elapsed" << elapsed.count() << " ms\n";
  }
#endif
}
