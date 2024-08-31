#include <chrono>
#include <iostream>

#include "splay_order_set.hpp"

template <typename T>
typename throttle::splay_order_set<T>::size_type
get_count_less_than(throttle::splay_order_set<T> &p_set, const T &p_key) {
  if (p_set.empty()) {
    return 0;
  }

  auto min = *p_set.min();
  if (p_key <= min) {
    return 0;
  }

  auto bound = std::prev(p_set.lower_bound(p_key));
  auto rank = p_set.get_rank_of(bound);
  return rank;
}

int main() {
  if (!std::cin || !std::cout) {
    std::abort();
  }
  throttle::splay_order_set<int> t{};

  bool valid = true;
  while (valid) {
    char query_type = 0;
    int key = 0;

    if (!(std::cin >> query_type >> key)) {
      break;
    }

    try {
      switch (query_type) {
      case 'k':
        t.insert(key);
        break;
      case 'm':
        std::cout << *t.select_rank(key) << " ";
        break;
      case 'n':
        std::cout << get_count_less_than(t, key) << " ";
        break;
      default:
        std::cout << "Invalid operation";
        valid = false;
      }
    } catch (std::exception &e) {
      std::cout << e.what();
      break;
    }
  }

  std::cout << "\n";
}
