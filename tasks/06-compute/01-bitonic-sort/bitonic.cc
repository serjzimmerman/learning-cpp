/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#define STRINGIFY0(v) #v
#define STRINGIFY(v) STRINGIFY0(v)

#ifndef TYPE__
#define TYPE__ int
#endif

#include "bitonic.hpp"

#include <algorithm>
#include <bit>
#include <climits>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "popl.hpp"

#ifdef PAR_CPU_SORT

#include <parallel/algorithm>
#define CPU_SORT ::__gnu_parallel::sort
#define CPU_SORT_NAME "__gnu_parallel::sort"

#else

#define CPU_SORT std::sort
#define CPU_SORT_NAME "std::sort"

#endif

using vector_type = std::vector<TYPE__>;

void vprint(const std::string title, const auto &vec) {
  std::cout << title << ": { ";
  for (auto &elem : vec) {
    std::cout << elem << " ";
  }
  std::cout << "}\n";
}

int validate_results(const auto &origin, const auto &res, const auto &check,
                     bool print_on_failure) {
  if (std::equal(res.begin(), res.end(), check.begin())) {
    std::cout << "Bitonic sort works fine\n";
    return EXIT_SUCCESS;
  }

  std::cout << "Bitonic sort is broken\n";

  if (print_on_failure) {
    vprint("Original", origin);
    vprint("Result", res);
    vprint("Correct", check);
  }

  return EXIT_FAILURE;
}

template <typename T> struct type_name {};
template <> struct type_name<TYPE__> {
  static constexpr const char *name_str = STRINGIFY(TYPE__);
};

template <typename T> void optimal_bitonic_sort(std::vector<T> &vec) {
  const unsigned n = vec.size();

  if (n == 0 || n == 1) {
    return; /* Nothing to do */
  }

  // Closest power of two size
  unsigned closest_size;
  if (std::popcount(n) == 1) {
    closest_size = n;
  } else {
    closest_size = 1 << (sizeof(decltype(n)) * CHAR_BIT - std::countl_zero(n));
  }

  bitonic::gpu_bitonic<T> sorter_base{false};

  const auto get_possible_local_size = [&sorter_base]() {
    constexpr float mem_occupied = 0.95f;

    const unsigned max_wg_size =
        sorter_base.template get_device_info<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
    const auto max_device_local_mem =
        sorter_base.template get_device_info<CL_DEVICE_LOCAL_MEM_SIZE>();
    const unsigned max_elems_in_local_mem =
        mem_occupied * max_device_local_mem / sizeof(T);

    // Multiply by 2 because local kernel uses half the threads
    return std::min(2 * max_wg_size, max_elems_in_local_mem);
  };

  unsigned optimal_lsz;
  const unsigned possible_lsz = get_possible_local_size();

  if (std::popcount(possible_lsz) == 1) {
    optimal_lsz = possible_lsz;
  } else {
    optimal_lsz = 1 << (sizeof(decltype(possible_lsz)) * CHAR_BIT -
                        std::countl_zero(possible_lsz) - 1);
  }

  std::unique_ptr<bitonic::i_bitonic_sort<T>> sorter;
  if (optimal_lsz > closest_size) {
    sorter =
        std::make_unique<bitonic::naive_bitonic<TYPE__, type_name<TYPE__>>>(
            sorter_base);
  } else {
    sorter =
        std::make_unique<bitonic::local_bitonic<TYPE__, type_name<TYPE__>>>(
            optimal_lsz, sorter_base);
  }

  const auto dummy_element = vec.front();
  long count_dummy = closest_size - n;
  vec.resize(closest_size, dummy_element);
  sorter->sort(vec);

  auto pred = [&count_dummy, dummy_element](const auto &elem) {
    if (elem == dummy_element && count_dummy > 0) {
      count_dummy--;
      return true;
    }

    return false;
  };

  vec.erase(std::remove_if(vec.begin(), vec.end(), pred), vec.end());
}

int main(int argc, char **argv) try {
  const auto maximum = std::numeric_limits<TYPE__>::max(),
             minimum = std::numeric_limits<TYPE__>::min();

  popl::OptionParser op("Avaliable options");
  auto help_option =
      op.add<popl::Switch>("h", "help", "Print this help message");
  auto print_option = op.add<popl::Switch>("p", "print", "Print on failure");
  auto skip_option =
      op.add<popl::Switch>("s", "skip", "Skip comparing with std::sort");
  auto random_option =
      op.add<popl::Switch>("r", "random", "Generate random vectors");

  auto lower_option =
      op.add<popl::Implicit<TYPE__>>("", "lower", "Lower bound", minimum);
  auto upper_option =
      op.add<popl::Implicit<TYPE__>>("", "upper", "Upper bound", maximum);

  auto num_option = op.add<popl::Implicit<unsigned>>(
      "", "num", "Length of the array to sort = 2^n", 24);
  auto kernel_option = op.add<popl::Implicit<std::string>>(
      "", "kernel", "Which kernel to use: naive, cpu, local", "naive");
  auto lsz_option =
      op.add<popl::Implicit<unsigned>>("", "lsz", "Local memory size", 256);

  op.parse(argc, argv);

  if (help_option->is_set()) {
    std::cout << op << "\n ";
    return EXIT_SUCCESS;
  }

  if (!random_option->is_set()) {
    std::cin.exceptions(std::cin.exceptions() | std::ios::badbit |
                        std::ios::failbit);
    vector_type input;

    unsigned n;
    std::cin >> n;

    if (n == 0) {
      return EXIT_SUCCESS;
    }

    vector_type original;
    original.reserve(n);
    std::copy_n(std::istream_iterator<TYPE__>{std::cin}, n,
                std::back_inserter(original));
    optimal_bitonic_sort(original);

    for (const auto e : original) {
      std::cout << e << " ";
    }

    std::cout << "\n";
    return EXIT_SUCCESS;
  }

  const bool skip_std_sort = skip_option->is_set(),
             print_on_failure = print_option->is_set();
  const auto lower = lower_option->value(), upper = upper_option->value();
  const auto num = num_option->value();
  const auto kernel_name = kernel_option->value();
  const auto lsz = lsz_option->value();
  const bool verbose = random_option->is_set();

  std::unique_ptr<bitonic::i_bitonic_sort<TYPE__>> sorter;

  if (kernel_name == "naive") {
    sorter =
        std::make_unique<bitonic::naive_bitonic<TYPE__, type_name<TYPE__>>>(
            verbose);
  } else if (kernel_name == "cpu") {
    sorter = std::make_unique<bitonic::cpu_bitonic_sort<TYPE__>>();
  } else if (kernel_name == "local") {
    sorter =
        std::make_unique<bitonic::local_bitonic<TYPE__, type_name<TYPE__>>>(
            lsz, verbose);
  } else {
    std::cout << "Unknown type of kernel: " << kernel_name << "\n ";
    return EXIT_FAILURE;
  }

  if (kernel_name != "local" && lsz_option->is_set()) {
    std::cout << "Warning: local size provided but kernel used is not "
                 "\"local\", ignoring --lsz option\n";
  }

  if (lower >= upper) {
    std::cout << "Error: lower bound can't be greater than the upper bound\n";
    return EXIT_FAILURE;
  }

  const unsigned size = (1 << num);
  const auto print_sep = []() { std::cout << " -------- \n"; };

  std::cout << "Sorting vector of size = " << size << "\n";
  print_sep();

  vector_type origin;
  origin.resize(size);

  auto rand_gen = clutils::create_random_number_generator<TYPE__>(lower, upper);
  rand_gen(origin);

  std::chrono::milliseconds wall;
  auto check = origin;

  if (!skip_std_sort) {
    auto wall_start = std::chrono::high_resolution_clock::now();
    CPU_SORT(check.begin(), check.end());
    auto wall_end = std::chrono::high_resolution_clock::now();
    wall = std::chrono::duration_cast<std::chrono::milliseconds>(wall_end -
                                                                 wall_start);
  }

  clutils::profiling_info prof_info;
  auto vec = origin;

  sorter->sort(vec, &prof_info);

  if (!skip_std_sort)
    std::cout << CPU_SORT_NAME << " wall time: " << wall.count() << " ms\n";

  std::cout << "bitonic wall time: " << prof_info.wall.count() << " ms\n";
  std::cout << "bitonic pure time: " << prof_info.pure.count() << " ms\n";

  print_sep();

  if (skip_std_sort)
    return EXIT_SUCCESS;
  return validate_results(origin, vec, check, print_on_failure);

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
