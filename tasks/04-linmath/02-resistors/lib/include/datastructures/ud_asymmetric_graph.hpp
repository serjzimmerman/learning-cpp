/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "disjoint_set_forest.hpp"
#include "vector.hpp"

#include <functional>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace throttle::containers {

template <typename T, typename U, typename t_comp = std::equal_to<T>,
          typename t_hash = std::hash<T>>
class ud_asymmetric_graph {
public:
  using size_type = std::size_t;
  using self_type = ud_asymmetric_graph<T, U, t_comp, t_hash>;

private:
  using adjecency_map = std::unordered_map<T, U, t_hash, t_comp>;
  using adjecency_map_it = typename adjecency_map::iterator;
  using const_adjecency_map_it = typename adjecency_map::const_iterator;

  std::unordered_map<T, adjecency_map> m_vertices;

  t_comp m_comp;
  size_type m_edges = 0;

public:
  ud_asymmetric_graph() = default;

  using iterator = typename decltype(m_vertices)::iterator;
  using const_iterator = typename decltype(m_vertices)::const_iterator;

  auto insert_vertex(T identifier) {
    return m_vertices.insert({identifier, {}});
  }

  void insert_edge(std::pair<T, T> vert, U attr_fwd, U attr_bck) {
    auto [first_id, second_id] = vert;
    if (m_comp(first_id, second_id))
      throw std::invalid_argument{"Graph can't have self loops"};

    auto first_found = m_vertices.find(first_id);
    auto second_found = m_vertices.find(second_id);

    if (first_found == m_vertices.end()) {
      auto insert_first = insert_vertex(first_id);
      first_found = insert_first.first;
    }

    if (second_found == m_vertices.end()) {
      auto insert_second = insert_vertex(second_id);
      second_found = insert_second.first;
    }

    auto &first_adj = first_found->second;
    auto &second_adj = second_found->second;

    auto fwd_edge_found = first_adj.find(second_id);
    if (fwd_edge_found != first_adj.end()) {
      first_adj.at(second_id) = attr_fwd;
      second_adj.at(first_id) = attr_bck;
      return;
    }

    first_adj.insert({second_id, attr_fwd});
    second_adj.insert({first_id, attr_bck});
    ++m_edges;
  }

  size_type vertices() const { return m_vertices.size(); }
  size_type edges() const { return m_edges; }

  bool contains_vertex(const T &id) const { return m_vertices.contains(id); }
  bool contains_edge(std::pair<T, T> vert) const {
    return lookup_edge(vert).has_value();
  }

  std::optional<std::pair<adjecency_map_it, adjecency_map_it>>
  lookup_edge(std::pair<T, T> vert) {
    auto [first_id, second_id] = vert;

    if (m_comp(first_id, second_id))
      return std::nullopt;

    auto first_found = m_vertices.find(first_id);
    auto second_found = m_vertices.find(second_id);

    if (first_found == m_vertices.end() || second_found == m_vertices.end())
      return std::nullopt;

    auto &first_adj = first_found->second;
    auto &second_adj = second_found->second;
    auto fwd_edge_found = first_adj.find(second_id);

    if (fwd_edge_found != first_adj.end()) {
      return std::make_pair(fwd_edge_found, second_adj.find(first_id));
    }

    return std::nullopt;
  }

  std::optional<std::pair<const_adjecency_map_it, const_adjecency_map_it>>
  lookup_edge(std::pair<T, T> vert) const {
    auto [first_id, second_id] = vert;

    if (m_comp(first_id, second_id))
      return std::nullopt;

    auto first_found = m_vertices.find(first_id);
    auto second_found = m_vertices.find(second_id);

    if (first_found == m_vertices.end() || second_found == m_vertices.end())
      return std::nullopt;

    auto &first_adj = first_found->second;
    auto &second_adj = second_found->second;
    auto fwd_edge_found = first_adj.find(second_id);

    if (fwd_edge_found != first_adj.end()) {
      return std::make_pair(fwd_edge_found, second_adj.find(first_id));
    }

    return std::nullopt;
  }

  std::vector<self_type> connected_components() const {
    containers::disjoint_set_forest<T> dsu;

    for (const auto &v : m_vertices) {
      dsu.make_set(v.first);
    }

    for (const auto &v : m_vertices) {
      for (const auto &p : v.second) {
        // Here we do double work because each edge is visited twice, but this
        // part isn't perfomance critical.
        dsu.union_set(v.first, p.first);
      }
    }

    std::unordered_map<T, self_type> connected_representatives;
    // find_set does not change the component representaive. Here we iterate
    // over all the nodes and find their representaive to find all connected
    // components.
    for (const auto &v : m_vertices) {
      const auto &found = dsu.find_set(v.first);
      if (connected_representatives.contains(found))
        continue;
      connected_representatives.insert({found, self_type{}});
    }

    for (const auto &v : m_vertices) {
      const auto &found = dsu.find_set(v.first);
      connected_representatives[found].insert_vertex(v.first);
      for (const auto &p : v.second) {
        auto [fwd_edge, bck_edge] = lookup_edge({v.first, p.first}).value();
        connected_representatives[found].insert_edge(
            {v.first, p.first}, fwd_edge->second, bck_edge->second);
      }
    }

    std::vector<self_type> result;
    for (const auto &v : connected_representatives) {
      result.push_back(std::move(v.second));
    }

    return result;
  }

  auto begin() { return m_vertices.begin(); }
  auto end() { return m_vertices.end(); }
  auto begin() const { return m_vertices.cbegin(); }
  auto end() const { return m_vertices.cend(); }
  auto cbegin() const { return m_vertices.cbegin(); }
  auto cend() const { return m_vertices.cend(); }

  bool empty() const { return (vertices() == 0); }
};

} // namespace throttle::containers
