/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include <cassert>
#include <deque>
#include <functional>
#include <iterator>
#include <list>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "stl_lfu.hpp"

namespace caches {

namespace detail {

template <typename K, typename U> struct local_node_lfuda_t {
  using W = std::size_t;

  K m_key;
  U m_value;
  W m_freq, m_weight;

  local_node_lfuda_t(K p_key, U p_val, W p_weight, W p_freq = 1)
      : m_key{p_key}, m_value{p_val}, m_freq{p_freq}, m_weight{p_weight} {}
};

template <typename K, typename U> class local_list_lfuda_t {
  using W = std::size_t;

  using node_t__ = local_node_lfuda_t<K, U>;
  using self_t__ = local_list_lfuda_t<K, U>;
  std::list<node_t__> m_list;

public:
  W m_weight;

  using it__ = typename std::list<node_t__>::iterator;

  local_list_lfuda_t(W p_weight) : m_list{}, m_weight{p_weight} {}

  bool is_empty() const { return m_list.empty(); }

  auto size() const noexcept { return m_list.size(); }

  it__ front() { return m_list.front(); }

  it__ last() {
    assert(!is_empty());
    return std::prev(m_list.end());
  }

  it__ push_front(node_t__ p_node) {
    p_node.m_weight = m_weight; // Guarantee that the weight of node corresponds
                                // to the weight of the list.
    m_list.push_front(p_node);
    return m_list.begin();
  }

  it__ push_front(K p_key, U p_val, W p_freq = 1) {
    m_list.push_front({p_key, p_val, m_weight, p_freq});
    return m_list.begin();
  }

  void splice_upfront(self_t__ &p_other, it__ p_elem) {
    m_list.splice(m_list.begin(), p_other.m_list, p_elem);
    p_elem->m_weight = m_weight;
  }
};

}; // namespace detail

template <typename U, typename K = int> class lfuda_t {
  using W = std::size_t;

  std::size_t m_size, m_hits;
  W m_age;

  using local_node_t__ = detail::local_node_lfuda_t<K, U>;
  using local_list_t__ = detail::local_list_lfuda_t<K, U>;
  using local_list_it__ = typename local_list_t__::it__;

  std::map<W, local_list_t__> m_weight_map;
  std::unordered_map<K, local_list_it__> m_key_it_map;

  bool is_present(K p_key) const {
    return (m_key_it_map.find(p_key) != m_key_it_map.end());
  }

  local_list_t__ &freq_node_with_weight(W p_weight) {
    auto inserted = m_weight_map.emplace(p_weight, p_weight);
    return inserted.first->second;
  }

  void remove_if_empty(const W p_weight) {
    auto found = m_weight_map.find(p_weight);
    assert(found != m_weight_map.end());
    auto local_list = found->second;
    if (local_list.is_empty()) {
      m_weight_map.erase(p_weight);
    }
  }

  U promote(const K &p_key) {
    // Lookup the corresponding to the key local list node.
    auto found = m_key_it_map.find(p_key);
    assert(found != m_key_it_map.end());
    auto node_to_promote = found->second;

    W old_weight =
        node_to_promote->m_weight; // This is the old weight of the node.
    auto &old_local_list = m_weight_map.find(old_weight)->second;
    W new_weight = ++node_to_promote->m_freq +
                   m_age; // Increment the frequency and calculate next weight.

    // Here we handle the case when the current local list contains the single
    // promoted element.
    auto found_next = m_weight_map.find(new_weight);
#if 0
    if ((old_local_list.size() == 1) && (found_next == m_weight_map.end())) {
      auto node = m_weight_map.extract(old_weight);
      node.key() = node.mapped().m_weight = node_to_promote->m_weight = new_weight;
      m_weight_map.insert(std::move(node));
    } 
    
    else {
#endif
    auto &new_freq_node =
        ((found_next == m_weight_map.end()) ? freq_node_with_weight(new_weight)
                                            : found_next->second);
    new_freq_node.splice_upfront(
        old_local_list, node_to_promote); // Move the node to the next list.
    remove_if_empty(old_weight);          // Clean up after ourselves.
#if 0
    }
#endif

    return node_to_promote->m_value;
  }

  void insert(const K &p_key, U p_val) {
    W new_weight = m_age + 1;
    auto to_insert = local_node_t__{p_key, p_val, new_weight};
    auto inserted = freq_node_with_weight(new_weight).push_front(to_insert);
    m_key_it_map.insert({p_key, inserted});
  }

  void evict_and_replace(const K &p_key, U p_val) {
    assert(m_key_it_map.size()); // Cache should contain some elements or I've
                                 // made an oopsie somewhere.
    auto &[least_weight, least_local_list] = *m_weight_map.begin();

    m_age = least_weight; // Upadte weight to the evicted entry's weight.
    auto to_evict = least_local_list.last();
    m_key_it_map.erase(to_evict->m_key);

    to_evict->m_key = p_key;
    to_evict->m_value = p_val; // Reuse the entry for
    to_evict->m_freq = 1;      // the newly inserted element.

    W new_weight = m_age + 1;
    m_weight_map.insert({new_weight, new_weight});

    // Here i tried minimizing the memory allocations and deallocations. Turns
    // out it actually makes the perfomance worse in test cases.
    auto found_next = m_weight_map.find(new_weight);
#if 0
    if ((least_local_list.size() == 1) && (found_next == m_weight_map.end())) {
      auto node = m_weight_map.extract(least_weight);
      node.key() = node.mapped().m_weight = to_evict->m_weight = new_weight;
      m_weight_map.insert(std::move(node));
    }

    else {
#endif
    auto &new_freq_node =
        ((found_next == m_weight_map.end()) ? freq_node_with_weight(new_weight)
                                            : found_next->second);
    new_freq_node.splice_upfront(least_local_list, to_evict);
    m_key_it_map.insert({p_key, to_evict});
    remove_if_empty(least_weight); // As always, cleanup after yourself.
#if 0
    }
#endif
  }

public:
  explicit lfuda_t(std::size_t p_size)
      : m_size{p_size}, m_hits{0}, m_age{0}, m_weight_map{}, m_key_it_map{} {
    if (!p_size) {
      throw std::invalid_argument("lfuda_t()");
    }
  }

  bool is_full() const { return (m_key_it_map.size() == m_size); }

  std::size_t get_hits() const { return m_hits; }

  template <typename F> U lookup(K p_key, F p_slow_get) {
    // Case 1. The entry is present in the cache. Then it gets promoted.
    if (is_present(p_key)) {
      m_hits++;
      return promote(p_key);
    }

    U val = p_slow_get(p_key);

    // Case 2. If the entry is not present at the moment, and cache is not full.
    // Then it gets inserted into a weight bucket with weight "m_age + 1" and
    // pushed into the beginning of the corresponding weight list;
    if (!is_full()) {
      insert(p_key, val);
    }

    // Case 3. If we get here, then the cache is full and we need to choose an
    // entry to evict, that the new entry will replace.
    else {
      evict_and_replace(p_key, val);
    }

    return val;
  }
};

}; // namespace caches
