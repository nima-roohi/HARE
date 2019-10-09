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

#include "hare/poly_ha_parser.hpp"
#include "hare/poly_ha_safety_checker.hpp"
#include "hare/performance_counter.hpp"
#include "hare/safety_result.hpp"

#include "cmn/misc/utils.hpp"

#include <iostream>
#include <string>
#include <tuple>
#include <vector>

using namespace hare::poly_ha;
using namespace hare::poly_ha::parser;

BOOST_AUTO_TEST_SUITE ( poly_ha_safety_checker_test )

template<typename P = NNCPoly>
bool is_safe(const std::string& text) { 
  hare::pc::Record record;
  const auto safety = parse_poly_safety_from_string<P>(text);
  const safety_checker<P> mc(safety, true);
  const auto res = mc.safety_result();
  return res == ::hare::poly_safety_result::SAFE; }

BOOST_AUTO_TEST_CASE ( simple_unsafe_test ) {
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
    "  0 \"x==0 && y==0\" }                             \n"
    "unsafes {                                          \n"
    "  0 \"x > 5\"  }                                   \n"
    "props {                                            \n"
    "  mc-poly {                                        \n"
    "    direction forward                              \n"
    "  }                                                \n"
    "}                                                  \n";
  BOOST_CHECK(!is_safe(text));
  BOOST_CHECK(!is_safe(cmn::misc::utils::replace_all<std::string>(text, "forward", "backward")));
  BOOST_CHECK(!is_safe(cmn::misc::utils::replace_all<std::string>(text, "forward", "smaller-or-forward")));
  BOOST_CHECK(!is_safe(cmn::misc::utils::replace_all<std::string>(text, "forward", "smaller-or-backward")));
}

BOOST_AUTO_TEST_CASE ( simple_safe_test ) {
  const std::string text = 
    "vars \"x, y\"                                      \n"
    "hybrid-automaton ha-1 {                            \n"
    "   loc 0 {                                         \n"
    "     inv  \"y<=1 && x< 10 && x > -6\"              \n"
    "     flow \"x==1 && y==1\"  }                      \n"
    "   loc 1 {                                         \n"
    "     inv  \"y<=1 && x< 10 && x > -6\"              \n"
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
    "  0 \"x==0 && y==0\" }                             \n"
    "unsafes {                                          \n"
    "  0 \"x == 6 && y==0\"                             \n"
    "  1 \"x == 6 && y==1\"  }                          \n"
    "props {                                            \n"
    "  mc-poly {                                        \n"
    "    direction forward                              \n"
    "  }                                                \n"
    "}                                                  \n";
  BOOST_CHECK(is_safe(text));
  BOOST_CHECK(is_safe(cmn::misc::utils::replace_all<std::string>(text, "forward", "backward")));
  BOOST_CHECK(is_safe(cmn::misc::utils::replace_all<std::string>(text, "forward", "smaller-or-forward")));
  BOOST_CHECK(is_safe(cmn::misc::utils::replace_all<std::string>(text, "forward", "smaller-or-backward")));
}

BOOST_AUTO_TEST_CASE ( simple_parallel_unsafe_test ) {
  const std::string text = 
    "vars \"x, y, z\"                                   \n"
    "hybrid-automaton ha-1 {                            \n"
    "   loc 0 {                                         \n"
    "     inv  \"y<=1\"                                 \n"
    "     flow \"x==1 && y==1\"  }                      \n"
    "   loc 1 {                                         \n"
    "     inv  \"y<=1\"                                 \n"
    "     flow \"x==0.5 && y/2==1\" }                   \n"
    "  edge lbl! {                                      \n"
    "     src 0                                         \n"
    "     dst 1                                         \n"
    "     rel \"y==1 && y'==0 && x==x'\" }              \n"
    "  edge lbl? {                                      \n"
    "     src 1                                         \n"
    "     dst 0                                         \n"
    "     rel \"y==1 && y'==0 && x==x'\" } }            \n"
    "hybrid-automaton ha-2 {                            \n"
    "   loc 0 {                                         \n"
    "     inv  \"y<=1\"                                 \n"
    "     flow \"x==1 && y==1\"  }                      \n"
    "   loc 1 {                                         \n"
    "     inv  \"y<=1\"                                 \n"
    "     flow \"x==0.5 && y/2==1\" }                   \n"
    "  edge lbl? {                                      \n"
    "     src 0                                         \n"
    "     dst 1                                         \n"
    "     rel \"y==1 && y'==0 && x==x'\" }              \n"
    "  edge lbl! {                                      \n"
    "     src 1                                         \n"
    "     dst 0                                         \n"
    "     rel \"y==1 && y'==0 && x==x'\" } }            \n"
    "inits {                                            \n"
    "  [0,0] \"x==0 && y==0\" }                         \n"
    "unsafes {                                          \n"
    "  [0,0] \"x > 5\"                                  \n"
    "  [1,_] \"x > 5\"  }                               \n"
    "props {                                            \n"
    "  mc-poly {                                        \n"
    "    direction forward                              \n"
    "  }                                                \n"
    "}                                                  \n";

  BOOST_CHECK(!is_safe(text));
  BOOST_CHECK(!is_safe(cmn::misc::utils::replace_all<std::string>(text, "forward", "backward")));
  BOOST_CHECK(!is_safe(cmn::misc::utils::replace_all<std::string>(text, "forward", "smaller-or-forward")));
  BOOST_CHECK(!is_safe(cmn::misc::utils::replace_all<std::string>(text, "forward", "smaller-or-backward")));

  using T = std::tuple<std::string, std::string>;
  const auto text2 = cmn::misc::utils::replace_all<std::string>(text, std::vector<T>({ std::make_tuple("lbl?", "lbl"), std::make_tuple("lbl!", "lbl") }) );
  BOOST_CHECK(!is_safe(text2));
  BOOST_CHECK(!is_safe(cmn::misc::utils::replace_all<std::string>(text2, "forward", "backward")));
  BOOST_CHECK(!is_safe(cmn::misc::utils::replace_all<std::string>(text2, "forward", "smaller-or-forward")));
  BOOST_CHECK(!is_safe(cmn::misc::utils::replace_all<std::string>(text2, "forward", "smaller-or-backward")));
}

BOOST_AUTO_TEST_CASE ( simple_parallel_safe_test ) {
  const std::string text = 
    "vars \"x, y\"                                      \n"
    "hybrid-automaton ha-1 {                            \n"
    "   loc 0 {                                         \n"
    "     inv  \"y<=1 && x< 10 && x > -6\"              \n"
    "     flow \"x==1 && y==1\"  }                      \n"
    "   loc 1 {                                         \n"
    "     inv  \"y<=1 && x< 10 && x > -6\"              \n"
    "     flow \"x==0.5 && y/2==1\" }                   \n"
    "  edge \"a\" {                                     \n"
    "     src 0                                         \n"
    "     dst 1                                         \n"
    "     rel \"y==1 && y'==0 && x==x'\" }              \n"
    "  edge a {                                         \n"
    "     src 1                                         \n"
    "     dst 0                                         \n"
    "     rel \"y==1 && y'==0 && x==x'\" } }            \n"
    "hybrid-automaton ha-2 {                            \n"
    "   loc 0 {                                         \n"
    "     inv  \"y<=1 && x< 10 && x > -6\"              \n"
    "     flow \"x==1 && y==1\"  }                      \n"
    "   loc 1 {                                         \n"
    "     inv  \"y<=1 && x< 10 && x > -6\"              \n"
    "     flow \"x==0.5 && y/2==1\" }                   \n"
    "  edge \"a\" {                                     \n"
    "     src 0                                         \n"
    "     dst 1                                         \n"
    "     rel \"y==1 && y'==0 && x==x'\" }              \n"
    "  edge a {                                         \n"
    "     src 1                                         \n"
    "     dst 0                                         \n"
    "     rel \"y==1 && y'==0 && x==x'\" }  }           \n"
    "inits {                                            \n"
    "  [0,0] \"x==0 && y==0\" }                         \n"
    "unsafes {                                          \n"
    "  [0,0] \"x == 5 && y==1\"                         \n"
    "  [1,0] \"x == 5 && y==0\"  }                      \n"
    "props {                                            \n"
    "  mc-poly {                                        \n"
    "    direction forward                              \n"
    "  }                                                \n"
    "}                                                  \n";
  BOOST_CHECK(is_safe(text));
  BOOST_CHECK(is_safe(cmn::misc::utils::replace_all<std::string>(text, "forward", "backward")));
  BOOST_CHECK(is_safe(cmn::misc::utils::replace_all<std::string>(text, "forward", "smaller-or-forward")));
  BOOST_CHECK(is_safe(cmn::misc::utils::replace_all<std::string>(text, "forward", "smaller-or-backward")));

  using T = std::tuple<std::string, std::string>;
  const auto text2 = cmn::misc::utils::replace_all<std::string>(text, std::vector<T>({ std::make_tuple("lbl?", "lbl"), std::make_tuple("lbl!", "lbl") }) );
  BOOST_CHECK(is_safe(text2));
  BOOST_CHECK(is_safe(cmn::misc::utils::replace_all<std::string>(text2, "forward", "backward")));
  BOOST_CHECK(is_safe(cmn::misc::utils::replace_all<std::string>(text2, "forward", "smaller-or-forward")));
  BOOST_CHECK(is_safe(cmn::misc::utils::replace_all<std::string>(text2, "forward", "smaller-or-backward")));
}

BOOST_AUTO_TEST_CASE ( fischer_test ) {
  // note that I encode ID as a constant real variable. Although this lowers number of (discrete) locations, but the result code is still thousansds of times
  // slower than UPPAAL. A reason could be that I am using \c mpq_class and they are using \c int. Another reason could be that since my algorithm reduces the 
  // computations to the highest possible dimension (more general). Another reason could be that PPL switches between V-representation and H-representations 
  // after each continuous/discrete transition, while this is not reaquired for TA. Note that none these can be the whole answer, since more than 40% of 
  // computatoin times is still spent on checking for fixed-point. 
  const std::string params = "params { kk    2                        \n"
                             "         eps   0 ; 0.1                  \n"
                             "         delta 0 } ; 0.2   }            \n";
  const std::string proc = 
    "hybrid-automaton HA {                                            \n"
    "   loc 0 {                                                       \n"
    "     name init                                                   \n"
    "     flow \"1-eps<=x && x<=1+eps && id==0\"  }                   \n"
    "   loc 1 {                                                       \n"
    "     name req                                                    \n"
    "     inv  \"x <= kk+delta\"                                      \n"
    "     flow \"1-eps<=x && x<=1+eps && id==0\"  }                   \n"
    "   loc 2 {                                                       \n"
    "     name wait                                                   \n"
    "     flow \"1-eps<=x && x<=1+eps && id==0\"  }                   \n"
    "   loc 3 {                                                       \n"
    "     name cs                                                     \n"
    "     flow \"1-eps<=x && x<=1+eps && id==0\"  }                   \n"
    "  edge {                                                         \n"
    "     src 0                                                       \n"
    "     dst 1                                                       \n"
    "     rel \"id==0 && id'==id && x'==0 EQ\" }                      \n"
    "  edge {                                                         \n"
    "     src 1                                                       \n"
    "     dst 2                                                       \n"
    "     rel \"x<=kk+delta && x'==0 && id'==pid EQ\" }               \n"
    "  edge {                                                         \n"
    "     src 2                                                       \n"
    "     dst 3                                                       \n"
    "     rel \"x>=kk+1-delta && x'==x && id==pid && id'==id EQ\" }   \n"
    "  edge {                                                         \n"
    "     src 3                                                       \n"
    "     dst 0                                                       \n"
    "     rel \"x'==x && id'==0 EQ\" }                                \n"
    "  edge {                                                         \n"
    "     src 2                                                       \n"
    "     dst 1                                                       \n"
    "     rel \"id==0 && id'==id && x'==0 EQ \" } }                   \n";
  const std::string tail = 
    "props {                                                          \n"
    "  mc-poly {                                                      \n"
    "    direction                    backward                        \n"
    "    check-unsafe-after-disc-edge false                           \n"
    "    separate-identity-resets     false                           \n"
    "    add-to-visiteds-on-check     true                            \n"
    "  }                                                              \n"
    "}                                                                \n";

  const unsigned count = 5; //must be at least 2
  unsigned pid;
  std::stringstream buff;

  buff << params;

  buff << "vars \"id";
  for(pid = 0; pid < count; pid++) buff << ", x" << pid;
  buff << "\"\n";

  for(unsigned pid = 0; pid < count; pid++) {
    std::stringstream eq_buff;
    for(unsigned i = 0; i < count; i++)
      if(i!=pid) 
        eq_buff << " && x" << i << "'==x" << i; 
    auto spec = cmn::misc::utils::replace_all<std::string>(proc, "x", std::string("x") + std::to_string(pid));
         spec = cmn::misc::utils::replace_all<std::string>(spec, "pid", std::to_string(pid+1));
         spec = cmn::misc::utils::replace_all<std::string>(spec, "EQ", eq_buff.str());
    buff << spec; }

  buff << "inits { [0";
  for(unsigned pid = 1; pid < count; pid++) buff << ",0";
  buff << "] \"x0==0";
  for(unsigned pid = 1; pid < count; pid++) buff << " && x" << pid << "==0";
  buff << "\" } \n";

  buff << "unsafes { ";
  for(unsigned i = 0; i < count; i++) for(unsigned j = i + 1; j < count; j++) {
    buff << "[";
    for(unsigned k = 0; k < count; k++) {
      if(k>0) buff<<",";
      if(k==i || k==j) buff << "3";
      else buff << "_"; }
    buff << "] true \n";
  }
  buff << " } \n";

  buff << tail;
  auto parallel = buff.str();
  //std::cout << parse_poly_safety_from_string<CPoly>(parallel);
  BOOST_CHECK(is_safe<CPoly>(parallel));
  hare::pc::print_table(std::cout);
}

BOOST_AUTO_TEST_SUITE_END()
