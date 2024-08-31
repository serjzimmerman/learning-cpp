#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <numeric>
#include <set>

template <typename T>
int set_range_query(const std::set<T> &p_set, T p_first, T p_second) {
  if (p_first > p_second)
    return 0;
  auto its = p_set.lower_bound(p_first);
  auto ite = p_set.upper_bound(p_second);
  return std::distance(its, ite);
}

int main() {
  int n = 0;
  if (!(std::cin >> n)) {
    std::abort();
  }

  std::set<int> s{};

  for (int i = 0; i < n; ++i) {
    int temp = 0;
    if (!(std::cin >> temp)) {
      std::abort();
    }
    s.insert(temp);
  }

  int m = 0;
  if (!(std::cin >> m)) {
    std::abort();
  }

  for (int i = 0; i < m; ++i) {
    int temp1 = 0, temp2 = 0;
    if ((!(std::cin >> temp1 >> temp2))) {
      std::abort();
    }
    std::cout << set_range_query(s, temp1, temp2) << " ";
  }
}
