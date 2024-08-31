#include <chrono>
#include <iostream>

#define RECURSIVE_FIND_SET
#include "offline_rmq.hpp"

#ifdef BOOST_FOUND__
#include <boost/program_options.hpp>
#include <boost/program_options/option.hpp>
namespace po = boost::program_options;
#endif

#include <cstdlib>

int main(int argc, char *argv[]) {
#ifdef BOOST_FOUND__
  po::options_description desc("Available options");
  desc.add_options()("help,h", "Print this help message")(
      "measure,m", "Print perfomance metrics")("hide", "Hide output");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  bool measure = vm.count("measure");
  bool hide = vm.count("hide");
#endif

  unsigned n, m;
  if (!(std::cin >> n)) {
    std::cout << "Can't read number of elements \"n\"\n";
    return 1;
  }

  std::vector<int> vec{};
  vec.reserve(n);

  for (unsigned i = 0; i < n; ++i) {
    int temp;
    if (!(std::cin >> temp)) {
      std::cout << "Can't read element at index " << i << " out of " << n
                << "\n";
      return 1;
    }
    vec.push_back(temp);
  }

  if (!(std::cin >> m)) {
    std::cout << "Can't read number of queries \"m\"\n";
    return 1;
  }

  size_t max_index = vec.size() - 1;
  std::vector<std::pair<unsigned, unsigned>> q_vec{};
  q_vec.reserve(m);

  for (unsigned i = 0; i < m; ++i) {
    unsigned left, right;
    if (!(std::cin >> left >> right)) {
      std::cout << "Can't read query at index " << i << " out of " << m << "\n";
    }
    if ((left >= right) || (left > max_index) || (right > max_index)) {
      std::cout << "[" << left << ", " << right
                << "] in not a valid range in array of size " << n << "\n";
      return 1;
    }
    q_vec.push_back({left, right});
  }

  auto start = std::chrono::high_resolution_clock::now();
  auto ans = throttle::iterative_offline_rmq<int, std::less<int>>(
      vec.begin(), vec.end(), q_vec.begin(), q_vec.end());
  auto finish = std::chrono::high_resolution_clock::now();
  auto elapsed = std::chrono::duration<double, std::milli>(finish - start);

#ifdef BOOST_FOUND__
  if (!hide) {
    for (const auto &v : ans) {
      std::cout << vec.at(v) << " ";
    }
    std::cout << "\n";
  }

  if (measure) {
    std::cout << "throttle::iterative_offline_rmq took " << elapsed.count()
              << "ms to run\n";
  }
#else
  for (const auto &v : ans) {
    std::cout << vec.at(v) << " ";
  }

  std::cout << "\n";
#endif
}
