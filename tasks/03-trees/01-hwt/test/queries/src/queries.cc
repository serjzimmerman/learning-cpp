#include <chrono>
#include <iostream>

#include <order_statistic_set.hpp>

template <typename T>
typename throttle::order_statistic_set<T>::size_type
get_count_less_than(const throttle::order_statistic_set<T> &p_set,
                    const T &p_key) {
  if (p_set.empty()) {
    return 0;
  }

  auto min = p_set.min();
  if (p_key <= min) {
    return 0;
  }

  auto bound = p_set.closest_left(p_key);
  auto rank = p_set.get_rank_of(bound);
  return (bound == p_key ? rank - 1 : rank);
}

int main() {
  if (!std::cin || !std::cout) {
    std::abort();
  }
  throttle::order_statistic_set<int> t{};

  bool valid = true;
  while (valid) {
    char query_type;
    int key;

    if (!(std::cin >> query_type >> key)) {
      break;
    }

    try {
      switch (query_type) {
      case 'k':
        t.insert(key);
        break;
      case 'm':
        std::cout << t.select_rank(key) << " ";
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
