#include <chrono>
#include <iostream>

#include "offline_rmq.hpp"

#include <cstdlib>

int main() {
  unsigned n, m;
  if (!(std::cin >> n)) {
    std::cout << "Invalid input\n";
    return 1;
  }

  std::vector<int> vec{};
  vec.reserve(n);

  for (unsigned i = 0; i < n; ++i) {
    int temp;
    if (!(std::cin >> temp)) {
      std::cout << "Invalid input\n";
      return 1;
    }
    vec.push_back(temp);
  }

  if (!(std::cin >> m)) {
    std::cout << "Invalid input\n";
    return 1;
  }

  size_t max_index = vec.size() - 1;
  std::vector<std::pair<unsigned, unsigned>> q_vec{};
  q_vec.reserve(m);

  for (unsigned i = 0; i < m; ++i) {
    unsigned left, right;
    if (!(std::cin >> left >> right) || (left >= right) || (left > max_index) ||
        (right > max_index)) {
      std::cout << "Invalid input\n";
      return 1;
    }
    q_vec.push_back({left, right});
  }

  auto recursive_start = std::chrono::high_resolution_clock::now();
  auto &&ans_rec = throttle::recursive_offline_rmq<int, std::less<int>>(
      vec.begin(), vec.end(), q_vec.begin(), q_vec.end());
  auto recursive_finish = std::chrono::high_resolution_clock::now();
  auto recursive_elapsed = std::chrono::duration<double, std::milli>(
      recursive_finish - recursive_start);

  auto iterative_start = std::chrono::high_resolution_clock::now();
  auto &&ans_iter = throttle::iterative_offline_rmq<int, std::less<int>>(
      vec.begin(), vec.end(), q_vec.begin(), q_vec.end());
  auto iterative_finish = std::chrono::high_resolution_clock::now();
  auto iterative_elapsed = std::chrono::duration<double, std::milli>(
      iterative_finish - iterative_start);

#if COMPARE_OUTPUTS
  std::cout << "Outputs of throttle::recursive_offline_rmq and "
               "throttle::iterative_offline_rmq "
            << (ans_rec == ans_iter ? "match\n" : "differ\n");
#endif

  std::cout << "throttle::recursive_offline_rmq took "
            << recursive_elapsed.count() << "ms to run\n";
  std::cout << "throttle::iterative_offline_rmq " << iterative_elapsed.count()
            << "ms to run\n";
}
