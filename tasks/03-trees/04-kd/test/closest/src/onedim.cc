#include <chrono>
#include <iostream>

#include "kd_tree.hpp"
#include "point_n.hpp"

#include <cstdlib>
#include <range/v3/all.hpp>

namespace {

template <typename T, std::size_t N>
struct indexed_point_n : public throttle::point_n<T, N> {
  unsigned index;
};

using point1 = indexed_point_n<float, 1>;

auto closest(auto &&range, auto val) {
  auto cl = ranges::lower_bound(range, val, std::less<float>{});

  if (cl == range.begin()) {
    return cl;
  } else if (cl == range.end()) {
    return std::prev(cl);
  } else {
    return (std::abs(val - *cl) < std::abs(val - *std::prev(cl))
                ? cl
                : std::prev(cl));
  }
}

} // namespace

int main(int argc, char *argv[]) {
  std::vector<point1> points;
  throttle::kd_tree<point1> kdtree;

  unsigned n;
  if (!(std::cin >> n)) {
    std::cout << "Can't read number of points\n";
    return 1;
  }

  if (n == 0) {
    std::cout << "Number of points can't be equal to 0\n";
    return 1;
  }

  points.reserve(n);
  for (unsigned i = 0; i < n; ++i) {
    point1 point;

    if (!(std::cin >> point)) {
      std::cout << "Can't read point " << i << "\n";
      return 1;
    }

    point.index = i;
    kdtree.insert(point);
    points.push_back(point);
  }

  unsigned m;
  if (!(std::cin >> m)) {
    std::cout << "Can't read number of requests\n";
    return 1;
  }

  std::vector<point1> reqs;
  reqs.reserve(m);

  for (unsigned i = 0; i < m; ++i) {
    point1 point;
    if (!(std::cin >> point)) {
      std::cout << "Can't read point " << i << "\n";
      return 1;
    }

    reqs.push_back(point);
  }

  std::chrono::duration<double, std::milli> kdduration{},
      lower_bound_duration{};
  ranges::sort(points, std::less<float>{}, [](auto &&elem) { return elem[0]; });

  kdtree.nearest_neighbour(reqs[0]);

  bool match = true;
  for (const auto &q : reqs) {
    auto start_it = std::chrono::high_resolution_clock::now();
    auto closest_kd = kdtree.nearest_neighbour(q)[0];
    auto end_it = std::chrono::high_resolution_clock::now();

    kdduration += (end_it - start_it);

    start_it = std::chrono::high_resolution_clock::now();
    auto projected_range =
        ranges::views::transform(points, [](auto &&val) { return val[0]; });
    auto closest_std = *closest(projected_range, q[0]);
    end_it = std::chrono::high_resolution_clock::now();
    lower_bound_duration += (end_it - start_it);

    if (closest_std != closest_kd)
      match = false;

    std::cout << closest_kd << " - " << closest_std << " : "
              << (closest_kd == closest_std ? "match" : "do not match") << "\n";
  }

  std::cout << "kd-tree took " << kdduration.count() << " ms\n";
  std::cout << "std::lower_bound took " << lower_bound_duration.count()
            << " ms\n";
  std::cout << "answers " << (match ? "match" : "do not match") << "\n";
}
