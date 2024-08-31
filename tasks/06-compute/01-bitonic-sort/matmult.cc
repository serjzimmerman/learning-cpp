/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "opencl_include.hpp"
#include "selector.hpp"
#include "utils.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <random>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>

#include <type_traits>

#include "popl.hpp"

#include "linmath/contiguous_matrix.hpp"

#include "kernelhpp/matmult_naive_kernel.hpp"
#include "kernelhpp/matmult_tiled_arb_kernel.hpp"
#include "kernelhpp/matmult_tiled_kernel.hpp"

#define STRINGIFY0(v) #v
#define STRINGIFY(v) STRINGIFY0(v)

#ifndef TYPE__
#define TYPE__ int
#endif

#ifdef EIGEN_MAT_MULT
#include <Eigen/Dense>
using eigen_matrix_type =
    Eigen::Matrix<TYPE__, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
#endif

namespace linmath = throttle::linmath;

using matrix_type = linmath::contiguous_matrix<TYPE__>;

namespace app {

struct matrix_sizes {
  matrix_type::size_type ax, ay, by;
};

struct ndrange_query {
  cl::NDRange global, local;
};

using clutils::profiling_info;
class i_matmult {
public:
  virtual matrix_type operator()(const matrix_type &, const matrix_type &,
                                 profiling_info *) = 0;

  matrix_type multiply(const matrix_type &mata, const matrix_type &matb,
                       profiling_info *time = nullptr) {
    return operator()(mata, matb, time);
  }

  virtual ~i_matmult() {}
};

class gpu_matmult : public i_matmult, protected clutils::platform_selector {
protected:
  cl::Context m_ctx;
  cl::CommandQueue m_queue;

protected:
  static constexpr clutils::platform_version c_api_version = {2, 2};

  gpu_matmult()
      : clutils::platform_selector{c_api_version}, m_ctx{m_device},
        m_queue{m_ctx, cl::QueueProperties::Profiling} {}

  using func_signature = cl::Event(cl::Buffer, cl::Buffer, cl::Buffer);
  matrix_type run_boilerplate(const matrix_type &mata, const matrix_type &matb,
                              std::function<func_signature> func,
                              profiling_info *time) {
    if (mata.cols() != matb.rows())
      throw std::invalid_argument{"Mismatched matrix sizes"};

    const auto mat_size = [](const auto &m) {
      return std::distance(m.begin(), m.end());
    };
    const auto mat_bin_size = [&mat_size](const auto &m) {
      return mat_size(m) * sizeof(matrix_type::value_type);
    };

    auto wall_start = std::chrono::high_resolution_clock::now();

    matrix_type matc = {mata.rows(), matb.cols()};

    cl::Buffer bufa = {m_ctx, CL_MEM_READ_ONLY, mat_bin_size(mata)};
    cl::Buffer bufb = {m_ctx, CL_MEM_READ_ONLY, mat_bin_size(matb)};
    cl::Buffer bufc = {m_ctx, CL_MEM_WRITE_ONLY, mat_bin_size(matc)};

    cl::copy(m_queue, mata.begin(), mata.end(), bufa);
    cl::copy(m_queue, matb.begin(), matb.end(), bufb);

    auto event = func(bufa, bufb, bufc);
    event.wait();
    cl::copy(m_queue, bufc, matc.begin(), matc.end());
    auto wall_end = std::chrono::high_resolution_clock::now();

    std::chrono::nanoseconds pure_start{
        event.getProfilingInfo<CL_PROFILING_COMMAND_START>()},
        pure_end{event.getProfilingInfo<CL_PROFILING_COMMAND_END>()};

    auto pure = std::chrono::duration_cast<std::chrono::milliseconds>(
        pure_end - pure_start);
    auto wall = std::chrono::duration_cast<std::chrono::milliseconds>(
        wall_end - wall_start);

    if (time)
      *time = {pure, wall};
    return matc;
  }
};

class naive_matmult : public gpu_matmult {
  using kernel = matmult_naive_kernel;

private:
  cl::Program m_program;
  kernel::functor_type m_functor;

public:
  naive_matmult()
      : gpu_matmult{},
        m_program{m_ctx, kernel::source(STRINGIFY(TYPE__)), true},
        m_functor{m_program, kernel::entry()} {}

  matrix_type operator()(const matrix_type &mata, const matrix_type &matb,
                         profiling_info *time = nullptr) override {
    const auto func = [&](auto bufa, auto bufb, auto bufc) {
      cl::EnqueueArgs args = {m_queue, {mata.rows(), matb.cols()}};
      return m_functor(args, bufa, bufb, bufc, mata.rows(), mata.cols(),
                       matb.cols());
    };
    return gpu_matmult::run_boilerplate(mata, matb, func, time);
  }
};

class tiled_matmult : public gpu_matmult {
  using kernel = matmult_tiled_kernel;

private:
  cl::Program m_program;
  kernel::functor_type m_functor;

  unsigned m_tile_size;

public:
  tiled_matmult(unsigned tile_size)
      : gpu_matmult{},
        m_program{m_ctx, kernel::source(STRINGIFY(TYPE__), tile_size), true},
        m_functor{m_program, kernel::entry()}, m_tile_size{tile_size} {}

  matrix_type operator()(const matrix_type &mata, const matrix_type &matb,
                         profiling_info *time = nullptr) {
    if (mata.rows() % m_tile_size != 0 || mata.cols() % m_tile_size != 0 ||
        matb.cols() % m_tile_size != 0 || matb.rows() % m_tile_size != 0)
      throw std::invalid_argument{
          "Matrix sizes should be divisible by the tile size"};

    const auto func = [&](auto bufa, auto bufb, auto bufc) {
      cl::EnqueueArgs args = {
          m_queue, {mata.rows(), matb.cols()}, {m_tile_size, m_tile_size}};
      return m_functor(args, bufa, bufb, bufc, mata.rows(), mata.cols(),
                       matb.cols());
    };

    return gpu_matmult::run_boilerplate(mata, matb, func, time);
  }
};

class tiled_arbitrary_matmult : public gpu_matmult {
  using kernel = matmult_tiled_arb_kernel;

private:
  cl::Program m_program;
  kernel::functor_type m_functor;

  unsigned m_tile_size;

public:
  tiled_arbitrary_matmult(unsigned tile_size)
      : gpu_matmult{},
        m_program{m_ctx, kernel::source(STRINGIFY(TYPE__), tile_size), true},
        m_functor{m_program, kernel::entry()}, m_tile_size{tile_size} {}

  matrix_type operator()(const matrix_type &mata, const matrix_type &matb,
                         profiling_info *time = nullptr) override {
    const auto func = [&](auto bufa, auto bufb, auto bufc) {
      const auto tile_sz = m_tile_size;
      const auto recalc_size = [tile_sz](auto sz) {
        if (sz % tile_sz == 0)
          return sz / tile_sz;
        return (sz / tile_sz + 1);
      };

      auto recalc_rows = recalc_size(mata.rows()) * tile_sz;
      auto recalc_cols = recalc_size(matb.cols()) * tile_sz;

      cl::EnqueueArgs args = {
          m_queue, {recalc_rows, recalc_cols}, {tile_sz, tile_sz}};

      int tile_count = recalc_size(mata.cols());
      return m_functor(args, bufa, bufb, bufc, mata.rows(), mata.cols(),
                       matb.cols(), tile_count);
    };
    return gpu_matmult::run_boilerplate(mata, matb, func, time);
  }
};

} // namespace app

namespace {

#ifdef EIGEN_MAT_MULT
eigen_matrix_type to_eigen_matrix(const matrix_type &matrix) {
  eigen_matrix_type e = Eigen::Map<const eigen_matrix_type>(
      matrix.data(), matrix.rows(), matrix.cols());
  return e;
}
#endif

} // namespace

int main(int argc, char *argv[]) try {
  popl::OptionParser op("Avaliable options");
  auto help_option =
      op.add<popl::Switch>("h", "help", "Print this help message");
  auto print_option = op.add<popl::Switch>("p", "print", "Print on failure");
  auto eigen_option = op.add<popl::Switch>(
      "e", "eigen", "Compare with Eigen matrix multiplication");
  auto skip_option =
      op.add<popl::Switch>("s", "skip", "Skip naive cpu calculation");

  auto lower_option =
      op.add<popl::Implicit<TYPE__>>("", "lower", "Lower bound", -32);
  auto upper_option =
      op.add<popl::Implicit<TYPE__>>("", "upper", "Upper bound", +32);

  auto ax_option = op.add<popl::Implicit<unsigned>>(
      "", "ax", "Number of rows in matrix A", 512);
  auto ay_option = op.add<popl::Implicit<unsigned>>(
      "", "ay", "Number of cols in matrix A", 512);
  auto by_option = op.add<popl::Implicit<unsigned>>(
      "", "by", "Number of cols in matrix B", 512);

  auto kernel_option = op.add<popl::Implicit<std::string>>(
      "", "kernel", "Which kernel to use: naive, tiled, tiledarb", "naive");
  auto lsz_option =
      op.add<popl::Implicit<unsigned>>("", "lsz", "Local tile size", 256);

  op.parse(argc, argv);

  if (help_option->is_set()) {
    std::cout << op << "\n ";
    return EXIT_SUCCESS;
  }

  const auto lower = lower_option->value(), upper = upper_option->value();
  const auto kernel_name = kernel_option->value();
  const auto lsz = lsz_option->value();
  const auto ax = ax_option->value(), ay = ay_option->value(),
             by = by_option->value();

  if (lower >= upper) {
    std::cout << "Error: lower bound can't be greater than the upper bound\n";
    return EXIT_FAILURE;
  }

  const bool skip_cpu = skip_option->is_set();
  const bool print_on_failure = print_option->is_set();
  const bool compare_eigen = eigen_option->is_set();

#ifndef EIGEN_MAT_MULT
  if (compare_eigen)
    std::cout
        << "Warning: app wasn't built with Eigen, ignoring --eigen option\n";
#endif

  std::unique_ptr<app::i_matmult> mult;
  if (kernel_name == "naive") {
    mult = std::make_unique<app::naive_matmult>();
  } else if (kernel_name == "tiled") {
    mult = std::make_unique<app::tiled_matmult>(lsz);
  } else if (kernel_name == "tiledarb") {
    mult = std::make_unique<app::tiled_arbitrary_matmult>(lsz);
  } else {
    std::cout << "Unknown type of kernel: " << kernel_name << "\n";
    return EXIT_FAILURE;
  }

  if (kernel_name == "naive" && lsz_option->is_set()) {
    std::cout << "Warning: local size provided but kernel used is \"naive\", "
                 "ignoring --lsz option\n";
  }

  const auto print_sep = []() { std::cout << " -------- \n"; };
  std::cout << "Multiplying A [" << ax << " x " << ay << "] by B [" << ay
            << " x " << by << "]\n";
  print_sep();

  matrix_type a{ax, ay}, b{ay, by};

  auto random_filler =
      clutils::create_random_number_generator<matrix_type::value_type>(lower,
                                                                       upper);
  random_filler(a);
  random_filler(b);

  std::chrono::milliseconds wall_cpu_naive;

  const auto measure_cpu_time = [](auto func) {
    auto wall_start = std::chrono::high_resolution_clock::now();
    func();
    auto wall_end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(wall_end -
                                                                 wall_start);
  };

  matrix_type c;
  if (!skip_cpu) {
    wall_cpu_naive = measure_cpu_time([&a, &b, &c]() { c = a * b; });
  }

#ifdef EIGEN_MAT_MULT
  std::chrono::milliseconds wall_cpu_eigen;
  if (compare_eigen) {
    eigen_matrix_type a_e = to_eigen_matrix(a), b_e = to_eigen_matrix(b), c_e;
    wall_cpu_eigen =
        measure_cpu_time([&a_e, &b_e, &c_e]() { c_e = a_e * b_e; });
  }
#endif

  static const auto matrix_print = [](auto name, auto &mat) {
    std::cout << name << " : \n";
    for (unsigned i = 0; i < mat.rows(); ++i) {
      for (auto v : mat[i]) {
        std::cout << v << "\t";
      }
      std::cout << "\n";
    }
  };

  app::profiling_info prof_info;

  auto res = mult->multiply(a, b, &prof_info);
  if (!skip_cpu)
    std::cout << "CPU wall time: " << wall_cpu_naive.count() << " ms\n";

#ifdef EIGEN_MAT_MULT
  if (compare_eigen)
    std::cout << "Eigen wall time: " << wall_cpu_eigen.count() << " ms\n";
#endif

  std::cout << "GPU wall time: " << prof_info.wall.count() << " ms\n";
  std::cout << "GPU pure time: " << prof_info.pure.count() << " ms\n";

  print_sep();

  const auto validate_results = [&c, &res, &a, &b, print_on_failure]() {
    if (c == res) {
      std::cout << "GPU matrix multiplication works fine\n";
      return EXIT_SUCCESS;
    }

    std::cout << "GPU matrix multiplication is borked\n";
    if (!print_on_failure)
      return EXIT_FAILURE;

    matrix_print("Matrix A", a);
    matrix_print("Matrix B", b);

    matrix_print("Matrix from GPU", res);
    matrix_print("Correct", c);

    return EXIT_FAILURE;
  };

  if (skip_cpu)
    return EXIT_SUCCESS;
  return validate_results();
} catch (cl::BuildError &e) {
  std::cerr << "Compilation failed:\n";
  for (const auto &v : e.getBuildLog()) {
    std::cerr << v.second << "\n";
  }
} catch (cl::Error &e) {
  std::cerr << "OpenCL error: " << e.what() << "(" << e.err() << ")\n";
} catch (std::exception &e) {
  std::cerr << "Encountered error: " << e.what() << "\n";
} catch (...) {
  std::cerr << "Unknown error\n";
}
