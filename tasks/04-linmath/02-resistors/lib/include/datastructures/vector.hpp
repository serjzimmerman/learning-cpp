/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include <algorithm>
#include <bit>
#include <climits>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>

#include "utility.hpp"

namespace throttle::containers {

template <typename T>
  requires std::is_nothrow_move_constructible_v<T> &&
           std::is_nothrow_destructible_v<T>
class vector {
private:
  T *m_buffer_ptr = nullptr;
  T *m_past_capacity_ptr = nullptr;
  T *m_past_end_ptr = nullptr;

  static constexpr std::size_t default_capacity = 8;

public:
  using size_type = std::size_t;
  using value_type = T;

  using iterator = utility::contiguous_iterator<value_type>;
  using const_iterator = utility::const_contiguous_iterator<value_type>;

private:
  using pointer = value_type *;
  using const_pointer = const value_type *;

  using reference = value_type &;
  using const_reference = const value_type &;

  void delete_elements() noexcept {
    if constexpr (!std::is_trivially_destructible_v<value_type>) {
      std::destroy(m_buffer_ptr, m_past_end_ptr);
    }
    m_past_end_ptr = m_buffer_ptr;
  }

public:
  static size_type amortized_buffer_size(size_type x) {
    return size_type{1} << (CHAR_BIT * sizeof(size_type) - std::countl_zero(x));
  }

public:
  vector()
      : m_buffer_ptr{static_cast<pointer>(
            ::operator new(sizeof(value_type) * default_capacity))},
        m_past_capacity_ptr{m_buffer_ptr + default_capacity},
        m_past_end_ptr{m_buffer_ptr} {}

  vector(size_type count, const value_type &value = value_type{})
    requires std::copyable<value_type>
  {
    vector temp{};
    temp.reserve(count);

    for (size_type i = 0; i < count; ++i) {
      temp.push_back(value);
    }

    *this = std::move(temp);
  }

  template <std::input_iterator it> vector(it start, it finish) {
    vector temp{};
    std::copy(start, finish, std::back_inserter(temp));
    std::swap(*this, temp);
  }

  template <std::random_access_iterator it> vector(it start, it finish) {
    vector temp{};
    temp.reserve(std::distance(start, finish));
    std::copy(start, finish, std::back_inserter(temp));
    *this = std::move(temp);
  }

  ~vector() {
    auto deleter = [](pointer ptr) { ::operator delete(ptr); };

    std::unique_ptr<value_type, decltype(deleter)> uptr{m_buffer_ptr, deleter};
    delete_elements();
  }

  vector(vector &&rhs) noexcept {
    std::swap(m_buffer_ptr, rhs.m_buffer_ptr);
    std::swap(m_past_capacity_ptr, rhs.m_past_capacity_ptr);
    std::swap(m_past_end_ptr, rhs.m_past_end_ptr);
  }

  vector(const vector &other)
    requires std::copyable<value_type>
  {
    vector temp{};
    temp.reserve(other.capacity());

    const size_type sz = other.size();
    if constexpr (std::is_trivially_copyable<value_type>::value) {
      std::memcpy(temp.m_buffer_ptr, other.m_buffer_ptr,
                  sz * sizeof(value_type));
    } else {
      std::uninitialized_copy(other.m_buffer_ptr, other.m_past_end_ptr,
                              temp.m_buffer_ptr);
    }
    temp.m_past_end_ptr += sz;

    *this = std::move(temp);
  }

  vector &operator=(const vector &rhs)
    requires std::copyable<value_type>
  {
    if (this == std::addressof(rhs))
      return *this;
    vector temp{rhs};
    *this = std::move(temp);
    return *this;
  }

  vector &operator=(vector &&rhs) noexcept {
    if (this == std::addressof(rhs))
      return *this;
    std::swap(m_buffer_ptr, rhs.m_buffer_ptr);
    std::swap(m_past_capacity_ptr, rhs.m_past_capacity_ptr);
    std::swap(m_past_end_ptr, rhs.m_past_end_ptr);
    return *this;
  }

  void reserve_exact(size_type cap) {
    if (cap <= capacity())
      return;

    pointer temp_buf =
        static_cast<pointer>(::operator new(sizeof(value_type) * cap));
    auto deleter = [](pointer ptr) { ::operator delete(ptr); };
    std::unique_ptr<value_type, decltype(deleter)> uptr{temp_buf, deleter};

    const size_type sz = size();
    if constexpr (std::is_trivially_copyable<value_type>::value) {
      std::memcpy(uptr.get(), m_buffer_ptr, sz * sizeof(value_type));
    } else {
      std::uninitialized_move(m_buffer_ptr, m_past_end_ptr, uptr.get());
    }

    ::operator delete(m_buffer_ptr);
    m_buffer_ptr = uptr.release();
    m_past_end_ptr = m_buffer_ptr + sz;
    m_past_capacity_ptr = m_buffer_ptr + cap;
  }

  void reserve(size_type cap) { reserve_exact(amortized_buffer_size(cap)); }

  void resize(size_type count, const value_type &val = value_type{})
    requires std::copyable<value_type>
  {
    const size_type sz = size();
    if (count == sz)
      return;

    if (count < sz) {
      for (size_type i = 0; i < sz - count; ++i) {
        pop_back();
      }
      return;
    }

    else {
      reserve(count);
      size_type i;
      try {
        for (i = 0; i < count - sz; ++i) {
          push_back(val);
        }
      } catch (...) {
        for (size_type j = 0; j < i; ++j) {
          pop_back();
        }
        throw;
      }
    }
  }

private:
  void reserve_if_necessary() {
    if (m_past_capacity_ptr - m_past_end_ptr > 0)
      return;
    reserve(capacity());
  }

public:
  void push_back(const value_type &val)
    requires std::copyable<value_type>
  {
    reserve_if_necessary();
    value_type tmp{val};
    new (m_past_end_ptr++) value_type{std::move(tmp)};
  }

  void push_back(value_type &&val)
    requires std::movable<value_type>
  {
    reserve_if_necessary();
    new (m_past_end_ptr++) value_type{std::move(val)};
  }

  template <typename... Ts> void emplace_back(Ts &&...args) {
    reserve_if_necessary();
    new (m_past_end_ptr) value_type{(std::forward<Ts>(args))...};
    m_past_end_ptr++;
  }

  void clear() { delete_elements(); }
  void pop_back() { std::destroy_at(--m_past_end_ptr); }

  size_type size() const noexcept { return m_past_end_ptr - m_buffer_ptr; }
  size_type capacity() const noexcept {
    return m_past_capacity_ptr - m_buffer_ptr;
  }

  bool empty() const noexcept { return (size() == 0); }

  reference back() { return *(m_past_end_ptr - 1); }
  const_reference back() const { return *(m_past_end_ptr - 1); }

  reference front() { return *m_buffer_ptr; }
  const_reference front() const { return *m_buffer_ptr; }

  value_type *data() { return m_buffer_ptr; }
  const value_type *data() const { return m_buffer_ptr; }

  reference operator[](size_type index) { return *(m_buffer_ptr + index); }
  const_reference operator[](size_type index) const {
    return *(m_buffer_ptr + index);
  }

  reference at(size_type index) {
    if (index >= size())
      throw std::out_of_range("index out of range.");
    return (*this)[index];
  }

  const_reference at(size_type index) const {
    if (index >= size())
      throw std::out_of_range("index out of range.");
    return (*this)[index];
  }

  iterator begin() { return iterator{m_buffer_ptr}; }
  iterator end() { return iterator{m_past_end_ptr}; }
  const_iterator begin() const { return cbegin(); }
  const_iterator end() const { return cend(); }

  const_iterator cbegin() const { return const_iterator{m_buffer_ptr}; }
  const_iterator cend() const { return const_iterator{m_past_end_ptr}; }
};

} // namespace throttle::containers
