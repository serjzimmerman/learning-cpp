/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

/* This file contains two versions of Offline LCA algorithm in application to
 * solving RMQ in bulk: recursive and iterative. Recursive algorithm is left
 * there for reference. RMQ is solved using following steps:
 * 1. Reduce RMQ to LCA by constructing a Cartesian Tree.
 * 2. Traverse the tree in DFS (more similar to Euler tour). During traversal an
 * additional array of visited flags is maintained.
 * 3. Apply Tarjan's algorithm using a Disjoint Set Union.
 *
 */

#pragma once

#include <cstddef>
#include <functional>
#include <list>
#include <unordered_map>
#include <utility>
#include <vector>

#include "cartesian_set.hpp"
#include "indexed_disjoint_map.hpp"

namespace throttle {
namespace detail {

using rmq_query = std::pair<unsigned, unsigned>;
using rmq_query_2d_vec =
    std::unordered_map<unsigned, std::vector<std::pair<unsigned, unsigned>>>;

class rmq_query_container {};

template <typename T, typename t_comp> class offline_rmq_solver_base {
protected:
  using map_type = cartesian_set<T, t_comp>;
  using map_size_type = typename map_type::size_type;
  using dsu_type = indexed_disjoint_map<map_size_type>;

  map_type m_map; // Cartesian map.
  dsu_type m_dsu; // Disjoint set.
  rmq_query_2d_vec m_queries;
  std::vector<unsigned> m_ans;

  std::vector<bool> m_visited; // Array for visited flags.

  bool visited(const typename map_type::node_proxy &p_node) const {
    return m_visited.at(p_node.index());
  }

  void set_visited(const typename map_type::node_proxy &p_node) {
    m_visited.at(p_node.index()) = true;
  }

  template <typename t_data_inp_iter, typename t_query_inp_iter>
  offline_rmq_solver_base(t_data_inp_iter p_start_dat,
                          t_data_inp_iter p_finish_dat,
                          t_query_inp_iter p_start_q,
                          t_query_inp_iter p_finish_q)
      : m_map{}, m_dsu{}, m_queries{}, m_ans{} {
    unsigned i = 0;
    for (; p_start_dat != p_finish_dat; ++p_start_dat, ++i) {
      m_map.append(*p_start_dat);
      m_dsu.append_set(0);
    }
    m_visited.resize(i, false);

    i = 0;
    for (; p_start_q != p_finish_q; ++p_start_q, ++i) {
      m_queries[p_start_q->first].push_back({p_start_q->second, i});
      m_queries[p_start_q->second].push_back({p_start_q->first, i});
    }

    m_ans.resize(i);
  }

  void
  write_ans_after_subtree_complete(typename map_type::size_type p_curr_index) {
    auto found = m_queries.find(p_curr_index);
    if ((found != m_queries.end())) {
      for (auto its = (found->second).begin(), ite = (found->second).end();
           its != ite; ++its) {
        typename map_type::node_proxy other = m_map.at(its->first);
        if (visited(other)) {
          m_ans[its->second] = *m_dsu.find_set(other.index());
        }
      }
    }
  }

public:
  std::vector<unsigned> get_ans() && { return std::move(m_ans); }
};

template <typename T, typename t_comp = std::less<T>>
class recursive_offline_rmq_solver : public offline_rmq_solver_base<T, t_comp> {
  using base = offline_rmq_solver_base<T, t_comp>;

  using typename base::dsu_type;
  using typename base::map_size_type;
  using typename base::map_type;

public:
  template <typename t_data_inp_iter, typename t_query_inp_iter>
  recursive_offline_rmq_solver(t_data_inp_iter p_start_dat,
                               t_data_inp_iter p_finish_dat,
                               t_query_inp_iter p_start_q,
                               t_query_inp_iter p_finish_q)
      : offline_rmq_solver_base<T, t_comp>{p_start_dat, p_finish_dat, p_start_q,
                                           p_finish_q} {}

private:
  void fill_ans_helper(typename map_type::node_proxy p_node) {
    base::set_visited(p_node); // Visited
    const auto curr_index = p_node.index();
    *(base::m_dsu.find_set(curr_index)) = curr_index;

    typename map_type::node_proxy left = p_node.left(), right = p_node.right();

    if (left) {
      fill_ans_helper(left);
      base::m_dsu.union_set(curr_index, left.index());
      *(base::m_dsu.find_set(curr_index)) = curr_index;
    }

    if (right) {
      fill_ans_helper(right);
      base::m_dsu.union_set(curr_index, right.index());
      *(base::m_dsu.find_set(curr_index)) = curr_index;
    }

    base::write_ans_after_subtree_complete(curr_index);
  }

public:
  void fill_ans() { fill_ans_helper(base::m_map.root()); }
};

template <typename T, typename t_comp = std::less<T>>
class iterative_offline_rmq_solver : public offline_rmq_solver_base<T, t_comp> {
  using base = offline_rmq_solver_base<T, t_comp>;

  using typename base::dsu_type;
  using typename base::map_size_type;
  using typename base::map_type;

public:
  template <typename t_data_inp_iter, typename t_query_inp_iter>
  iterative_offline_rmq_solver(t_data_inp_iter p_start_dat,
                               t_data_inp_iter p_finish_dat,
                               t_query_inp_iter p_start_q,
                               t_query_inp_iter p_finish_q)
      : offline_rmq_solver_base<T, t_comp>{p_start_dat, p_finish_dat, p_start_q,
                                           p_finish_q} {}

public:
  void fill_ans() {
    typename map_type::node_proxy curr = base::m_map.root();

    while (curr) {
      typename map_type::node_proxy left = curr.left(), right = curr.right();
      const auto curr_index = curr.index();
      const bool descending = !base::visited(curr);

      // Here we came from curr.parent() and neither left or right child have
      // been visited.
      if (descending) {
        base::set_visited(curr);
        *(base::m_dsu.find_set(curr_index)) = curr_index;

        if (left) {
          curr = left;
        } else if (right) {
          curr = right;
        } else {
          curr = curr.parent();
          base::write_ans_after_subtree_complete(curr_index);
        }
        continue;
      }

      // If there's a left node and we are ascending to the root then there are
      // 2 cases.
      if (left) {
        // 1. There isn't a right child or it hasn't been visited. Then we've
        // come from curr.left().
        if (!right || !base::visited(right)) {
          base::m_dsu.union_set(curr_index, left.index());
          *(base::m_dsu.find_set(curr_index)) = curr_index;
          if (!right)
            base::write_ans_after_subtree_complete(curr_index);
          curr = (right ? right : curr.parent());
          continue;
        }
        // 2. There is a right child and it has been visited. Then we just came
        // from it and need to continue traversing to the root.
        base::m_dsu.union_set(curr_index, right.index());
        *(base::m_dsu.find_set(curr_index)) = curr_index;
        base::write_ans_after_subtree_complete(curr_index);
        curr = curr.parent();
      }

      // There is no left child but the right one has already been traversed,
      // because otherwise this block of code wouldn't be reached.
      else {
        base::m_dsu.union_set(curr_index, right.index());
        *(base::m_dsu.find_set(curr_index)) = curr_index;
        base::write_ans_after_subtree_complete(curr_index);
        curr = curr.parent();
      }
    }
  }
};

} // namespace detail

template <typename T, typename t_comp, typename t_data_inp_iter,
          typename t_query_inp_iter>
std::vector<unsigned>
recursive_offline_rmq(t_data_inp_iter p_start_dat, t_data_inp_iter p_finish_dat,
                      t_query_inp_iter p_start_q, t_query_inp_iter p_finish_q) {
  detail::recursive_offline_rmq_solver<T, t_comp> solver{
      p_start_dat, p_finish_dat, p_start_q, p_finish_q};
  solver.fill_ans();
  return std::move(solver).get_ans();
}

template <typename T, typename t_comp, typename t_data_inp_iter,
          typename t_query_inp_iter>
std::vector<unsigned>
iterative_offline_rmq(t_data_inp_iter p_start_dat, t_data_inp_iter p_finish_dat,
                      t_query_inp_iter p_start_q, t_query_inp_iter p_finish_q) {
  detail::iterative_offline_rmq_solver<T, t_comp> solver{
      p_start_dat, p_finish_dat, p_start_q, p_finish_q};
  solver.fill_ans();
  return std::move(solver).get_ans();
}

} // namespace throttle
