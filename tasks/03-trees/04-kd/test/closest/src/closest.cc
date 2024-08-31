#include <chrono>
#include <iostream>

#include "kd_tree.hpp"
#include "point_n.hpp"

#include <cstdlib>
#include <range/v3/all.hpp>
#include <string>

namespace {

template <typename T, std::size_t N>
struct indexed_point_n : public throttle::point_n<T, N> {
  unsigned index;
};

using point4 = indexed_point_n<float, 4>;

} // namespace

int main(int argc, char *argv[]) {
  std::vector<point4> points;
  throttle::kd_tree<point4> kdtree;

  int n;
  if (!(std::cin >> n)) {
    std::cout << "Can't read number of points\n";
    return 1;
  }

  if (n <= 0) {
    std::cout << "Number of points can't be less or equal to 0\n";
    return 1;
  }

  points.reserve(n);
  for (int i = 0; i < n; ++i) {
    point4 point;

    if (!(std::cin >> point)) {
      std::cout << "Can't read point " << i << "\n";
      return 1;
    }

    point.index = i;
    kdtree.insert(point);
  }

  int m;
  if (!(std::cin >> m)) {
    std::cout << "Can't read number of points\n";
    return 1;
  }

  if (m < 0) {
    std::cout << "Number of queries can't be less than 0\n";
    return 1;
  }

  std::vector<point4> reqs;
  reqs.reserve(m);

  for (int i = 0; i < m; ++i) {
    point4 point;
    if (!(std::cin >> point)) {
      std::cout << "Can't read point " << i << "\n";
      return 1;
    }

    reqs.push_back(point);
  }

  for (const auto &q : reqs) {
    auto closest = kdtree.nearest_neighbour(q).index;
    std::cout << closest << "\n";
  }
}
