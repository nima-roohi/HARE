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

#include "hare/utils_nlfpoly_ha.hpp"
#include "hare/nlfpoly_ha_parser.hpp"

#include <iostream>
#include <string>
#include <tuple>
#include <vector>

using namespace hare::nlfpoly_ha;
using namespace hare::nlfpoly_ha::parser;

BOOST_AUTO_TEST_SUITE ( utils_nlfpoly_ha_test )

template<typename P = NNCPoly>
auto parse(const std::string& text) { 
  return parse_nlfpoly_safety_from_string<P>(text); }

template<typename Safety>
auto deep_equal(const Safety& fst, const Safety& snd) {
  if(fst != snd) return false;
  for(auto i = fst.ha_seq().size(); i > 0;) {
    const auto& ha1 = fst.ha(--i);
    const auto& ha2 = snd.ha(  i);
    for(auto j = ha1.locs().size(); j > 0;) {
      const auto& q1 = ha1.loc(--j);
      const auto& q2 = ha2.loc(  j);
      if(q1.nq(q2)) return false; 
    }
  }
  return true;
}

BOOST_AUTO_TEST_CASE ( is_normal_and_normalize_test ) {
  const std::string text = 
    "vars \"x, y\"                                      \n"
    "hybrid-automaton ha-1 {                            \n"
    "   loc 0 {                                         \n"
    "     inv  \"y<=1\"                                 \n"
    "     flow { x 1 y 0 } }                            \n"
    "   loc 1 {                                         \n"
    "     inv  \"y<=2\"                                 \n"
    "     flow { x \"sin(x)\" y \"x+1\" } }             \n"
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
    "  1 \"y > 5\"  }                                   \n"
    "props {                                            \n"
    "  mc-poly {                                        \n"
    "    direction forward                              \n"
    "  }                                                \n"
    "}                                                  \n";
  auto safety = parse<NNCPoly>(text);
  BOOST_CHECK(!is_normal(safety));  normalize(safety);
  BOOST_CHECK( is_normal(safety));
}

BOOST_AUTO_TEST_CASE ( closeness_openness_test_test ) {
  const std::string text = 
    "vars \"x, y\"                                      \n"
    "hybrid-automaton ha-1 {                            \n"
    "   loc 0 {                                         \n"
    "     inv  \"y<=1\"                                 \n"
    "     flow { x \"-x\" y \"1/x\" } }                 \n"
    "   loc 1 {                                         \n"
    "     inv  \"y<=2\"                                 \n"
    "     flow { x \"abs(x)/tan(x)\" y \"1/sin(x)\" } } \n"
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
    "  1 \"y > 5\"  }                                   \n"
    "props {                                            \n"
    "  mc-poly {                                        \n"
    "    direction forward                              \n"
    "  }                                                \n"
    "}                                                  \n";
  auto safety = parse<NNCPoly>(text);
  BOOST_CHECK(!is_closed(safety));
  BOOST_CHECK(!is_open  (safety));
  BOOST_CHECK( is_closed(closed_safety(safety)));
  BOOST_CHECK( is_open  (open_safety  (safety)));
}

BOOST_AUTO_TEST_CASE ( inverse_time_test ) {
  const std::string forward = 
    "vars \"x, y\"                                      \n"
    "hybrid-automaton ha-1 {                            \n"
    "   loc 0 {                                         \n"
    "     inv  \"x>0 && y<=1\"                          \n"
    "     flow { x 1 y \"x\" } }                        \n"
    "   loc 1 {                                         \n"
    "     inv  \"y<=2\"                                 \n"
    "     flow {                                        \n"
    "       x \"1/sin(x)+(y^2)\"                        \n"
    "       y \"x+y-x-y\" } }                           \n"
    "  edge \"a\" {                                     \n"
    "     src 0                                         \n"
    "     dst 1                                         \n"
    "     rel \"y==1 && y'==0 && x==x'\" }              \n"
    "  edge a {                                         \n"
    "     src 1                                         \n"
    "     dst 0                                         \n"
    "     rel \"x+y<1 && 2x-y>5y'+4x'\" } }             \n"
    "inits {                                            \n"
    "  1 \"x==0 && y==0\" }                             \n"
    "unsafes {                                          \n"
    "  0 \"x > 5\"                                      \n"
    "  1 \"y > 5\"  }                                   \n"
    "props {                                            \n"
    "  mc-poly {                                        \n"
    "    direction forward                              \n"
    "  }                                                \n"
    "}                                                  \n";
  const std::string backward = 
    "vars \"x, y\"                                      \n"
    "hybrid-automaton ha-1 {                            \n"
    "   loc 0 {                                         \n"
    "     inv  \"x>0 && y<=1\"                          \n"
    "     flow { x 1 y \"-x\" } }                       \n"
    "   loc 1 {                                         \n"
    "     inv  \"y<=2\"                                 \n"
    "     flow {                                        \n"
    "       x \"1/sin(-x)+((-y)^2)\"                    \n"
    "       y \"- x + -y - -x - -y\" } }                \n"
    "  edge \"a\" {                                     \n"
    "     src 1                                         \n"
    "     dst 0                                         \n"
    "     rel \"y'== 1 && y==0 && x'==x\" }             \n"
    "  edge a {                                         \n"
    "     src 0                                         \n"
    "     dst 1                                         \n"
    "     rel \"x'+y'<1 && 2x'-y'>5y+4x\" } }           \n"
    "unsafes {                                          \n"
    "  1 \"x==0 && y==0\" }                             \n"
    "inits {                                            \n"
    "  0 \"x > 5\"                                      \n"
    "  1 \"y > 5\"  }                                   \n"
    "props {                                            \n"
    "  mc-poly {                                        \n"
    "    direction forward                              \n"
    "  }                                                \n"
    "}                                                  \n";
  auto actual   = parse<NNCPoly>(forward ); inverse_time(actual);
  auto expected = parse<NNCPoly>(backward);
  BOOST_CHECK_MESSAGE(deep_equal(actual, expected), "\nACTUAL:\n" << actual << "Expected:\n" << expected);
  auto actual2   = closed_safety(parse<NNCPoly>(forward )); inverse_time(actual2);
  auto expected2 = closed_safety(parse<NNCPoly>(backward));
  BOOST_CHECK_MESSAGE(deep_equal(actual2, expected2), "\nACTUAL:\n" << actual2 << "Expected:\n" << expected2);
  auto actual3   = open_safety(parse<NNCPoly>(forward )); inverse_time(actual3);
  auto expected3 = open_safety(parse<NNCPoly>(backward));
  BOOST_CHECK_MESSAGE(deep_equal(actual3, expected3), "\nACTUAL:\n" << actual3 << "Expected:\n" << expected3);
}

BOOST_AUTO_TEST_CASE ( has_only_linear_flows_test ) {
  const std::string text1 = 
    "vars \"x, y\"                                      \n"
    "hybrid-automaton ha-1 {                            \n"
    "   loc 0 {                                         \n"
    "     inv  \"y<=1\"                                 \n"
    "     flow { x \"-x\" y \"1/x\" } }                 \n"
    "   loc 1 {                                         \n"
    "     inv  \"y<=2\"                                 \n"
    "     flow { x \"abs(x)/tan(x)\" y \"1/sin(x)\" } } \n"
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
    "  1 \"y > 5\"  }                                   \n"
    "props {                                            \n"
    "  mc-poly {                                        \n"
    "    direction forward                              \n"
    "  }                                                \n"
    "}                                                  \n";
  const std::string text2 = 
    "vars \"x, y\"                                      \n"
    "hybrid-automaton ha-1 {                            \n"
    "   loc 0 {                                         \n"
    "     inv  \"y<=1\"                                 \n"
    "     flow { x \"-x\" y \"1+x\" } }                 \n"
    "   loc 1 {                                         \n"
    "     inv  \"y<=2\"                                 \n"
    "     flow { x \"x*3/4+y/3-3x\" y \"sqr(4)\" } }    \n"
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
    "  1 \"y > 5\"  }                                   \n"
    "props {                                            \n"
    "  mc-poly {                                        \n"
    "    direction forward                              \n"
    "  }                                                \n"
    "}                                                  \n";
  const std::string text3 = 
    "vars \"x, y\"                                      \n"
    "hybrid-automaton ha-1 {                            \n"
    "   loc 0 {                                         \n"
    "     inv  \"y<=1\"                                 \n"
    "     flow { x \"0\" y \"1\" } }                    \n"
    "   loc 1 {                                         \n"
    "     inv  \"y<=2\"                                 \n"
    "     flow { x \"2^3^sqr(4)\" y \"x+4\" } } \n"
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
    "  1 \"y > 5\"  }                                   \n"
    "props {                                            \n"
    "  mc-poly {                                        \n"
    "    direction forward                              \n"
    "  }                                                \n"
    "}                                                  \n";
  BOOST_CHECK(!has_only_linear_flows(parse(text1)));
  BOOST_CHECK( has_only_linear_flows(parse(text2)));
  BOOST_CHECK( has_only_linear_flows(parse(text3)));
}


BOOST_AUTO_TEST_SUITE_END()
