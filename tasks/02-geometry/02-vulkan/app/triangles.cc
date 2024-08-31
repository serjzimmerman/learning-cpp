/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "unified_includes/glfw_include.hpp"
#include "unified_includes/vulkan_hpp_include.hpp"

#include "ezvk/debug.hpp"
#include "ezvk/error.hpp"
#include "ezvk/window.hpp"
#include "ezvk/wrappers/device.hpp"

#include "application.hpp"
#include "config.hpp"
#include "misc/vertex.hpp"

#include "geometry/broadphase/broadphase_structure.hpp"
#include "geometry/broadphase/bruteforce.hpp"
#include "geometry/broadphase/octree.hpp"
#include "geometry/broadphase/uniform_grid.hpp"

#include "geometry/narrowphase/collision_shape.hpp"
#include "geometry/primitives/plane.hpp"
#include "geometry/primitives/triangle3.hpp"
#include "geometry/vec3.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <concepts>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/program_options/option.hpp>
namespace po = boost::program_options;

#include <spdlog/cfg/env.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace {

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

using point_type = typename throttle::geometry::point3<float>;
using triangle_type = typename throttle::geometry::triangle3<float>;

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

using triangles_vertices_type = std::vector<triangles::triangle_vertex_type>;
using wireframe_vertices_type = std::vector<triangles::wireframe_vertex_type>;

template <typename T>
auto convert_to_cube_edges(const glm::vec3 &min_corner, T width,
                           uint32_t color_index) {
  // Here's the cube:
  //
  //    f -- g     b -- c
  //    -    -     -    -  (bottom layer)
  //    e -- h     a -- d
  //

  const auto a = min_corner, b = a + glm::vec3{0, width, 0};
  const auto d = a + glm::vec3{width, 0, 0}, c = a + glm::vec3{width, width, 0};

  const auto e = a + glm::vec3{0, 0, width}, f = e + glm::vec3{0, width, 0};
  const auto h = e + glm::vec3{width, 0, 0}, g = e + glm::vec3{width, width, 0};

  return std::array<triangles::wireframe_vertex_type, 24>{
      {{a, color_index}, {b, color_index}, {a, color_index}, {d, color_index},
       {b, color_index}, {c, color_index}, {d, color_index}, {c, color_index},
       {e, color_index}, {f, color_index}, {e, color_index}, {h, color_index},
       {f, color_index}, {g, color_index}, {g, color_index}, {h, color_index},
       {a, color_index}, {e, color_index}, {b, color_index}, {f, color_index},
       {c, color_index}, {g, color_index}, {d, color_index}, {h, color_index}}};
}

template <typename T>
auto convert_to_cube_edges(const glm::vec3 &min_corner, T width_x, T width_y,
                           T width_z, uint32_t color_index) {
  // Here's the cube:
  //
  //    f -- g     b -- c
  //    -    -     -    -  (bottom layer)
  //    e -- h     a -- d
  //

  const auto a = min_corner, b = a + glm::vec3{0, width_y, 0};
  const auto d = a + glm::vec3{width_x, 0, 0},
             c = a + glm::vec3{width_x, width_y, 0};

  const auto e = a + glm::vec3{0, 0, width_z}, f = e + glm::vec3{0, width_y, 0};
  const auto h = e + glm::vec3{width_x, 0, 0},
             g = e + glm::vec3{width_x, width_y, 0};

  return std::array<triangles::wireframe_vertex_type, 24>{
      {{a, color_index}, {b, color_index}, {a, color_index}, {d, color_index},
       {b, color_index}, {c, color_index}, {d, color_index}, {c, color_index},
       {e, color_index}, {f, color_index}, {e, color_index}, {h, color_index},
       {f, color_index}, {g, color_index}, {g, color_index}, {h, color_index},
       {a, color_index}, {e, color_index}, {b, color_index}, {f, color_index},
       {c, color_index}, {g, color_index}, {d, color_index}, {h, color_index}}};
}

template <typename T>
wireframe_vertices_type
fill_wireframe_vertices(throttle::geometry::bruteforce<T, indexed_geom> &) {
  // Do nothing
  return {};
}

template <typename T>
wireframe_vertices_type
fill_wireframe_vertices(throttle::geometry::octree<T, indexed_geom> &octree) {
  wireframe_vertices_type vertices;

  for (const auto &elem : octree) {
    if (elem.m_contained_shape_indexes.empty())
      continue;
    glm::vec3 min_corner = {elem.m_center[0] - elem.m_halfwidth,
                            elem.m_center[1] - elem.m_halfwidth,
                            elem.m_center[2] - elem.m_halfwidth};
    auto vertices_arr = convert_to_cube_edges(
        min_corner, elem.m_halfwidth * 2, triangles::config::wiremesh_index);
    std::copy(vertices_arr.begin(), vertices_arr.end(),
              std::back_inserter(vertices));
  }

  return vertices;
}

template <typename T>
wireframe_vertices_type fill_wireframe_vertices(
    throttle::geometry::uniform_grid<T, indexed_geom> &uniform) {
  wireframe_vertices_type vertices;
  auto cell_size = uniform.cell_size();

  for (const auto &elem : uniform) {
    glm::vec3 cell = {elem.second[0] * cell_size, elem.second[1] * cell_size,
                      elem.second[2] * cell_size};
    auto vertices_arr = convert_to_cube_edges(
        cell, cell_size, triangles::config::wiremesh_index);
    std::copy(vertices_arr.begin(), vertices_arr.end(),
              std::back_inserter(vertices));
  }

  return vertices;
}

wireframe_vertices_type
fill_bounding_box_vertices(std::span<const triangle_type> triangles) {
  wireframe_vertices_type vertices;

  for (const auto &tr : triangles) {
    auto box = throttle::geometry::axis_aligned_bb<float>{tr.a, tr.b, tr.c};
    auto min_corner = box.minimum_corner();

    glm::vec3 cell = {min_corner[0], min_corner[1], min_corner[2]};
    auto vertices_arr = convert_to_cube_edges(
        cell, 2 * box.m_halfwidth_x, 2 * box.m_halfwidth_y,
        2 * box.m_halfwidth_z, triangles::config::bbox_index);

    std::copy(vertices_arr.begin(), vertices_arr.end(),
              std::back_inserter(vertices));
  }

  return vertices;
}

struct input_result {
  bool success = false;
  std::vector<triangles::triangle_vertex_type> tr_vert;
  std::vector<triangles::wireframe_vertex_type> broad_vert, bbox_vert;
};

template <typename broad>
input_result application_loop(
    std::istream &is,
    throttle::geometry::broadphase_structure<broad, indexed_geom> &cont,
    unsigned n) {
  triangles_vertices_type vertices;
  std::vector<triangle_type> triangles;

  triangles.reserve(n);
  for (unsigned i = 0; i < n; ++i) {
    point_type a, b, c;
    if (!(is >> a[0] >> a[1] >> a[2] >> b[0] >> b[1] >> b[2] >> c[0] >> c[1] >>
          c[2])) {
      spdlog::error("Can't read i-th = {} triangle out of {}", i, n);
      return {};
    }
    cont.add_collision_shape({i, shape_from_three_points(a, b, c)});
    triangles.push_back({a, b, c});
  }

  auto result = cont.many_to_many();

  std::unordered_set<unsigned> intersecting;
  for (const auto &v : result) {
    intersecting.insert(v->index);
  }

  vertices.reserve(6 * n);
  std::for_each(triangles.begin(), triangles.end(),
                [i = 0, &intersecting, &vertices](auto triangle) mutable {
                  triangles::triangle_vertex_type vertex;

                  auto norm = triangle.norm();
                  vertex.color_index = (intersecting.contains(i)
                                            ? triangles::config::intersect_index
                                            : triangles::config::regular_index);
                  vertex.norm = {norm[0], norm[1], norm[2]};

                  vertex.pos = {triangle.a[0], triangle.a[1], triangle.a[2]};
                  vertices.push_back(vertex);
                  vertex.pos = {triangle.b[0], triangle.b[1], triangle.b[2]};
                  vertices.push_back(vertex);
                  vertex.pos = {triangle.c[0], triangle.c[1], triangle.c[2]};
                  vertices.push_back(vertex);

                  ++i;
                });

  // Here we add triangles oriented in the opposite direction to light them
  // differently
  auto sz = vertices.size();
  for (unsigned i = 0; i < sz; i += 3) {
    const auto offsets = std::array{0, 2, 1};
    for (const auto &o : offsets) {
      auto new_vertex = vertices[i + o];
      new_vertex.norm *= -1;
      vertices.push_back(new_vertex);
    }
  }

  const auto mesh_vertices = fill_wireframe_vertices(cont.impl());
  const auto bboxes_vertices = fill_bounding_box_vertices(triangles);

  return {true, vertices, mesh_vertices, bboxes_vertices};
}

} // namespace

int main(int argc, char *argv[]) try {
  auto err_logger = spdlog::stderr_color_mt("stderr");
  spdlog::set_default_logger(err_logger);
  spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");

  ezvk::enable_glfw_exceptions();
  spdlog::cfg::load_env_levels(); // Read logging level from environment
                                  // variables

  // Intersection
  std::istream *isp = &std::cin;

  std::string opt, input;
  po::options_description desc("Available options");
  desc.add_options()("help,h", "Print this help message")(
      "broad", po::value<std::string>(&opt)->default_value("octree"),
      "Algorithm for broad phase (bruteforce, octree, uniform-grid)")(
      "input,i", po::value<std::string>(&input),
      "Optional input file to use instead of stdin");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  std::ifstream ifs;
  ifs.exceptions(ifs.exceptions() | std::ifstream::badbit |
                 std::ifstream::failbit);
  if (vm.count("input")) {
    ifs.open(input);
    isp = &ifs;
  }

  glfwInit();

  constexpr float glfw_timeout = 0.25f;
  glfwWaitEventsTimeout(glfw_timeout);
  static constexpr auto app_info =
      vk::ApplicationInfo{.pApplicationName = "Hello, World!",
                          .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                          .pEngineName = "Junk Inc.",
                          .engineVersion = VK_MAKE_VERSION(1, 0, 0),
                          .apiVersion = VK_MAKE_VERSION(1, 1, 0)};

  vk::raii::Context ctx;
  const auto extensions = triangles::config::required_vk_extensions();

#ifdef USE_DEBUG_EXTENSION
  const auto layers = triangles::config::required_vk_layers(true);
#else
  const auto layers = triangles::config::required_vk_layers();
#endif

  ezvk::instance raw_instance = {ctx,
                                 app_info,
                                 extensions.begin(),
                                 extensions.end(),
                                 layers.begin(),
                                 layers.end()};

#ifdef USE_DEBUG_EXTENSION
  ezvk::generic_instance instance =
      ezvk::debugged_instance{std::move(raw_instance)};
#else
  ezvk::generic_instance instance = std::move(raw_instance);
#endif
  const auto physical_device_extensions =
      triangles::config::required_physical_device_extensions();
  auto suitable_physical_devices =
      ezvk::physical_device_selector::enumerate_suitable_physical_devices(
          instance(), physical_device_extensions.begin(),
          physical_device_extensions.end());

  if (suitable_physical_devices.empty()) {
    throw ezvk::vk_error{"No suitable physical devices found"};
  }

  auto p_device = std::move(suitable_physical_devices.front());
  auto window = ezvk::unique_glfw_window{"Triangles intersection",
                                         vk::Extent2D{800, 600}, true};
  auto surface = ezvk::surface{instance(), window};

  auto platform =
      triangles::applicaton_platform{std::move(instance), std::move(window),
                                     std::move(surface), std::move(p_device)};

  auto &app = triangles::application::instance().get(&platform);
  std::atomic_bool should_close = false, should_kill = false;

  auto rendering_thread = std::thread{[&app, &should_close, &should_kill]() {
    try {
      while (!should_close.load()) {
        app.loop();
      }

      app.shutdown();
      return;
    } catch (ezvk::error &e) {
      spdlog::error("Application encountered an error: {}", e.what());
    } catch (vk::SystemError &e) {
      spdlog::error("Vulkan error: {}", e.what());
    } catch (std::exception &e) {
      spdlog::error("Other error: {}", e.what());
    } catch (...) {
      spdlog::error("Unknown error, bailing out...");
    }

    should_kill = true;
  }};

  auto intersecting_thread = std::thread{[&app, opt, isp, &should_kill]() {
    assert(isp);

    const auto read_input = [&]() -> bool {
      unsigned n;
      if (!(*isp >> n)) {
        spdlog::error("Can't read number of triangles");
        return false;
      }

      input_result res;
      if (opt == "octree") {
        throttle::geometry::octree<float, indexed_geom> octree{
            apporoximate_optimal_depth(n)};
        if (!(res = application_loop(*isp, octree, n)).success)
          return false;
      } else if (opt == "bruteforce") {
        throttle::geometry::bruteforce<float, indexed_geom> bruteforce{n};
        if (!(res = application_loop(*isp, bruteforce, n)).success)
          return false;
      } else if (opt == "uniform-grid") {
        throttle::geometry::uniform_grid<float, indexed_geom> uniform{n};
        if (!(res = application_loop(*isp, uniform, n)).success)
          return false;
      } else
        throw std::runtime_error{"Unknown broad option"};

      triangles::input_data data = {res.tr_vert, res.broad_vert, res.bbox_vert};
      app.load_input_data(data);

      return true;
    };

    try {
      if (!read_input())
        should_kill = true;
      return;
    } catch (ezvk::error &e) {
      spdlog::error("Application encountered an error: {}", e.what());
    } catch (vk::SystemError &e) {
      spdlog::error("Vulkan error: {}", e.what());
    } catch (std::exception &e) {
      spdlog::error("Other error: {}", e.what());
    } catch (...) {
      spdlog::error("Unknown error, bailing out...");
    }

    should_kill = true;
  }};

  while (!glfwWindowShouldClose(app.window())) {
    if (should_kill)
      break;
    glfwWaitEvents();
  }

  should_close = true;
  rendering_thread.join();

  app.shutdown();
  intersecting_thread.join();
  triangles::application::instance().destroy();
} catch (ezvk::error &e) {
  spdlog::error("Application encountered an error: {}", e.what());
} catch (vk::SystemError &e) {
  spdlog::error("Vulkan error: {}", e.what());
} catch (std::exception &e) {
  spdlog::error("Other error: {}", e.what());
} catch (...) {
  spdlog::error("Unknown error, bailing out...");
}
