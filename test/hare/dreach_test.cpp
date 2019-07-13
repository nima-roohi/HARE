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

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "ha/utils_dreach.hpp"
#include "ha/poly_ha_parser.hpp"
#include "ha/poly_ha_safety_checker.hpp"
#include "ha/safety_result.hpp"

using namespace ha::dreach_utils;
using namespace ha::poly_ha;
using namespace ha::poly_ha::parser;

BOOST_AUTO_TEST_SUITE ( dreach_test )

template<typename P = NNCPoly>
bool is_safe(const std::string& text) { 
  const auto safety = parse_poly_safety_from_string<P>(text);
  const safety_checker<P> mc(safety);
  const auto res = mc.safety_result();
  return res == ha::poly_safety_result::SAFE; }

BOOST_AUTO_TEST_CASE ( dreach_test ) {
  const std::string text = 
    "vars \"x, y, z\"                                   \n"
    "hybrid-automaton ha-1 {                            \n"
    "   loc 0 {                                         \n"
    "     inv  \"y<=1\"                                 \n"
    "     flow \"x==1 && y==1\"  }                      \n"
    "   loc 1 {                                         \n"
    "     inv  \"y<=1\"                                 \n"
    "     flow \"x==0.5 && y/2==1\" }                   \n"
    "  edge \"a\" {                                     \n"
    "     src 0                                         \n"
    "     dst 1                                         \n"
    "     rel \"y==1 && y'==0 && x==x'\" }              \n"
    "  edge a {                                         \n"
    "     src 1                                         \n"
    "     dst 0                                         \n"
    "     rel \"y==1 && y'==0 && x==x'\" } }            \n"
    "inits {                                            \n"
    "  _ \"x==0 && y==0\" }                             \n"
    "unsafes {                                          \n"
    "  0 \"x > 5\"                                      \n"
    "  1 \"x > 5\"  }                                   \n"
    "props {                                            \n"
    "  mc-poly {                                        \n"
    "    direction forward                              \n"
    "  }                                                \n"
    "}                                                  \n";
  BOOST_CHECK(!is_safe(text));
  parse_poly_safety_from_file<NNCPoly>("./test/file.info");
//  to_dreach(std::cout, safety);
}

BOOST_AUTO_TEST_SUITE_END()
