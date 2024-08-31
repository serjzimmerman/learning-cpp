#include <boost/lexical_cast/bad_lexical_cast.hpp>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <set>
#include <string>
#include <tuple>

#include "datastructures/vector.hpp"
#include "linmath/contiguous_matrix.hpp"
#include "linmath/matrix.hpp"

#include <concepts>
#include <optional>
#include <sstream>
#include <string>

#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

#include <boost/program_options.hpp>
#include <boost/program_options/option.hpp>

#if 0

#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>
#include <boost/spirit/home/x3/support/utility/error_reporting.hpp>

#else

#include "driver.hpp"

#endif

#include "circuits/resistor_network.hpp"
#include "linmath/linear_solver.hpp"

namespace po = boost::program_options;

#if 0

namespace x3 = boost::spirit::x3;
namespace ascii = x3::ascii;

namespace circuit_parser {

namespace graph {
struct network_edge : x3::position_tagged {
  unsigned                first, second;
  double                  res;
  boost::optional<double> emf;
};

} // namespace graph

struct error_handler {
  x3::error_handler_result on_error(auto &, auto const &, auto const &x, auto const &context) {
    auto       &error_handler = x3::get<x3::error_handler_tag>(context).get();
    std::string message = "Parsing error! Expecting: '" + x.which() + "' here:";
    error_handler(x.where(), message);
    return x3::error_handler_result::fail;
  }
};

} // namespace circuit_parser

BOOST_FUSION_ADAPT_STRUCT(circuit_parser::graph::network_edge, first, second, res, emf)

namespace circuit_parser {

struct rule_d : error_handler {};
struct rule_u : error_handler {};

constexpr x3::rule<rule_d, double>   double_named = {"double"};
constexpr x3::rule<rule_u, unsigned> unsigned_named = {"unsigned"};

const auto double_named_def = x3::real_parser<double>{};
const auto unsigned_named_def = x3::int_parser<unsigned>{};

BOOST_SPIRIT_DEFINE(double_named, unsigned_named);

struct edge_class;

constexpr x3::rule<edge_class, circuit_parser::graph::network_edge> edge = "edge";
constexpr auto edge_def = unsigned_named > '-' > '-' > unsigned_named > ',' > double_named > ';' >>
                          -(double_named >> 'V' >> -x3::lit(';'));

BOOST_SPIRIT_DEFINE(edge);

struct edge_class : x3::annotate_on_success, error_handler {};

std::optional<std::vector<circuit_parser::graph::network_edge>> parse_circuit() {
  std::vector<circuit_parser::graph::network_edge> parse_result;

  using ascii::space;

  using x3::phrase_parse;
  using x3::with;
  using iter_type = std::string::const_iterator;

  std::string input;
  std::noskipws(std::cin);
  std::copy(std::istream_iterator<char>{std::cin}, std::istream_iterator<char>{}, std::back_inserter(input));

  using error_handler_type = x3::error_handler<iter_type>;
  error_handler_type error_handler{input.begin(), input.end(), std::cerr};

  const auto parser = with<x3::error_handler_tag>(std::ref(error_handler))[+edge];
  bool       res = phrase_parse(input.begin(), input.end(), parser, space, parse_result);

  if (!res) return std::nullopt;

  return parse_result;
}

} // namespace circuit_parser

#else

namespace circuit_parser {

std::vector<circuits::network_edge> parse_circuit() {
  std::vector<circuits::network_edge> parse_result;

  std::string input;
  std::noskipws(std::cin);
  std::copy(std::istream_iterator<char>{std::cin},
            std::istream_iterator<char>{}, std::back_inserter(input));

  circuits::driver drv{};

  std::istringstream iss{input};
  drv.switch_input_stream(&iss);
  drv.parse();

  return drv.m_parsed;
}

} // namespace circuit_parser

#endif

int calculate_currents(auto circuit, bool verbose) {
  using network_type = throttle::circuits::resistor_network<unsigned>;
  network_type network;

  std::unordered_map<unsigned, unsigned> input_vertex_to_mapped;
  std::unordered_map<unsigned, unsigned> mapped_to_input_vertex;

  unsigned count = 0;

  const auto get_or_insert = [&count, &input_vertex_to_mapped,
                              &mapped_to_input_vertex](unsigned index) {
    if (input_vertex_to_mapped.contains(index)) {
      return input_vertex_to_mapped.at(index);
    }

    auto this_index = count++;
    input_vertex_to_mapped[index] = this_index;
    mapped_to_input_vertex[this_index] = index;

    return this_index;
  };

  std::vector<std::tuple<unsigned, unsigned, unsigned, unsigned>>
      currents_to_print;

  for (const auto &v : circuit) {
    auto first_mapped = get_or_insert(v.first),
         second_mapped = get_or_insert(v.second);
    auto temporary_first = count++, temporary_second = count++;
    network.insert(first_mapped, temporary_first);
    network.insert(temporary_first, temporary_second, v.res,
                   v.emf.value_or(0.0));
    network.insert(temporary_second, second_mapped);
    currents_to_print.push_back(
        std::make_tuple(temporary_first, temporary_second, v.first, v.second));
  }

  network_type::solution_currents currents;
  try {
    currents = network.solve().second;
  } catch (throttle::circuits::circuit_error &e) {
    std::cerr << "Bad circuit, bailing out. Here's the error message: "
              << e.what() << "\n";
    return EXIT_FAILURE;
  }

  for (const auto &v : currents_to_print) {
    auto [temp_1, temp_2, name_1, name_2] = v;
    constexpr auto precision = 1e-6;

    auto current = currents.at(temp_1).at(temp_2);
    auto rounded_current = (std::abs(current) > precision ? current : 0.0);

    if (verbose) {
      std::cout << name_1 << " -- " << name_2 << ": " << rounded_current
                << " A\n";
    } else {
      std::cout << rounded_current << "\n";
    }
  }

  return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) try {
  bool non_verbose = false;

  po::options_description desc("Available options");
  desc.add_options()("help,h", "Print this help message")("nonverbose,n",
                                                          "Non-verbose output");
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  non_verbose = vm.count("nonverbose");
  auto parsed = circuit_parser::parse_circuit();
  return calculate_currents(parsed, !non_verbose);
} catch (std::exception &e) {
  std::cerr << "Encountered error: " << e.what() << "\n";
} catch (...) {
  std::cerr << "Unknown error\n";
}
