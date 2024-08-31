/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "datastructures/disjoint_set_forest.hpp"
#include "datastructures/ud_asymmetric_graph.hpp"
#include "datastructures/vector.hpp"
#include "equal.hpp"
#include "linmath/contiguous_matrix.hpp"
#include "linmath/linear_solver.hpp"
#include "linmath/matrix.hpp"

#include <algorithm>
#include <exception>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <boost/functional/hash.hpp>

#pragma once

namespace throttle::circuits {

using resistance_emf_pair = std::pair<double, double>;

class circuit_error : public std::exception {
  std::string m_message;

public:
  circuit_error(const std::string &msg) : m_message{msg} {}
  const char *what() const noexcept override { return m_message.c_str(); }
};

namespace detail {
template <typename T> class connected_resistor_network {
public:
  struct short_circuit_edge {
    T first, second;
    double emf;
  };

  using circuit_graph_type =
      containers::ud_asymmetric_graph<T, resistance_emf_pair>;
  using solution_potentials = std::unordered_map<T, double>;
  using solution_currents =
      std::unordered_map<T, std::unordered_map<T, double>>;
  using solution = std::pair<solution_potentials, solution_currents>;

private:
  circuit_graph_type m_graph;
  std::vector<short_circuit_edge> m_short_circuits;

  struct connected_resistor_network_solver {
    using system_type = linmath::linear_equation_system<double>;
    using equation_type = typename system_type::equation_type;
    using mapped_pair = std::pair<unsigned, unsigned>;

    const connected_resistor_network &network;

    std::unordered_map<T, unsigned>
        id_map; // Maps identifier from input to 0, 1, ...
    std::unordered_map<unsigned, T>
        inverse_id_map; // Maps 0, 1, ... to the input identifiers
    std::unordered_map<mapped_pair, std::pair<unsigned, double>,
                       boost::hash<mapped_pair>>
        short_circuit_current_map; // Maps pairs of input indexes to current
                                   // variable
    const typename circuit_graph_type::size_type vertices, num_short_circuits;
    unsigned zero_potential_mapped_id;

    connected_resistor_network_solver(const connected_resistor_network &circuit)
        : network{circuit}, vertices{network.m_graph.vertices()},
          num_short_circuits{network.m_short_circuits.size() / 2} {
      for (auto j = 0; const auto &v : network.m_graph) {
        id_map[v.first] = j;
        inverse_id_map[j++] = v.first;
      }

      for (auto j = vertices; const auto &v : network.m_short_circuits) {
        const auto mapped_first = id_map.at(v.second),
                   mapped_second = id_map.at(v.first);
        mapped_pair canonical_pair = {
            mapped_first < mapped_second
                ? mapped_pair{mapped_first, mapped_second}
                : mapped_pair{mapped_second, mapped_first}};
        if (short_circuit_current_map.contains(canonical_pair))
          continue;
        short_circuit_current_map.insert({canonical_pair, {j++, v.emf}});
      }

      zero_potential_mapped_id = id_map.at(network.m_graph.begin()->first);
    }

    auto make_system() const {
      const auto variables = vertices + num_short_circuits;

      // This is the linear system that we are constructing.
      system_type system;
      equation_type equation(variables);
      equation[zero_potential_mapped_id] = 1.0;
      system.push(equation);
      equation.reset();

      // Step 1. Iterate over all points and use vertex current method to
      // construct a linear equation.
      for (auto start = std::next(network.m_graph.begin()),
                finish = network.m_graph.end();
           start != finish; ++start) {
        const auto &v = *start;

        const auto &[current_id, adj_map] = v;
        const auto current_mapped_id = id_map.at(current_id);

        // Step 2. Find all vertices that are adjacent to the current one.
        for (const auto &a : adj_map) {
          const auto [res, emf] = a.second;
          const auto second_id = a.first;
          const auto second_mapped_id = id_map.at(second_id);
          const bool is_second_zero_potential =
              (second_mapped_id == zero_potential_mapped_id);

          if (throttle::is_roughly_equal(res, 0.0)) {
            const bool first_less_second = current_mapped_id < second_mapped_id;
            const auto current_var =
                (short_circuit_current_map.at(
                     (first_less_second)
                         ? std::make_pair(current_mapped_id, second_mapped_id)
                         : std::make_pair(second_mapped_id, current_mapped_id)))
                    .first;
            equation[current_var] += (first_less_second ? 1.0 : -1.0);
            continue;
          }

          const auto conductivity = 1.0 / res;

          equation[current_mapped_id] += conductivity;
          if (!is_second_zero_potential) {
            equation[second_mapped_id] -= conductivity;
          }

          equation.free_coeff() -= emf / res;
        }

        system.push(equation);
        equation.reset();
      }

      for (const auto &v : short_circuit_current_map) {
        const auto [first_id, second_id] = v.first;
        const auto emf = v.second.second;
        if (first_id != zero_potential_mapped_id)
          equation[first_id] = 1.0;
        if (second_id != zero_potential_mapped_id)
          equation[second_id] = -1.0;

        equation.free_coeff() = -emf;
        system.push(equation);
        equation.reset();
      }

      return system;
    }

    solution solve() const {
      if (network.m_graph.empty())
        return solution{}; // If the network is empty, then there's nothing to
                           // do

      // Solve the linear system of equations to find unkown potentials and
      // currents.
      auto system = make_system();

      auto res = system.solve();
      if (!res)
        throw circuit_error{
            "The circuit is undefined. Possible infinite current loop"};
      auto unknowns = system.solve().value();

      // Fill base node potential with zero.
      auto result_potentials = solution_potentials{};
      for (const auto &v : network.m_graph) {
        const auto id = v.first;
        const auto mapped = id_map.at(id);
        result_potentials[id] = unknowns[mapped][0];
      }

      // Fill unkown currents that were found as a part of linear system of
      // m_equations.
      auto result_currents = solution_currents{};

      for (const auto &v : short_circuit_current_map) {
        const auto fwd_current = unknowns[v.second.first][0];
        const auto first_original_id = inverse_id_map.at(v.first.first);
        const auto second_original_id = inverse_id_map.at(v.first.second);
        result_currents[first_original_id][second_original_id] = fwd_current;
        result_currents[second_original_id][first_original_id] = -fwd_current;
      }

      // Compute other currents from potentials, when there are no
      // short-circuits.
      for (const auto &v : network.m_graph) {
        for (const auto &c : v.second) {
          const auto first_id = v.first, second_id = c.first;
          if (throttle::is_roughly_equal(c.second.first, 0.0))
            continue;
          const auto [res, emf] = c.second;
          const auto fwd_current = (result_potentials[first_id] -
                                    result_potentials[second_id] + emf) /
                                   res;
          result_currents[first_id][second_id] = fwd_current;
        }
      }

      return solution{result_potentials, result_currents};
    }
  };

  friend connected_resistor_network_solver;

public:
  // The graph passed to this constructor must consist of only one connected
  // components. This requirement is not validated in any way whatsoever. This
  // class is not supposed to be used by the end user.
  connected_resistor_network(circuit_graph_type graph) : m_graph{graph} {
    for (const auto &v : m_graph) {
      for (const auto &p : v.second) {
        auto first = v.first, second = p.first;
        auto [res, emf] = p.second;
        if (throttle::is_roughly_equal(res, 0.0))
          m_short_circuits.push_back({first, second, emf});
      }
    }
  }

  circuit_graph_type graph() const { return m_graph; }

  solution solve() const {
    connected_resistor_network_solver solver{*this};
    return solver.solve();
  }
};
} // namespace detail

template <typename T> class resistor_network {
public:
  using connected_network_type = detail::connected_resistor_network<T>;
  using circuit_graph_type =
      typename connected_network_type::circuit_graph_type;
  using solution_potentials =
      typename connected_network_type::solution_potentials;
  using solution_currents = typename connected_network_type::solution_currents;
  using solution = std::pair<solution_potentials, solution_currents>;

private:
  circuit_graph_type m_graph;

public:
  std::vector<connected_network_type> connected_components() const {
    auto components = m_graph.connected_components();
    return {components.begin(), components.end()};
  }

  circuit_graph_type graph() const { return m_graph; }

  solution solve() const {
    auto components = connected_components();
    solution result;

    for (const auto &comp : components) {
      auto individual_sol = comp.solve();
      result.first.merge(individual_sol.first);
      result.second.merge(individual_sol.second);
    }

    return result;
  }

  void insert(T first, T second, double resistance = 0, double emf = 0) {
    resistance_emf_pair fwd_pair = {resistance, emf},
                        bck_pair = {resistance, -emf};
    m_graph.insert_edge({first, second}, fwd_pair, bck_pair);
  }
};

} // namespace throttle::circuits
