/**************************************************************************************************
 *                           HARE: Hybrid Abstraction Refinement Engine                           *
 *                                                                                                *
 * Copyright (C) 2019                                                                             *
 * Authors:                                                                                       *
 *   Nima Roohi <nroohi@ucsd.edu> (University of California San Diego)                            *
 *                                                                                                *
 * This program is free software: you can redistribute it and/or modify it under the terms        *
 * of the GNU General Public License as published by the Free Software Foundation, either         *
 * version 3 of the License, or (at your option) any later version.                               *
 *                                                                                                *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;      *
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.      *
 * See the GNU General Public License for more details.                                           *
 *                                                                                                *
 * You should have received a copy of the GNU General Public License along with this program.     *
 * If not, see <https://www.gnu.org/licenses/>.                                                   *
 **************************************************************************************************/

#include "cmn/dbc.hpp"
#include "cmn/logger.hpp"

#include "ha/nlfpoly_ha_parser.hpp"
#include "ha/nlfpoly_ha_safety_checker.hpp"
#include "ha/poly_ha_parser.hpp"
#include "ha/poly_ha_safety_checker.hpp"
#include "ha/performance_counter.hpp"
#include "ha/safety_result.hpp"

#include <spdlog/common.h>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <fstream>
#include <initializer_list>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/* I basically copy pasted this part from Stackoverflow */

#define VERSION_MAJOR 0
#define VERSION_MINOR 3

#define BUILD_YEAR_CH0 (__DATE__[ 7])
#define BUILD_YEAR_CH1 (__DATE__[ 8])
#define BUILD_YEAR_CH2 (__DATE__[ 9])
#define BUILD_YEAR_CH3 (__DATE__[10])

#define BUILD_MONTH_IS_JAN (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_FEB (__DATE__[0] == 'F')
#define BUILD_MONTH_IS_MAR (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r')
#define BUILD_MONTH_IS_APR (__DATE__[0] == 'A' && __DATE__[1] == 'p')
#define BUILD_MONTH_IS_MAY (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y')
#define BUILD_MONTH_IS_JUN (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_JUL (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l')
#define BUILD_MONTH_IS_AUG (__DATE__[0] == 'A' && __DATE__[1] == 'u')
#define BUILD_MONTH_IS_SEP (__DATE__[0] == 'S')
#define BUILD_MONTH_IS_OCT (__DATE__[0] == 'O')
#define BUILD_MONTH_IS_NOV (__DATE__[0] == 'N')
#define BUILD_MONTH_IS_DEC (__DATE__[0] == 'D')

#define BUILD_MONTH_CH0 ((BUILD_MONTH_IS_OCT || BUILD_MONTH_IS_NOV || BUILD_MONTH_IS_DEC) ? '1' : '0')

#define BUILD_MONTH_CH1 \
    ( \
        (BUILD_MONTH_IS_JAN) ? '1' : \
        (BUILD_MONTH_IS_FEB) ? '2' : \
        (BUILD_MONTH_IS_MAR) ? '3' : \
        (BUILD_MONTH_IS_APR) ? '4' : \
        (BUILD_MONTH_IS_MAY) ? '5' : \
        (BUILD_MONTH_IS_JUN) ? '6' : \
        (BUILD_MONTH_IS_JUL) ? '7' : \
        (BUILD_MONTH_IS_AUG) ? '8' : \
        (BUILD_MONTH_IS_SEP) ? '9' : \
        (BUILD_MONTH_IS_OCT) ? '0' : \
        (BUILD_MONTH_IS_NOV) ? '1' : \
        (BUILD_MONTH_IS_DEC) ? '2' : \
        /* error default */    '?' \
    )

#define BUILD_DAY_CH0 ((__DATE__[4] >= '0') ? (__DATE__[4]) : '0')
#define BUILD_DAY_CH1 (__DATE__[ 5])

#if VERSION_MAJOR > 100
#define VERSION_MAJOR_INIT                  \
    ((VERSION_MAJOR / 100) + '0'),          \
    (((VERSION_MAJOR % 100) / 10) + '0'),   \
    ((VERSION_MAJOR % 10) + '0')
#elif VERSION_MAJOR > 10
#define VERSION_MAJOR_INIT                  \
    ((VERSION_MAJOR / 10) + '0'),           \
    ((VERSION_MAJOR % 10) + '0')
#else
#define VERSION_MAJOR_INIT                  \
    (VERSION_MAJOR + '0')
#endif

#if VERSION_MINOR > 100
#define VERSION_MINOR_INIT                  \
    ((VERSION_MINOR / 100) + '0'),          \
    (((VERSION_MINOR % 100) / 10) + '0'),   \
    ((VERSION_MINOR % 10) + '0')
#elif VERSION_MINOR > 10
#define VERSION_MINOR_INIT                  \
    ((VERSION_MINOR / 10) + '0'),           \
    ((VERSION_MINOR % 10) + '0')
#else
#define VERSION_MINOR_INIT                  \
    (VERSION_MINOR + '0')
#endif

const unsigned char completeVersion[] = {
    VERSION_MAJOR_INIT, '.',
    VERSION_MINOR_INIT, '-',
    BUILD_YEAR_CH0, BUILD_YEAR_CH1, BUILD_YEAR_CH2, BUILD_YEAR_CH3, '.',
    BUILD_MONTH_CH0, BUILD_MONTH_CH1, '.',
    BUILD_DAY_CH0, BUILD_DAY_CH1,
    '\0'
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

using namespace ha;

using CPoly   = poly_ha::CPoly  ;
using NNCPoly = poly_ha::NNCPoly;

using nlf  = nlfpoly_safety_result;
using rect = poly_safety_result   ;

namespace po = boost::program_options;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#define HELP       "help"
#define VERSION    "version"
#define POLYHEDRAL "polyhedral"
#define NON_LINEAR "non-linear"
#define BATCH      "batch"
#define METRICS    "metrics"
#define FILEE      "file"
#define VERBOSITY  "verbosity"
#define OUTPUT     "output"

#define SHORT_HELP      "h"
#define SHORT_BATCH     "b"
#define SHORT_METRICS   "m"
#define SHORT_FILEE     "f"
#define SHORT_VERBOSITY "v"
#define SHORT_OUTPUT    "o"


//-------------------------------------------------------------------------------------------------------------------------------------------------------------

enum class verbosity : char { TRACE, DEBUG, INFO, WARN, ERROR, FATAL, OFF };

template<typename Out>
inline Out& operator<<(Out& os, const verbosity value) {
  switch(value) {
  case verbosity::TRACE : os << "trace"; return os;
  case verbosity::DEBUG : os << "debug"; return os;
  case verbosity::INFO  : os << "info" ; return os;
  case verbosity::WARN  : os << "warn" ; return os;
  case verbosity::ERROR : os << "error"; return os;
  case verbosity::FATAL : os << "fatal"; return os;
  case verbosity::OFF   : os << "off"  ; return os;
  }
}

verbosity string_to_verbosity(const std::string& str) {
  const auto pair = [](const verbosity val) { std::stringstream buff; buff << val; return std::make_pair(buff.str(), val); };
  static std::unordered_map<std::string, verbosity> map({ pair(verbosity::TRACE), pair(verbosity::DEBUG), pair(verbosity::INFO), pair(verbosity::WARN),
                                                          pair(verbosity::ERROR), pair(verbosity::FATAL), pair(verbosity::OFF ) });
  const auto it = map.find(str);
  if(it != map.end()) return it->second;
  else throw po::validation_error(po::validation_error::invalid_option_value, VERBOSITY, str); }

template<typename In>
In& operator>>(In &in, verbosity& res) {
  std::string token;
  in >> token;
  res = string_to_verbosity(token);
  return in; }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

enum class output_kind : char { SAFETY, COUNTER_EXAMPLE, ANNOTATED_COUNTER_EXAMPLE, REACHABLE_SET };

template<typename Out>
inline Out& operator<<(Out& os, const output_kind value) {
  switch(value) {
  case output_kind::SAFETY                    : os << "safety"                   ; return os;
  case output_kind::COUNTER_EXAMPLE           : os << "counter-example"          ; return os;
  case output_kind::ANNOTATED_COUNTER_EXAMPLE : os << "annotated-counter-example"; return os;
  case output_kind::REACHABLE_SET             : os << "reachable-set"            ; return os;
  }
}

output_kind string_to_output_kind(const std::string& str) {
  const auto pair = [](const output_kind val) { std::stringstream buff; buff << val; return std::make_pair(buff.str(), val); };
  static std::unordered_map<std::string, output_kind> map({ pair(output_kind::SAFETY), pair(output_kind::COUNTER_EXAMPLE), 
                                                            pair(output_kind::ANNOTATED_COUNTER_EXAMPLE),
                                                            pair(output_kind::REACHABLE_SET) });
  const auto it = map.find(str);
  if(it != map.end()) return it->second;
  else throw po::validation_error(po::validation_error::invalid_option_value, OUTPUT, str);
}

template<typename In>
In& operator>>(In &in, output_kind& res) {
  std::string token;
  in >> token;
  res = string_to_output_kind(token);
  return in; }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

enum class expected_safety_result : char { ANYTHING, UNKNOWN, UNSAFE, SAFE, BOUNDED_SAFE };

template<typename Out>
inline Out& operator<<(Out& os, const expected_safety_result value) {
  switch(value) {
  case expected_safety_result::ANYTHING    : os << "?"           ; return os;
  case expected_safety_result::UNKNOWN     : os << "unknown"     ; return os;
  case expected_safety_result::UNSAFE      : os << "unsafe"      ; return os;
  case expected_safety_result::SAFE        : os << "safe"        ; return os;
  case expected_safety_result::BOUNDED_SAFE: os << "bounded-safe"; return os;
  }
}

expected_safety_result string_to_expected_safety_result(const std::string& str) {
  const auto pair = [](const expected_safety_result val) { std::stringstream buff; buff << val; return std::make_pair(buff.str(), val); };
  static std::unordered_map<std::string, expected_safety_result> map({ pair(expected_safety_result::ANYTHING), pair(expected_safety_result::UNKNOWN), 
                                                                       pair(expected_safety_result::UNSAFE)  , pair(expected_safety_result::SAFE   ),
                                                                       pair(expected_safety_result::BOUNDED_SAFE) });
  const auto it = map.find(str);
  if(it != map.end()) return it->second;
  else throw po::validation_error(po::validation_error::invalid_option_value, OUTPUT, str);
}

template<typename In>
In& operator>>(In &in, expected_safety_result& res) {
  std::string token; 
  in >> token;
  res = string_to_expected_safety_result(token);
  return in;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename Enum, std::enable_if<std::is_enum<Enum>::value>* = nullptr>
std::string enum_to_str(const Enum& val) {
  std::stringstream buff;
  buff << val;
  return buff.str();
}

std::string concatenate(const std::initializer_list<std::string>& args) {
  std::stringstream buff;
  for(const auto& arg : args)
    buff << arg;
  return buff.str();
}

bool invalid_filepath(const std::string& file) {
  boost::filesystem::path path(file);
  return !boost::filesystem::exists(path) || !boost::filesystem::is_regular_file(path); }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

constexpr const char* AS_EXPECTED     = " (as expected)";
constexpr const char* BUT_EXPECTED    = ", but it is expected to be ";

constexpr const char* COUNTER_EXAMPLE           = "Counter-Example";
constexpr const char* ANNOTATED_COUNTER_EXAMPLE = "Annotated Counter-Example";
constexpr const char* REACHABLE_SET             = "Reachable Set";

void model_check_polyhedral_file(const std::string& file, const expected_safety_result expected, const output_kind out_kind) {
  const pc::Record record{file};
  std::cout << "checking " << file << std::endl;
  auto safety = poly_ha::parser::parse_poly_safety_from_file<NNCPoly>(file);
  const poly_ha::safety_checker<NNCPoly> mc(safety, out_kind == output_kind::ANNOTATED_COUNTER_EXAMPLE);
  const rect result = mc.safety_result();
  std::cout << "Safety Result: " << result;
  if(expected != expected_safety_result::ANYTHING) {
    const auto check = [](const rect actual, const expected_safety_result expected) {
                       switch(actual) {
                         case rect::UNSAFE       : return expected == expected_safety_result::UNSAFE      ;
                         case rect::SAFE         : return expected == expected_safety_result::SAFE        ; 
                         case rect::BOUNDED_SAFE : return expected == expected_safety_result::BOUNDED_SAFE;
                       } } ;
    if(check(result, expected)) std::cout << AS_EXPECTED  << ".";
    else                        std::cout << BUT_EXPECTED << expected << ".";  }
  std::cout << std::endl;

  if(result == rect::UNSAFE) {
    if(out_kind == output_kind::COUNTER_EXAMPLE || out_kind == output_kind::ANNOTATED_COUNTER_EXAMPLE) {
      std::cout << COUNTER_EXAMPLE << ": " << std::endl;
      mc.print_counter_example(std::cout) << std::endl; /* single-line, so need "std::endl" afterward */ }

    if(out_kind == output_kind::ANNOTATED_COUNTER_EXAMPLE) {
      std::cout << ANNOTATED_COUNTER_EXAMPLE << ": " << std::endl;
      mc.print_annotated_counter_example(std::cout); /* multiline, so no need for "std::endl" afterward */ }
  }

  if(out_kind == output_kind::REACHABLE_SET) {
    std::cout << REACHABLE_SET;
    if(result == rect::BOUNDED_SAFE) std::cout << " (after bounded number of iterations)";
    if(result == rect::UNSAFE)       std::cout << " (up to the point an unsafe state is found reachable)";
    std::cout << ": " << std::endl;
    mc.print_reachable_states(std::cout); /* multiline, so no need for "std::endl" afterward */ }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

void model_check_non_linear_file(const std::string& file, const expected_safety_result expected, const output_kind out_kind) {
  const pc::Record record{file};
  std::cout << "checking " << file << std::endl;
  auto safety = nlfpoly_ha::parser::parse_nlfpoly_safety_from_file<NNCPoly>(file);
//std::cout << safety << std::endl;
  const nlfpoly_ha::safety_checker<NNCPoly> mc(safety);
  const nlf result = mc.safety_result();
  std::cout << "Safety Result: " << result; 
  if(expected != expected_safety_result::ANYTHING) {
    const auto check = [](const nlf actual, const expected_safety_result expected) {
                       switch(actual) {
                         case nlf::UNKNOWN               : return expected == expected_safety_result::UNKNOWN     ;
                         case nlf::UNSAFE                : return expected == expected_safety_result::UNSAFE      ;
                         case nlf::SAFE                  : return expected == expected_safety_result::SAFE        ; 
                         case nlf::ABSTRACT_BOUNDED_SAFE : return expected == expected_safety_result::BOUNDED_SAFE;
                       } } ;
    if(check(result, expected)) std::cout << AS_EXPECTED  << ".";
    else                        std::cout << BUT_EXPECTED << expected << ".";  }
  std::cout << std::endl;

  if(result == nlf::UNSAFE) {
    if(out_kind == output_kind::COUNTER_EXAMPLE || out_kind == output_kind::ANNOTATED_COUNTER_EXAMPLE) {
      std::cout << "Abstract " << COUNTER_EXAMPLE << ": " << std::endl;
      mc.print_abstract_counter_example(std::cout) << std::endl; /* single-line, so need "std::endl" afterward */ 
      std::cout << "Concrete " << COUNTER_EXAMPLE << ": " << std::endl;
      mc.print_concrete_counter_example(std::cout) << std::endl; /* single-line, so need "std::endl" afterward */ }

    if(out_kind == output_kind::ANNOTATED_COUNTER_EXAMPLE) {
      std::cout << "Abstract " << ANNOTATED_COUNTER_EXAMPLE << ": " << std::endl;
      mc.print_annotated_counter_example(std::cout); /* multiline, so no need for "std::endl" afterward */ }
  }

  if(out_kind == output_kind::REACHABLE_SET) {
    std::cout << "Abstract " << REACHABLE_SET;
    if(result == nlf::ABSTRACT_BOUNDED_SAFE) std::cout << " (after bounded number of iterations)";
    if(result == nlf::UNSAFE)                std::cout << " (up to the point an unsafe state is found reachable)";
    if(result == nlf::UNKNOWN)               std::cout << "(when maximum iterations is reached)";
    std::cout << ": " << std::endl;
    mc.print_reachable_states(std::cout); /* multiline, so no need for "std::endl" afterward */ }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

void model_check_batch_file(const std::string& batch_file, const output_kind out_kind) { 
  std::ifstream file(batch_file);
  std::string line;
  std::uint64_t line_num = 0;
  while(std::getline(file, line)) {
    line_num++;
    line = cmn::misc::utils::trim(line);
    if(line.empty() || line[0] == '#') continue;
    const auto p1 = line.find(",",    0);
    const auto p2 = line.find(",", p1+1); 
    const auto p3 = line.find(",", p2+1);
    const auto p4 = line.find("#", p3+1);
    requires_msg(p1 != std::string::npos && p2 != std::string::npos && p3 != std::string::npos, "Incorrect data at line " << line_num) 
    requires_msg(p1 < p2 && p2 < p3 && p3 < p4, "Incorrect data at line " << line_num) 
    const auto name     = cmn::misc::utils::trim(line.substr(   0, p1         ));
    const auto type     = cmn::misc::utils::trim(line.substr(p1+1, p2 - p1 - 1));
    const auto expected = cmn::misc::utils::trim(line.substr(p2+1, p3 - p2 - 1));
    const auto path     = cmn::misc::utils::trim(line.substr(p3+1, p4 - p3 - 1));
    
    const bool polyhedral = type == "polyhedral";
    const bool non_linear = type == "non-linear";
    if(!polyhedral && !non_linear) {
      log_error << "invalid type \"" << type << "\" at line " << line_num << ". This line will be ignored.";
      continue;
    }

    expected_safety_result exp;
    try { 
      exp = string_to_expected_safety_result(expected);
    } catch(const po::validation_error& e) {
      log_error << "invalid expected value at line " << line_num << ". This line will be ignored.";
      continue; }

    if(invalid_filepath(path)) {
      log_error << "cannot read from file " << path << " at line " << line_num << ". This line will be ignored.";
      continue; }

    if(polyhedral) model_check_polyhedral_file(path, exp, out_kind);
    else           model_check_non_linear_file(path, exp, out_kind);
  }
  file.close(); 
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

int main(const int argc, const char *argv[]) {

  po::options_description desc("Allowed Options (example: hare --non-linear --metrics --file ./nav01/nav.prb)");
  desc.add_options()
      (HELP  "," SHORT_HELP     , "shows the usage")
      (VERSION                  , "print the version")
      (POLYHEDRAL               , "input file specifies a polyhedral hybrid automaton (everything is polyhedron)")
      (NON_LINEAR               , "input file specifies a non-linear hybrid automaton (everything is polyhedron except flows that are either affine or "
                                  "non-linear ODEs)")
      (BATCH "," SHORT_BATCH    , "batch processing.\n"
                                  "Non-empty lines in the input file that are not started with the character #, each contains 4 elements (comma separated):\n"
                                  "1. \tname (need not be unique),\n"
                                  "2. \ttype of input (polyhedral or non-linear),\n"
                                  "3. \texpected answer (safe, unsafe, bounded-safe, unknown, ?).\n"
                                  "   i)  \tExpected answer 'unknown' only makes sense if the input type is non-linear.\n"
                                  "   ii) \tExpected answer '?' means there is no expected answer.\n"
                                  "4. \t(full) path to the model file.")
      (METRICS   "," SHORT_METRICS  , "print performance metrics after successfull termination")
      (FILEE     "," SHORT_FILEE    , po::value<std::string>(), "input file")
      (VERBOSITY "," SHORT_VERBOSITY, po::value<verbosity>()->default_value(verbosity::DEBUG), 
                                      "log level (trace, debug, info, warn, error, fatal, off).")
      (OUTPUT    "," SHORT_OUTPUT   , po::value<output_kind>()->default_value(output_kind::SAFETY), 
                                      concatenate({ "kind of information to be generated on successful termination. Possible values are:\n",
                                                    "1. \t", enum_to_str(output_kind::SAFETY), ": "  
                                                             "whether or not the system is safe.\n"
                                                    "2. \t", enum_to_str(output_kind::COUNTER_EXAMPLE), ": "
                                                             "in addition to the previous part, in case the input was unsafe, a counter-example (sequence of "
                                                             "edges) will also be generated.\n"
                                                    "3. \t", enum_to_str(output_kind::ANNOTATED_COUNTER_EXAMPLE), ": "  
                                                             "in addition to the previous part, in case the input was unsafe, (abstract) states reachable "
                                                             "along with the generated counter example will also be generated.\n"
                                                    "4. \t", enum_to_str(output_kind::REACHABLE_SET), ": "
                                                             "(abstract) reachable states whether or not the system is found to be safe."

                                                  } ).c_str() ) 
  ;

  po::variables_map vm;

  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm); 

    if(vm.count(HELP) > 0 || vm.count("-" SHORT_HELP) > 0) {
      std::cout << desc << std::endl;
      return 0; }

    if(vm.count(VERSION) > 0) {
      std::cout << "Hybrid Abstraction Refinement Engine (HARE) version " << completeVersion << std::endl;
      return 0; }   

    const verbosity level = vm[vm.count("-" SHORT_VERBOSITY) > 0 ? "-" SHORT_VERBOSITY : VERBOSITY].as<verbosity>();
    switch(level) {
    case verbosity::TRACE : cmn::default_logger().set_level(spdlog::level::trace   ); break;
    case verbosity::DEBUG : cmn::default_logger().set_level(spdlog::level::debug   ); break;
    case verbosity::INFO  : cmn::default_logger().set_level(spdlog::level::info    ); break;
    case verbosity::WARN  : cmn::default_logger().set_level(spdlog::level::warn    ); break;
    case verbosity::ERROR : cmn::default_logger().set_level(spdlog::level::err     ); break;
    case verbosity::FATAL : cmn::default_logger().set_level(spdlog::level::critical); break;
    case verbosity::OFF   : cmn::default_logger().set_level(spdlog::level::off     ); break;
    }

    const output_kind out_kind = vm[vm.count("-" SHORT_OUTPUT) > 0 ? "-" SHORT_OUTPUT : OUTPUT].as<output_kind>();

    const bool polyhedral = vm.count(POLYHEDRAL) > 0;
    const bool non_linear = vm.count(NON_LINEAR) > 0;
    const bool batch      = vm.count(BATCH)      > 0 || vm.count("-" SHORT_BATCH)   > 0;
    const bool metrics    = vm.count(METRICS)    > 0 || vm.count("-" SHORT_METRICS) > 0;

    if(polyhedral && non_linear) {
      std::cerr << "At most one of options \"" << POLYHEDRAL << "\" and \"" << NON_LINEAR << "\" can be given." << std::endl;
      return 1; }

    if(batch && (polyhedral || non_linear)) {
      std::cerr << "Option \"" << BATCH << "\" cannot be specified simultaneously with \"" << POLYHEDRAL << "\" or \"" << NON_LINEAR << "." << std::endl;
      return 2; }

    if(!batch && !polyhedral && !non_linear) {
      std::cerr << "When Option \"" << BATCH << "\" is not set, one of options \"" << POLYHEDRAL << "\" or \"" << NON_LINEAR << " must be set." << std::endl;
      return 2; }

    const auto file_count = vm.count(FILEE);
    const auto f_count    = vm.count(SHORT_FILEE);
    if(file_count != 1 && f_count != 1) {
      std::cout << "Option --" << FILEE << " is not specified." << std::endl;
      return 3; }
    if(file_count == 1 && f_count == 1) {
      std::cout << "Options --" << FILEE << " and -" << SHORT_FILEE << " cannot be set simultaneously." << std::endl;
      return 4; }

    const std::string file = file_count == 1 ? 
                             vm[FILEE          ].as<std::string>() : 
                             vm["-" SHORT_FILEE].as<std::string>() ; 
    if (invalid_filepath(file)) {
      std::cerr << "Cannot read the input file: " << file << std::endl;
      return 5; }

    log_debug << "input file: " << file;
    if(polyhedral) model_check_polyhedral_file(file, expected_safety_result::ANYTHING, out_kind); else
    if(non_linear) model_check_non_linear_file(file, expected_safety_result::ANYTHING, out_kind); else
    if(batch)      model_check_batch_file(file, out_kind); else
    asserts_msg(false, "none of polyhedral, non_linear, or batch parameters are set.") 

    if(metrics) {
      if(batch) ha::pc::print_table      (std::cout);
      else      ha::pc::print_curr_record(std::cout); }

  } catch(po::error& e) { 
    std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
    std::cerr << desc << std::endl; 
    return -1;
  } catch(std::exception& e) { 
    std::cerr << "Unhandled Exception reached the top of main: " 
              << e.what() << ", application will now exit" << std::endl; 
    return -2; } 

  #undef HELP       
  #undef VERSION    
  #undef POLYHEDRAL 
  #undef NON_LINEAR 
  #undef BATCH      
  #undef METRICS    
  #undef FILEE      
  #undef VERBOSITY  

  #undef SHORT_HELP    
  #undef SHORT_BATCH   
  #undef SHORT_METRICS 
  #undef SHORT_FILEE   
}
