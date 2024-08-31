#include <chrono>
#include <iostream>

#include "geometry/broadphase/broadphase_structure.hpp"
#include "geometry/broadphase/bruteforce.hpp"
#include "geometry/broadphase/octree.hpp"
#include "geometry/broadphase/uniform_grid.hpp"

#include "geometry/narrowphase/collision_shape.hpp"
#include "geometry/primitives/plane.hpp"
#include "geometry/primitives/triangle3.hpp"
#include "geometry/vec3.hpp"

#include <chrono>
#include <cmath>
#include <set>
#include <string>
#include <vector>

#ifdef BOOST_FOUND__
#include <boost/program_options.hpp>
#include <boost/program_options/option.hpp>
namespace po = boost::program_options;
#endif

struct indexed_geom : public throttle::geometry::collision_shape<float> {
  unsigned index;
  indexed_geom(unsigned idx, auto &&base)
      : collision_shape{base}, index{idx} {};
};

using throttle::geometry::collision_shape;
using throttle::geometry::is_roughly_equal;
using throttle::geometry::point3;
using throttle::geometry::segment3;
using throttle::geometry::triangle3;
using throttle::geometry::vec3;

template <typename T>
throttle::geometry::collision_shape<T>
shape_from_three_points(const point3<T> &a, const point3<T> &b,
                        const point3<T> &c) {
  auto ab = b - a, ac = c - a;

  if (throttle::geometry::colinear(ab, ac)) { // Either a segment or a point
    if (is_roughly_equal(ab, vec3<T>::zero()) &&
        is_roughly_equal(ac, vec3<T>::zero())) {
      return throttle::geometry::barycentric_average<T>(a, b, c);
    }
    // This is a segment. Project the the points onto the most closely alligned
    // axis.
    auto max_index = ab.max_component().first;

    std::array<std::pair<point3<T>, T>, 3> arr = {
        std::make_pair(a, a[max_index]), std::make_pair(b, b[max_index]),
        std::make_pair(c, c[max_index])};
    std::sort(arr.begin(), arr.end(),
              [](const auto &left, const auto &right) -> bool {
                return left.second < right.second;
              });
    return segment3<T>{arr[0].first, arr[2].first};
  }

  return triangle3<T>{a, b, c};
}

static unsigned apporoximate_optimal_depth(unsigned number) {
  constexpr unsigned max_depth = 6;
  unsigned log_num = std::log10(float(number));
  return std::min(max_depth, log_num);
}

template <typename broad>
bool application_loop(
    throttle::geometry::broadphase_structure<broad, indexed_geom> &cont,
    unsigned n, bool hide = false) {
  using point_type = throttle::geometry::point3<float>;

  for (unsigned i = 0; i < n; ++i) {
    point_type a, b, c;
    if (!(std::cin >> a[0] >> a[1] >> a[2] >> b[0] >> b[1] >> b[2] >> c[0] >>
          c[1] >> c[2])) {
      std::cout << "Can't read i-th = " << i << " triangle\n";
      return false;
    }
    cont.add_collision_shape({i, shape_from_three_points(a, b, c)});
  }

  auto result = cont.many_to_many();
  if (hide)
    return true;

  for (const auto v : result)
    std::cout << v->index << " ";

  std::cout << "\n";
  return true;
}

int main(int argc, char *argv[]) {
  bool hide = false;

#ifdef BOOST_FOUND__
  std::string opt;
  po::options_description desc("Available options");
  desc.add_options()("help,h", "Print this help message")(
      "measure,m", "Print perfomance metrics")("hide", "Hide output")(
      "broad", po::value<std::string>(&opt)->default_value("octree"),
      "Algorithm for broad phase (bruteforce, octree, uniform-grid)");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  bool measure = vm.count("measure");
  hide = vm.count("hide");
#endif

  unsigned n;
  if (!(std::cin >> n)) {
    std::cout << "Can't read number of triangles\n";
    return 1;
  }

#ifdef BOOST_FOUND__
  auto start = std::chrono::high_resolution_clock::now();

  if (opt == "octree") {
    throttle::geometry::octree<float, indexed_geom> octree{
        apporoximate_optimal_depth(n)};
    if (!application_loop(octree, n, hide))
      return 1;
  } else if (opt == "bruteforce") {
    throttle::geometry::bruteforce<float, indexed_geom> bruteforce{n};
    if (!application_loop(bruteforce, n, hide))
      return 1;
  } else if (opt == "uniform-grid") {
    throttle::geometry::uniform_grid<float, indexed_geom> uniform{n};
    if (!application_loop(uniform, n, hide))
      return 1;
  }

  auto finish = std::chrono::high_resolution_clock::now();
  auto elapsed = std::chrono::duration<double, std::milli>(finish - start);

  if (measure) {
    std::cout << opt << " took " << elapsed.count() << "ms to run\n";
  }

#else
  throttle::geometry::octree<float, indexed_geom> octree{
      apporoximate_optimal_depth(n)};
  application_loop(octree, n, hide);
#endif
}
