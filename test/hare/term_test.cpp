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

#include "hare/term.hpp"

#include <sstream>

using namespace hare;

BOOST_AUTO_TEST_SUITE ( term_test )

const auto str = [](const auto& term) -> auto {
  std::stringstream ss;
  ss << term;
  return ss.str();
};

BOOST_AUTO_TEST_CASE ( to_string_test ) {
  const auto x = var(0, "x");
  const auto y = var(1, "y");
  const auto z = var(2, "z");
  const auto t1 = x + y;
  const auto t2 = x - ONE;
  const auto t3 = -t1;
  const auto t4 = x * t2;
  const auto t5 = z ^ t3;
  const auto t6 = t1 / t2;
  BOOST_CHECK_EQUAL(str(x),  "x");
  BOOST_CHECK_EQUAL(str(t1), "x+y");
  BOOST_CHECK_EQUAL(str(t2), "x-1");
  BOOST_CHECK_EQUAL(str(t3), "-(x+y)");
  BOOST_CHECK_EQUAL(str(t4), "x*(x-1)");
  BOOST_CHECK_EQUAL(str(t5), "z^(-(x+y))");
  BOOST_CHECK_EQUAL(str(t6), "(x+y)/(x-1)");
  BOOST_CHECK_EQUAL(str(x/ y/z) , "x/y/z");
  BOOST_CHECK_EQUAL(str(x/(y/z)), "x/(y/z)");
  BOOST_CHECK_EQUAL(str(x/val(1,2)), "x/(1/2)");
  BOOST_CHECK_EQUAL(str(val(1,2)), "1/2");
  BOOST_CHECK_EQUAL(str(x-y-z), "x-y-z");
  BOOST_CHECK_EQUAL(str(x-(y-z)), "x-(y-z)");
  BOOST_CHECK_EQUAL(str((x-y)-z), "x-y-z");
  BOOST_CHECK_EQUAL(str(x*val(3)), "x*3");
  BOOST_CHECK_EQUAL(str(val(1,2)*val(3)), "(1/2)*3");
  BOOST_CHECK_EQUAL(str(val(1,2)*val(3,4)), "(1/2)*(3/4)");             //val(1,2) is a constant and non-associative
  BOOST_CHECK_EQUAL(str((val(1)/val(2))*(val(3)/val(4))), "1/2*(3/4)"); //val(1)/val(2) is not a constant and is left-associative
  BOOST_CHECK_EQUAL(str(sin(t2)),  "sin(x-1)");
  BOOST_CHECK_EQUAL(str(cos(t3)),  "cos(-(x+y))");
  BOOST_CHECK_EQUAL(str(tan(t4)),  "tan(x*(x-1))");
  BOOST_CHECK_EQUAL(str(cot(t5)),  "cot(z^(-(x+y)))");
  BOOST_CHECK_EQUAL(str(abs(t6)),  "abs((x+y)/(x-1))");
  BOOST_CHECK_EQUAL(str(sqr(t6)),  "sqr((x+y)/(x-1))");
  BOOST_CHECK_EQUAL(str(sqrt(t6)), "sqrt((x+y)/(x-1))");
  BOOST_CHECK_EQUAL(str(cube(t6)), "cube((x+y)/(x-1))");
  BOOST_CHECK_EQUAL(str(log(t6)),  "log((x+y)/(x-1))");
  BOOST_CHECK_EQUAL(str(ln(t6)),   "ln((x+y)/(x-1))");
  BOOST_CHECK_EQUAL(str(lg(t6)),   "lg((x+y)/(x-1))");
  BOOST_CHECK_EQUAL(str(x<y)   , "x<y");
  BOOST_CHECK_EQUAL(str(x<=y)  , "x<=y");
  BOOST_CHECK_EQUAL(str(t1>y)  , "x+y>y");
  BOOST_CHECK_EQUAL(str(t2>=t3), "x-1>=-(x+y)");
  BOOST_CHECK_EQUAL(str(eq(t3,t4)), "-(x+y)==x*(x-1)");
  BOOST_CHECK_EQUAL(str(nq(t4,t5)), "x*(x-1)!=z^(-(x+y))");
  BOOST_CHECK_EQUAL(str(!(x<y && y<z))  , "!(x<y&&y<z)");
  BOOST_CHECK_EQUAL(str(x<y && y<z)     , "x<y&&y<z"  );
  BOOST_CHECK_EQUAL(str(x<y || y<z)     , "x<y||y<z"  );
  BOOST_CHECK_EQUAL(str(imply(x<y, y<z)), "x<y==>y<z" );
  BOOST_CHECK_EQUAL(str(iff  (x<y, y<z)), "x<y<==>y<z");
}

BOOST_AUTO_TEST_SUITE_END()
