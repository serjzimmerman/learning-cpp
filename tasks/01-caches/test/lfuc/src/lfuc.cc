#include <chrono>
#include <iostream>

#ifdef BOOST_FOUND__
#include <boost/program_options.hpp>
#include <boost/program_options/option.hpp>
namespace po = boost::program_options;
#endif

#include "belady.hpp"
#include "stl_lfu.hpp"

struct slow_getter_t {
  int operator()(int p_key) {
    // The answer to “the Ultimate Question of Life, the Universe, and
    // Everything.”
    return 42;
  }
};

int main(int argc, char *argv[]) {
  if (!std::cin || !std::cout) {
    std::abort();
  }

#ifdef BOOST_FOUND__
  po::options_description desc("Available options");
  desc.add_options()("help,h", "Print this help message")(
      "verbose,v", "Output verbose")("count-time,t",
                                     "Print perfomance metrics");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }
#endif

  std::size_t n{}, m{};

  std::cin >> m >> n;

  if (n == 0 || m == 0) {
    std::abort();
  }

  std::vector<int> vec{};
  vec.reserve(n);

#ifdef BOOST_FOUND__
  bool verbose = vm.count("verbose");

#endif
  for (unsigned i = 0; i < n; i++) {
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

  caches::lfu_t<int, int> cache{m};
  slow_getter_t g{};

  auto lfu_start = std::chrono::high_resolution_clock::now();

  for (const auto &elem : vec) {
    cache.lookup(elem, g);
  }

  auto lfu_finish = std::chrono::high_resolution_clock::now();
  auto lfu_elapsed =
      std::chrono::duration<double, std::milli>(lfu_finish - lfu_start);

#ifdef BOOST_FOUND__
  bool count_time = vm.count("count-time");
  if (count_time) {
    std::cout << "Time elapsed for LFU: " << lfu_elapsed.count() << " ms\n";
  }

  if (verbose) {
    auto optimal_start = std::chrono::high_resolution_clock::now();
    auto optimal_hits =
        caches::get_optimal_hits<int>(m, vec.begin(), vec.end());
    auto optimal_finish = std::chrono::high_resolution_clock::now();
    auto optimal_elapsed = std::chrono::duration<double, std::milli>(
        optimal_finish - optimal_start);

    if (count_time) {
      std::cout << "Time elapsed for Belady: " << optimal_elapsed.count()
                << " ms\n";
    }

    std::cout << "LFU hits: " << cache.get_hits()
              << "\nMaximum possible hits: " << optimal_hits << "\n";
    return 0;
  }
#endif

  std::cout << cache.get_hits() << std::endl;
}
