/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include <functional>
#include <initializer_list>
#include <iostream>

#include "detail/splay_order_tree.hpp"

namespace throttle {
template <typename T, typename t_comp = std::less<T>> class splay_order_set {
private:
  detail::splay_order_tree<T, t_comp, T> m_tree_impl;

public:
  class iterator {
    friend class splay_order_set<T, t_comp>;

  private:
    typename detail::splay_order_tree<T, t_comp, T>::iterator m_it_impl;

    iterator(
        typename detail::splay_order_tree<T, t_comp, T>::iterator p_wrapped)
        : m_it_impl{p_wrapped} {}

  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type =
        typename detail::splay_order_tree<T, t_comp,
                                          T>::iterator::difference_type;
    using value_type = T;
    using pointer = const T *;
    using reference = const T &;

    iterator() : m_it_impl{} {}

    reference operator*() const { return *m_it_impl; }

    pointer operator->() { return m_it_impl.get(); }

    iterator &operator++() {
      ++m_it_impl;
      return *this;
    }

    iterator operator++(int) {
      iterator temp{*this};
      ++(*this);
      return temp;
    }

    iterator &operator--() {
      --m_it_impl;
      return *this;
    }

    iterator operator--(int) {
      iterator temp{*this};
      --(*this);
      return temp;
    }

    bool operator==(const iterator &p_rhs) const {
      return m_it_impl == p_rhs.m_it_impl;
    }

    bool operator!=(const iterator &p_rhs) const {
      return m_it_impl != p_rhs.m_it_impl;
    }
  };

public:
  using value_type = T;
  using reference = const T &;
  using size_type = typename detail::splay_order_tree<T, t_comp, T>::size_type;
  using key_type = T;
  using difference_type = typename iterator::difference_type;
  using const_iterator = iterator;

public:
  bool empty() const { return m_tree_impl.empty(); }

  size_type size() const { return m_tree_impl.size(); }

  bool contains(const key_type &p_key) const { return (find(p_key) != end()); }

  void clear() { m_tree_impl.clear(); }

public: // Selectors
  iterator begin() const { return iterator{m_tree_impl.begin()}; }

  iterator end() const { return iterator{m_tree_impl.end()}; }

  const_iterator cbegin() const { return iterator{m_tree_impl.begin()}; }

  const_iterator cend() const { return iterator{m_tree_impl.end()}; }

  iterator min() const { return (empty() ? end() : begin()); }

  iterator max() const { return (empty() ? end() : std::prev(end())); }

  iterator find(const key_type &p_key) const {
    return iterator{m_tree_impl.find(p_key)};
  }

  iterator lower_bound(const key_type &p_key) const {
    return iterator{m_tree_impl.lower_bound(p_key)};
  }

  iterator upper_bound(const key_type &p_key) const {
    return iterator{m_tree_impl.upper_bound(p_key)};
  }

  iterator select_rank(size_type p_rank) const {
    return iterator{m_tree_impl.select_rank(p_rank)};
  }

  size_type get_rank_of(const key_type &p_key) const {
    return m_tree_impl.get_rank_of(p_key);
  }

  size_type get_rank_of(iterator p_pos) const {
    return m_tree_impl.get_rank_of(p_pos.m_it_impl);
  }

public:
  splay_order_set() : m_tree_impl{} {}

  splay_order_set(std::initializer_list<T> p_list) : splay_order_set{} {
    insert(p_list.begin(), p_list.end());
  }

  void insert(const value_type &p_val) { m_tree_impl.insert(p_val); }

  template <typename t_input_iter>
  void insert(t_input_iter p_start, t_input_iter p_finish) {
    for (t_input_iter its = p_start, ite = p_finish; its != ite; ++its) {
      insert(*its);
    }
  }

  // Erase an element with key "p_key".
  void erase(const key_type &p_key) { m_tree_impl.erase(p_key); }

  // Erase an element pointed to by "p_pos".
  void erase(iterator p_pos) { m_tree_impl.erase(p_pos.m_it_impl); }

  // Erase all elements in range [p_start, p_finish). Must be a valid range
  void erase(iterator p_start, iterator p_finish) {
    for (iterator its = p_start, ite = p_finish; its != ite;) {
      iterator nextit = std::next(its);
      erase(its);
      its = nextit;
    }
  }

  void dump(std::ostream &p_ostream) const { m_tree_impl.dump(p_ostream); }
};

template <typename T, typename t_comp>
std::ostream &operator<<(std::ostream &p_ostream,
                         splay_order_set<T, t_comp> &p_set) {
  p_set.dump(p_ostream);
  return p_ostream;
}

} // namespace throttle
