/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "fcl/geometry/shape/triangle_p.h"
#include "fcl/math/bv/utility.h"
#include "fcl/narrowphase/collision.h"

#include <array>
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <vector>

using triangle = fcl::TrianglePf;
using vec3 = fcl::Vector3f;

std::shared_ptr<triangle> read_triangle() {
  std::array<float, 9> points;

  for (unsigned j = 0; j < points.size(); ++j) {
    if (!(std::cin >> points[j])) {
      std::cout << "Can't read point\n";
      throw std::invalid_argument("Invalid input");
    }
  }

  return std::make_shared<triangle>(vec3{points[0], points[1], points[2]},
                                    vec3{points[3], points[4], points[5]},
                                    vec3{points[6], points[7], points[8]});
}

int main(int argc, char *argv[]) {
  unsigned n;
  if (!(std::cin >> n)) {
    std::cout << "Can't read number of triangles\n";
    return 1;
  }

  using model = fcl::BVHModel<fcl::OBBRSSf>;
  std::vector<std::shared_ptr<triangle>> vec;
  std::vector<std::shared_ptr<fcl::CollisionObjectf>> objs;

  for (unsigned i = 0; i < n; ++i) {
    vec.emplace_back(read_triangle());
    std::shared_ptr<model> geom = std::make_shared<model>();
    geom->beginModel();
    std::vector<vec3> vert = {vec.back()->a, vec.back()->b, vec.back()->c};
    std::vector<fcl::Triangle> tris = {{0, 1, 2}};
    geom->addSubModel(vert, tris);
    geom->endModel();
    objs.emplace_back(std::make_shared<fcl::CollisionObjectf>(geom));
  }

  std::set<unsigned> in_collision;
#ifndef FCL_BROADPHASE
  for (unsigned i = 0; i < n; ++i) {
    for (unsigned j = i + 1; j < n; ++j) {
      fcl::CollisionRequestf request(1, true);
      fcl::CollisionResultf result;
      bool collide =
          fcl::collide(objs.at(i).get(), objs.at(j).get(), request, result);
      if (collide) {
        in_collision.insert(i);
        in_collision.insert(j);
      }
    }
  }
#else
#error "Not yet implemented"
#endif

  for (const auto &v : in_collision) {
    std::cout << v << " ";
  }

  std::cout << "\n";
}
