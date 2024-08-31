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

#include <algorithm>
#include <cassert>
#include <deque>
#include <functional>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace caches {

namespace detail {

template <typename T> class occurence_map_t {
  using map_t__ = typename std::unordered_map<T, std::deque<std::size_t>>;
  map_t__ m_map;

public:
  template <typename t_iterator>
  occurence_map_t(t_iterator p_begin, t_iterator p_end) : m_map{} {
    // This constructs a map of vectors of corresponding occurence vectors.
    std::for_each(p_begin, p_end, [this, i = 0](const T &element) mutable {
      m_map[element].push_back(++i);
    });
  }
#if 0
  const T &find_latest_used(const std::unordered_set<T> &p_curr_set) {
    assert(p_curr_set.size());
    // The first version is a bit more elegant but works much worse than the second.
#if 0
    class set_comp__ {
      const map_t__ &m_map;

    public:
      set_comp__(const map_t__ &p_map) : m_map(p_map) {
      }

      bool operator()(const T &lhs, const T &rhs) {
        auto found_lhs = m_map.find(lhs);
        auto found_rhs = m_map.find(rhs);

        if (found_rhs == m_map.end()) {
          return true;
        }

        if (found_lhs == m_map.end()) {
          return false;
        }

        return found_lhs->second.front() < found_rhs->second.front();
      }
    };

    return *std::max_element(p_curr_set.begin(), p_curr_set.end(), set_comp__{m_map});

#else
    auto latest_used = p_curr_set.begin();
    auto found = m_map.find(*latest_used);

    if (found == m_map.end()) {
      return *latest_used;
    }

    auto latest_indx = found->second.front();
    for (auto its = p_curr_set.begin(), ite = p_curr_set.end(); its != ite; ++its) {
      auto found = m_map.find(*its);

      if (found == m_map.end()) {
        return *its;
      }

      if (found->second.front() > latest_indx) {
        latest_indx = found->second.front();
        latest_used = its;
      }
    }

    return *latest_used;
#endif
  }
#endif

  // Returns the first occurence of the element. "0" is used to indicate that
  // the value will never be encountered again.
  std::size_t first(const T &p_elem) {
    auto found = m_map.find(p_elem);
    return found->second.front();
  }

  void erase_first(const T &p_elem) {
    assert(m_map.find(p_elem) != m_map.end());
    auto &deq = m_map.find(p_elem)->second;
    deq.pop_front();
    if (deq.empty()) {
      // One more hacky way to avoid branching. Instead of checking for "emtpty"
      // in "first" method when the deque is empty we insert "0" in it. This
      // shaves off a few ms in the long run.
      deq.push_front(0);
    }
  }
};

template <typename T> class ideal_t {
  std::map<std::size_t, T> m_contained;
#if 0
  std::unordered_set<T> m_set;
#endif

  std::vector<T> m_vec;
  occurence_map_t<T> m_occur_map;
  std::size_t m_size, m_hits;

  bool is_full() const noexcept {
    // Here we subtract one from size because an element with "0" index is
    // basically garbage and holds all indexes that otherwise would never be
    // encountered.
    return ((m_contained.size() - 1) == m_size);
    // For the naive implementation this would be:
#if 0
    return (m_set.size() == m_size);
#endif
  }

  void insert(const T &p_elem) {
    std::size_t used_next = m_occur_map.first(p_elem);
    // See constructor. In the m_contained map there's always an element with 0
    // value. Its value is garbage but it turns out that removing a branch is
    // faster. For reference here's what was here before.
#if 0
    if (used_next) {
      m_contained[used_next] = p_elem;
    }
#else
    m_contained[used_next] = p_elem;
#endif
  }

  void lookup_elem(const T &p_elem) {
    // This is the naive implementation that I tried first. It works reasonably
    // well with small cache sizes, but has a assymptotic linear complexity with
    // size of cache m. But this complexity is actually ammortized because a
    // linear complexity algorithm is called only when there's a cache miss.
#if 0
    if (m_set.find(p_elem) != m_set.end()) {
      m_hits++;
    }

    else if (!is_full()) {
      m_set.insert(p_elem);
    }

    else {
      m_set.erase(m_occur_map.find_latest_used(m_set));
      m_set.insert(p_elem);
    }
    m_occur_map.erase_first(p_elem);
#else
    std::size_t prev_indx = m_occur_map.first(p_elem);
    m_occur_map.erase_first(p_elem);

    if (m_contained.find(prev_indx) != m_contained.end()) {
      m_hits++;

      m_contained.erase(prev_indx);
      insert(p_elem);

      return;
    }

    else if (!is_full()) {
      insert(p_elem);
    }

    else {
      // The std::prev is completely legal bacause there's must at least one
      // element present.
      auto [latest_indx, latest_used] = *(std::prev(m_contained.end()));
      m_contained.erase(latest_indx);
      insert(p_elem);
    }
#endif
  }

public:
  template <typename t_iterator>
  ideal_t(std::size_t p_size, t_iterator p_begin, t_iterator p_end)
      : m_contained{}, m_vec{}, m_occur_map{p_begin, p_end}, m_size{p_size},
        m_hits{0} {
    std::copy(p_begin, p_end, std::back_inserter(m_vec));
    // This is a very hacky but a reasonably good way to avoid branching in
    // "insert" method.
    m_contained[0] = 0;
  }

  std::size_t count_hits() {
    for (const auto &v : m_vec) {
      lookup_elem(v);
    }

    return m_hits;
  }
};

} // namespace detail

// Implementation of Belady's algorithm. Returns the number of maximum possible
// hits for a cache of size "size" and "vec" of requests.
template <typename T, typename t_iterator>
std::size_t get_optimal_hits(std::size_t p_size, t_iterator p_begin,
                             t_iterator p_end) {
  using namespace detail;

  if (!p_size || (p_begin == p_end)) {
    throw std::invalid_argument{"get_optiomal_hits()"};
  }

  ideal_t<T> cache{p_size, p_begin, p_end};
  return cache.count_hits();
}

} // namespace caches
