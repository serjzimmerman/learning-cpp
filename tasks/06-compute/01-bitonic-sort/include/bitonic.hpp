/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "opencl_include.hpp"
#include "selector.hpp"
#include "utils.hpp"

#include <bit>
#include <chrono>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>

#include "kernelhpp/bitonic_local_initial_kernel.hpp"
#include "kernelhpp/bitonic_naive_kernel.hpp"

namespace bitonic {

template <typename T> struct i_bitonic_sort {
  using size_type = unsigned;
  void sort(std::span<T> container, clutils::profiling_info *time = nullptr) {
    return operator()(container, time);
  }
  virtual void operator()(std::span<T>, clutils::profiling_info *) = 0;
  virtual ~i_bitonic_sort() {}
};

template <typename T> struct cpu_bitonic_sort : public i_bitonic_sort<T> {
  using typename i_bitonic_sort<T>::size_type;

  void operator()(std::span<T> container,
                  clutils::profiling_info *info) override {
    size_type size = container.size();
    if (std::popcount(size) != 1 || size < 2)
      throw std::runtime_error{"Only power-of-two sequences are supported"};

    const auto execute_step = [container, size](size_type stage,
                                                size_type step) {
      const size_type part_length = 1 << (step + 1);

      const auto calc_j = [stage, step, part_length](auto i) -> size_type {
        if (stage == step)
          return part_length - i - 1;
        return i + part_length / 2;
      };

      for (size_type k = 0; k < size / part_length; ++k) {
        for (size_type i = 0; i < part_length / 2; ++i) {
          const auto j = calc_j(i);
          auto &first = container[k * part_length + i];
          auto &second = container[k * part_length + j];
          if (first > second)
            std::swap(first, second);
        }
      }
    };

    const auto wall_start = std::chrono::high_resolution_clock::now();

    size_type stages = std::countr_zero(size);
    for (size_type stage = 0; stage < stages; ++stage) {
      for (size_type temp = 0, step = stage; temp <= stage;
           step = stage - (++temp)) {
        execute_step(stage, step);
      }
    }

    const auto wall_end = std::chrono::high_resolution_clock::now();

    if (info)
      info->wall = info->pure =
          std::chrono::duration_cast<std::chrono::milliseconds>(wall_end -
                                                                wall_start);
  }
};

template <typename T>
class gpu_bitonic : public i_bitonic_sort<T>,
                    protected clutils::platform_selector {
protected:
  cl::Context m_ctx;
  cl::CommandQueue m_queue;

  using typename i_bitonic_sort<T>::size_type;
  static constexpr clutils::platform_version cl_api_version = {2, 2};

public:
  gpu_bitonic(bool verbose)
      : clutils::platform_selector{cl_api_version, verbose}, m_ctx{m_device},
        m_queue{m_ctx, cl::QueueProperties::Profiling} {}

  template <long t_info> auto get_device_info() const {
    return m_device.getInfo<t_info>();
  }

private:
  void operator()(std::span<T>, clutils::profiling_info *) override {
  } // Dummy override so that the class is no longer abstract

protected:
  using func_signature = cl::Event(cl::Buffer);

  void run_boilerplate(std::span<T> container,
                       std::function<func_signature> func) {
    cl::Buffer buf = {m_ctx, CL_MEM_READ_WRITE,
                      clutils::sizeof_container(container)};
    cl::copy(m_queue, container.begin(), container.end(), buf);

    auto event = func(buf);
    event.wait();

    cl::copy(m_queue, buf, container.begin(), container.end());
  }
};

template <typename T, typename t_name>
class naive_bitonic : public gpu_bitonic<T> {
  using kernel = bitonic_naive_kernel;

private:
  cl::Program m_program;
  typename kernel::functor_type m_functor;

  using gpu_bitonic<T>::m_queue;
  using gpu_bitonic<T>::m_ctx;
  using gpu_bitonic<T>::run_boilerplate;

  using typename gpu_bitonic<T>::size_type;

public:
  naive_bitonic(gpu_bitonic<T> base)
      : gpu_bitonic<T>{base},
        m_program{m_ctx, kernel::source(t_name::name_str), true},
        m_functor{m_program, kernel::entry()} {}

  naive_bitonic(bool verbose) : naive_bitonic{gpu_bitonic<T>{verbose}} {}

  void operator()(std::span<T> container,
                  clutils::profiling_info *time = nullptr) override {
    const size_type size = container.size(), stages = std::countr_zero(size);
    if (std::popcount(size) != 1 || size < 2)
      throw std::runtime_error("Only power-of-two sequences are supported");
    cl::Event prev_event, first_event;

    auto submit = [&, first_iter = true](auto buf, auto stage,
                                         auto step) mutable {
      const auto global_size = size / 2;

      if (first_iter) {
        const auto args = cl::EnqueueArgs{m_queue, global_size};
        first_event = prev_event = m_functor(args, buf, stage, step);
        first_iter = false;
        return;
      }

      const auto args = cl::EnqueueArgs{m_queue, prev_event, global_size};
      prev_event = m_functor(args, buf, stage, step);
    };

    const auto func = [&, stages](auto buf) {
      for (unsigned stage = 0; stage < stages; ++stage) {
        for (int step = stage; step >= 0; --step) {
          submit(buf, stage, step);
        }
      }
      return prev_event;
    };

    const auto wall_start = std::chrono::high_resolution_clock::now();
    run_boilerplate(container, func);
    const auto wall_end = std::chrono::high_resolution_clock::now();

    const std::chrono::nanoseconds pure_start{
        first_event.getProfilingInfo<CL_PROFILING_COMMAND_START>()},
        pure_end{prev_event.getProfilingInfo<CL_PROFILING_COMMAND_END>()};

    if (time) {
      time->wall = std::chrono::duration_cast<std::chrono::milliseconds>(
          wall_end - wall_start);
      time->pure = std::chrono::duration_cast<std::chrono::milliseconds>(
          pure_end - pure_start);
    }
  }
};

template <typename T, typename t_name>
class local_bitonic : public gpu_bitonic<T> {
  using kernel_initial = bitonic_local_initial_kernel;
  using kernel_naive = bitonic_naive_kernel;

private:
  cl::Program m_program_initial, m_program_last;
  typename kernel_initial::functor_type m_functor_initial;
  typename kernel_naive::functor_type m_functor_last;

  using gpu_bitonic<T>::m_ctx;
  using gpu_bitonic<T>::m_queue;
  using gpu_bitonic<T>::run_boilerplate;

  using typename gpu_bitonic<T>::size_type;
  size_type m_local_size = 0;

public:
  local_bitonic(const size_type segment_size, gpu_bitonic<T> base)
      : gpu_bitonic<T>{base},
        m_program_initial{
            m_ctx, kernel_initial::source(t_name::name_str, segment_size),
            true},
        m_program_last{m_ctx, kernel_naive::source(t_name::name_str), true},
        m_functor_initial{m_program_initial, kernel_initial::entry()},
        m_functor_last{m_program_last, kernel_naive::entry()},
        m_local_size{segment_size} {
    if (std::popcount(segment_size) != 1 || segment_size < 2)
      throw std::runtime_error{"Segment size must be a natural power of 2"};
  }

  local_bitonic(const unsigned segment_size, bool verbose)
      : local_bitonic{segment_size, gpu_bitonic<T>{verbose}} {}

  void operator()(std::span<T> container,
                  clutils::profiling_info *time = nullptr) override {
    const size_type size = container.size(), stages = std::countr_zero(size),
                    initial_stages = std::countr_zero(m_local_size);
    if (std::popcount(size) != 1 || size < 2)
      throw std::runtime_error{"Only power-of-two sequences are supported"};
    if (size < m_local_size)
      throw std::runtime_error{"Total size can't be less than local size"};

    cl::Event prev_event, first_event;
    const auto initial_end_stage = std::min(initial_stages, stages);

    auto enqueue_initial = [&](auto buf) {
      auto args = cl::EnqueueArgs{m_queue, size / 2, m_local_size / 2};
      first_event = prev_event =
          m_functor_initial(args, buf, 0, initial_end_stage, 0);
    };

    auto enqueue_last = [&](auto buf) {
      for (unsigned stage = initial_end_stage; stage < stages; ++stage) {
        for (int step = stage; step >= 0; --step) {
          const size_type global_size = size / 2;
          const size_type part_length = 1 << (step + 1);

          if (part_length <= m_local_size) {
            const auto args =
                cl::EnqueueArgs{m_queue, size / 2, m_local_size / 2};
            prev_event =
                m_functor_initial(args, buf, stage, stage + 1, stage - step);
            break;
          }

          const auto args = cl::EnqueueArgs{m_queue, prev_event, global_size};
          prev_event = m_functor_last(args, buf, stage, step);
        }
      }
    };

    const auto func = [&](auto buf) {
      enqueue_initial(buf);
      enqueue_last(buf);
      return prev_event;
    };

    const auto wall_start = std::chrono::high_resolution_clock::now();
    run_boilerplate(container, func);
    const auto wall_end = std::chrono::high_resolution_clock::now();

    const std::chrono::nanoseconds pure_start{
        first_event.getProfilingInfo<CL_PROFILING_COMMAND_START>()},
        pure_end{prev_event.getProfilingInfo<CL_PROFILING_COMMAND_END>()};

    if (time) {
      time->wall = std::chrono::duration_cast<std::chrono::milliseconds>(
          wall_end - wall_start);
      time->pure = std::chrono::duration_cast<std::chrono::milliseconds>(
          pure_end - pure_start);
    }
  }
};

} // namespace bitonic
