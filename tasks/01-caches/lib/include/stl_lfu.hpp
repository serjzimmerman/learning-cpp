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

namespace caches {

namespace detail {

template <typename K, typename U> struct local_node_lfu_t {
  K m_key;
  U m_value;

  local_node_lfu_t(K p_key, U p_val) : m_key{p_key}, m_value{p_val} {}
};

template <typename K, typename U> class local_list_lfu_t {
  using node_t__ = local_node_lfu_t<K, U>;
  using self_t__ = local_list_lfu_t<K, U>;

public:
  using W = std::size_t;

  using const_it__ = typename std::list<node_t__>::const_iterator;
  using it__ = typename std::list<node_t__>::iterator;

private:
  std::list<node_t__> m_list;
  std::unordered_map<K, it__> m_map;

public:
  W m_weight;

  explicit local_list_lfu_t(W p_weight)
      : m_list{}, m_map{}, m_weight{p_weight} {}

  const W &weight() const noexcept { return m_weight; }

  auto size() const noexcept { return m_list.size(); }

  bool is_empty() const noexcept { return m_list.empty(); }

  void push_front(node_t__ p_node) {
    m_list.push_front(p_node);
    m_map.insert({p_node.m_key, m_list.begin()});
  }

  void splice_upfront(self_t__ &p_other, const_it__ p_elem) {
    p_other.m_map.erase(
        p_elem->m_key); // Erase "p_elem" from other list's lookup map.
    m_list.splice(m_list.begin(), p_other.m_list,
                  p_elem); // Move "p_elem" to the beginning of the "p_other".
    m_map.insert(
        {p_elem->m_key,
         m_list.begin()}); // Insert the element's key into current lookup map.
  }

  void splice_upfront(self_t__ &p_other, it__ p_elem, const K &p_new_key) {
    p_other.m_map.erase(
        p_elem->m_key); // Erase "p_elem" from other list's lookup map.
    m_list.splice(m_list.begin(), p_other.m_list,
                  p_elem); // Move "p_elem" to the beginning of the "p_other".
    m_map.insert(
        {p_new_key, m_list.begin()}); // Insert the element's key into current
                                      // lookup map with updated key.
    p_elem->m_key = p_new_key;
  }

  it__ lookup(const K &p_key) {
    auto found = m_map.find(p_key);
    // An element with key p_key will always be contained in a corresponding
    // local list.
    assert(found != m_map.end());
    return found->second;
  }

  it__ last() {
    // Local lists should never be empty if "last" is called.
    assert(!is_empty());
    return std::prev(m_list.end());
  }
};

}; // namespace detail

template <typename U, typename K = int> class lfu_t {
  std::size_t m_size, m_hits;

  using W = std::size_t;

  using local_node_t__ = typename detail::local_node_lfu_t<K, U>;
  using freq_list_node_t__ = detail::local_list_lfu_t<K, U>;
  using freq_node_it__ = typename std::list<freq_list_node_t__>::iterator;

  std::list<freq_list_node_t__> m_freq_list;
  std::unordered_map<K, freq_node_it__> m_weight_map;

  bool is_last(freq_node_it__ p_iter) const {
    return (std::next(p_iter) == m_freq_list.end());
  }

  // Returns node with weight "1" or create a new node with corresponding weight
  // if it doesn't already exist.
  freq_node_it__ first_weight_node() {
    auto front = m_freq_list.begin();

    // In case the list isn't empty and front node has weight "1".
    if (!m_freq_list.empty() && front->m_weight == 1) {
      return front;
    }

    m_freq_list.emplace_front(1);
    return m_freq_list.begin();
  }

  // Returns node with the least weight. Because the list is created in an
  // ascending sorted order it is the head of the list.
  freq_node_it__ least_weight_node() {
    // As this function will only be called on a non-empty list, it's safe to
    // call "begin()" that will never return "end".
    assert(!m_freq_list.empty());
    return m_freq_list.begin();
  }

  // This function creates a "frequency node" after "p_prev" with
  // "p_prev->m_weight + 1" key in case it does not exists. Otherwise and
  // returns an incremented iterator.
  freq_node_it__ next_weight_node(freq_node_it__ p_prev) {
    W next_weight = p_prev->m_weight + 1;

    if (is_last(p_prev)) {
      m_freq_list.emplace_back(next_weight);
      return std::next(p_prev);
    }

    freq_node_it__ next_it = std::next(p_prev);

    if (next_it->m_weight == next_weight) {
      return next_it;
    } else {
      return m_freq_list.insert(next_it, freq_list_node_t__{next_weight});
    }
  }

  // Removes the node of a list if the "local list" is empty.
  void remove_if_empty(freq_node_it__ p_it) {
    if (p_it->is_empty()) {
      m_freq_list.erase(p_it);
    }
  }

  // Helper function for Case 1 of "lookup". It promotes the the element
  // "p_key" according to LFU policy.
  U promote(const K &p_key) {
    auto found_weight = m_weight_map.find(
        p_key); // Lookup which local list "p_key" is present in.
    assert(found_weight != m_weight_map.end());

    // Corresponding iterator of "frequency list" and an node with incremented
    // weight.
    freq_node_it__ freq_it = found_weight->second;

#if 1
    // Handle the case when *freq_it contains only a single element. In this
    // case promotion would mean incrementing the weight of node. This way
    // possible allocation and deallocation is bypassed.
    if (freq_it->size() == 1 &&
        (std::next(freq_it)->m_weight != (freq_it->m_weight + 1))) {
      freq_it->m_weight++;
      return freq_it->last()->m_value;
    }
#endif

    freq_node_it__ next_it = next_weight_node(freq_it);
    found_weight->second = next_it; // Update weight map.

    auto elem_it = freq_it->lookup(p_key);
    next_it->splice_upfront(*freq_it,
                            elem_it); // Move "p_key" to the next local list.
    remove_if_empty(freq_it);

    return elem_it->m_value;
  }

  void insert(const K &p_key, U p_val) {
    auto first = first_weight_node();
    first->push_front(local_node_t__{p_key, p_val});
    m_weight_map.insert(std::make_pair(p_key, first));
  }

  void evict_and_replace(const K &p_key, U p_val) {
    // First we choose the "to_evict" entry by looking up the tail of a local
    // list with the least weight.
    auto least = least_weight_node();
    auto to_evict = least->last();

    m_weight_map.erase(to_evict->m_key); // Erase the entry from key-weight map.
    to_evict->m_value = p_val;           // Reuse the local list node.

    // Move the now evicted node to the bucket with weight "1".
    first_weight_node()->splice_upfront(*least, to_evict, p_key);
    remove_if_empty(least); // Clean up the frequency list, if "to_evict" was
                            // the only element in the local list.
    m_weight_map.insert(
        {p_key,
         least_weight_node()}); // Insert the new entry into the key-weight map.
  }

  bool is_present(const K &p_key) const {
    return (m_weight_map.find(p_key) != m_weight_map.end());
  }

public:
  explicit lfu_t(std::size_t p_size)
      : m_size{p_size}, m_hits{0}, m_freq_list{}, m_weight_map{} {
    if (!p_size) {
      throw std::invalid_argument("lfu_t()");
    }
  }

  bool is_full() const noexcept { return (m_weight_map.size() == m_size); }

  std::size_t get_hits() const noexcept { return m_hits; }

  template <typename F> U lookup(const K &p_key, F p_slow_get) {
    // Case 1. The entry is present in the cache. Then it gets promoted.
    if (is_present(p_key)) {
      m_hits++;
      return promote(p_key);
    }

    U val = p_slow_get(p_key);

    // Case 2. If the entry is not present at the moment, and cache is not full.
    // Then it gets inserted into a frequency bucket with weight "1" and pushed
    // into the beginning of the corresponding frequency list;
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
