#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <numeric>
#include <set>

#ifdef BOOST_FOUND__
#include <boost/program_options.hpp>
#include <boost/program_options/option.hpp>
namespace po = boost::program_options;
#endif

#include "splay_order_set.hpp"

std::pair<std::vector<int>, std::vector<std::pair<int, int>>> read_input() {
  int n = 0;
  if (!(std::cin >> n)) {
    std::abort();
  }

  std::vector<int> i_vec{};
  i_vec.reserve(n);

  for (int i = 0; i < n; ++i) {
    int temp = 0;
    if (!(std::cin >> temp)) {
      std::abort();
    }
    i_vec.push_back(temp);
  }

  int m = 0;
  if (!(std::cin >> m)) {
    std::abort();
  }
  std::vector<std::pair<int, int>> q_vec{};
  q_vec.reserve(m);
  for (int i = 0; i < m; ++i) {
    int temp1 = 0, temp2 = 0;
    if ((!(std::cin >> temp1 >> temp2))) {
      std::abort();
    }
    q_vec.push_back({temp1, temp2});
  }

  return {i_vec, q_vec};
}

template <typename Set, typename T>
int set_range_query(Set &p_set, T p_first, T p_second) {
  if (p_first > p_second)
    return 0;
  auto its = p_set.lower_bound(p_first);
  auto ite = p_set.upper_bound(p_second);
  return std::distance(its, ite);
}

template <typename T>
int my_set_range_query(throttle::splay_order_set<T> &p_set, T p_first,
                       T p_second) {
  if (p_set.empty() || p_first > p_second || p_first > *p_set.max()) {
    return 0;
  }
  auto its = p_set.lower_bound(p_first);
  auto ite = p_set.upper_bound(p_second);
  int rank_left = (its == p_set.end() ? 0 : p_set.get_rank_of(its));
  int rank_right =
      (ite == p_set.end() ? p_set.size() + 1 : p_set.get_rank_of(ite));
  return rank_right - rank_left;
}

int main(int argc, char *argv[]) {
  auto [i_vec, q_vec] = read_input();

  std::vector<int> my_set_ans{}, set_ans{};
  my_set_ans.reserve(q_vec.size());
  set_ans.reserve(q_vec.size());

  throttle::splay_order_set<int> t{};
  for (const auto &v : i_vec)
    t.insert(v);

  std::set<int> s{};
  for (const auto &v : i_vec)
    s.insert(v);

  auto my_set_start = std::chrono::high_resolution_clock::now();
  for (const auto &q : q_vec) {
    auto &&r = my_set_range_query(t, q.first, q.second);
    asm("" ::"r"(r));
  }
  auto my_set_finish = std::chrono::high_resolution_clock::now();
  auto my_set_elapsed =
      std::chrono::duration<double, std::milli>(my_set_finish - my_set_start);

  auto set_start = std::chrono::high_resolution_clock::now();
  for (const auto &q : q_vec) {
    auto &&r = set_range_query(s, q.first, q.second);
    asm("" ::"r"(r));
  }
  auto set_finish = std::chrono::high_resolution_clock::now();
  auto set_elapsed =
      std::chrono::duration<double, std::milli>(set_finish - set_start);

  std::cout << "throttle::splay_set took " << my_set_elapsed.count()
            << "ms to run\n";
  std::cout << "std::set took " << set_elapsed.count() << "ms to run\n";
}
