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
#include <iostream>
#include <iterator>
#include <random>
#include <span>
#include <stdexcept>
#include <string>

#include "popl.hpp"

#define STRINGIFY0(v) #v
#define STRINGIFY(v) STRINGIFY0(v)

#ifndef TYPE__
#define TYPE__ int
#endif

namespace app {

static const std::string adder_kernel = std::string{"#define TYPE "} +
                                        STRINGIFY(TYPE__) + std::string{"\n"} +
                                        R"(
  __kernel void vec_add(__global TYPE *A, __global TYPE *B, __global TYPE *C) {
    int i = get_global_id(0);
    C[i] = A[i] + B[i];
})";

class vecadd : private clutils::platform_selector {
  cl::Context m_ctx;
  cl::CommandQueue m_queue;

  cl::Program m_program;
  cl::KernelFunctor<cl::Buffer, cl::Buffer, cl::Buffer> m_functor;

public:
  static constexpr clutils::platform_version c_api_version = {2, 2};

  vecadd()
      : clutils::platform_selector{c_api_version}, m_ctx{m_device},
        m_queue{m_ctx, cl::QueueProperties::Profiling},
        m_program{m_ctx, adder_kernel, true}, m_functor{m_program, "vec_add"} {}

  std::vector<TYPE__> add(std::span<const TYPE__> spa,
                          std::span<const TYPE__> spb,
                          std::chrono::microseconds *time = nullptr) {
    if (spa.size() != spb.size())
      throw std::invalid_argument{"Mismatched vector sizes"};

    const auto size = spa.size();
    const auto bin_size = clutils::sizeof_container(spa);

    std::vector<TYPE__> cvec;
    cvec.resize(size);

    cl::Buffer abuf = {m_ctx, CL_MEM_READ_ONLY, bin_size};
    cl::Buffer bbuf = {m_ctx, CL_MEM_READ_ONLY, bin_size};
    cl::Buffer cbuf = {m_ctx, CL_MEM_WRITE_ONLY, bin_size};

    cl::copy(m_queue, spa.begin(), spa.end(), abuf);
    cl::copy(m_queue, spb.begin(), spb.end(), bbuf);

    cl::NDRange global = {size};
    cl::EnqueueArgs args = {m_queue, global};

    auto evnt = m_functor(args, abuf, bbuf, cbuf);

    evnt.wait();
    std::chrono::nanoseconds time_start{
        evnt.getProfilingInfo<CL_PROFILING_COMMAND_START>()},
        time_end{evnt.getProfilingInfo<CL_PROFILING_COMMAND_END>()};

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        time_end - time_start);
    if (time)
      *time = duration;

    cl::copy(m_queue, cbuf, cvec.begin(), cvec.end());
    return cvec;
  }
};

} // namespace app

int main(int argc, char *argv[]) try {
  popl::OptionParser op("Avaliable options");
  auto help_option =
      op.add<popl::Switch>("h", "help", "Print this help message");
  auto print_option = op.add<popl::Switch>("s", "skip", "Verbose print");

  auto lower_option =
      op.add<popl::Implicit<TYPE__>>("", "lower", "Lower bound", 0);
  auto upper_option =
      op.add<popl::Implicit<TYPE__>>("", "upper", "Upper bound", 32);

  auto num_option = op.add<popl::Implicit<unsigned>>(
      "", "num", "Length of the arrays to add together", 1048576);
  op.parse(argc, argv);

  if (help_option->is_set()) {
    std::cout << op << "\n ";
    return EXIT_SUCCESS;
  }

  const bool print = print_option->is_set();
  const auto lower = lower_option->value(), upper = upper_option->value();
  const auto num = num_option->value();

  if (lower >= upper) {
    std::cout << "Error: lower bound can't be greater than the upper bound\n";
    return EXIT_FAILURE;
  }

  app::vecadd adder;

  const auto print_array = [print](auto name, auto vec) {
    if (!print)
      return;
    std::cout << name << " := { ";
    for (const auto &v : vec) {
      std::cout << v << " ";
    }
    std::cout << "}\n";
  };

  auto fill_random_vector =
      clutils::create_random_number_generator(lower, upper);
  std::vector<TYPE__> a, b;
  a.resize(num);
  b.resize(num);
  fill_random_vector(a);
  fill_random_vector(b);

  print_array("A", a);
  print_array("B", b);

  std::chrono::microseconds pure_time;

  auto res = adder.add(a, b, &pure_time);
  print_array("C", res);
  bool correct = (a.size() == res.size());

  if (correct) {
    for (size_t i = 0; i < a.size(); ++i) {
      if (a[i] + b[i] == res[i])
        continue;
      std::cout << "Mismatch at position i = " << i << "\n";
      correct = false;
      break;
    }
  }

  std::cout << "GPU pure time: " << pure_time.count() << " us\n";

  if (correct) {
    std::cout << "GPU vector add works fine\n";
    return EXIT_SUCCESS;
  } else {
    std::cout << "GPU vector add is borked\n";
    return EXIT_FAILURE;
  }

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
