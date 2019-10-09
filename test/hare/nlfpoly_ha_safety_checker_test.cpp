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

#include "hare/nlfpoly_ha_parser.hpp"
#include "hare/nlfpoly_ha_safety_checker.hpp"
#include "hare/performance_counter.hpp"
#include "hare/safety_result.hpp"

#include "cmn/misc/utils.hpp"

#include <iostream>
#include <string>
#include <tuple>
#include <vector>

using namespace ha::nlfpoly_ha;
using namespace ha::nlfpoly_ha::parser;

BOOST_AUTO_TEST_SUITE ( nlfpoly_ha_safety_checker_test )

template<typename P = NNCPoly>
bool is_safe(const std::string& text) { 
  const ha::pc::Record record{};
  const auto safety = parse_nlfpoly_safety_from_string<P>(text);
  const safety_checker<P> mc(safety);
  const auto res = mc.safety_result();
  return res == ::ha::nlfpoly_safety_result::SAFE; }

BOOST_AUTO_TEST_CASE ( simple_test ) {
  const std::string text = 
    "vars \"x, y\"                                      \n"
    "hybrid-automaton ha-1 {                            \n"
    "   loc 0 {                                         \n"
    "     inv  \"-1<x+y && x+y<=0 && 1<=x-y && x-y<=2\" \n"
    "     flow { x 1  y \"y+1\"  } }                    \n"
    "   loc 1 {                                         \n"
    "     inv  \"1<=x && x<=2 && 2<=y && y<=3\"         \n"
    "     flow { x \"x+2y\"  y \"y+1\"  } }             \n"
    "  edge \"a\" {                                     \n"
    "     src 0                                         \n"
    "     dst 1                                         \n"
    "     rel \"y==1 && y'==3 && x'==x-2\" }            \n"
    "  edge a {                                         \n"
    "     src 1                                         \n"
    "     dst 0                                         \n"
    "     rel \"y==1 && y'==0 && x'==1\" } }            \n"
    "inits {                                            \n"
    "  0 \"x==4 && y<=1\" }                             \n"
    "unsafes {                                          \n"
    "  1 \"x < 2\"   }                                  \n"
    "props {                                            \n"
    "  mc-poly {                                        \n"
    "    direction forward                              \n"
    "    separate-identity-resets true                  \n"
    "  }                                                \n"
    "  mc-nlfpoly {                                     \n"
    "    bound-cont-trans true                          \n"
    "    cont-tran-duration \"10/3\"                    \n"
    "    connect-splitted-locs false                    \n"
    "    use-empty-labels-for-bounding-time true        \n"
    "    max-iter 2        \n"
    "  }                                                \n"
    "}                                                  \n";
  BOOST_CHECK(is_safe(text));
}


BOOST_AUTO_TEST_SUITE_END()
