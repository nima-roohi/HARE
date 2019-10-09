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
#include "hare/term_parser.hpp"

#include <sstream>

using namespace hare;
using namespace hare::parser;

BOOST_AUTO_TEST_SUITE ( term_parser_test )

const auto x  = var(0, "x");
const auto y  = var(1, "y");
const auto z  = var(2, "z");
const auto p  = var(2, "p");
const auto pe = var(3, "pe");
const name2var n2v({ {"x",x}, {"y",y}, {"z",z}, {"p",p}, {"pe",pe} });

template<std::size_t N>
auto parse(const char(&buff)[N]) {
  return parse_term(buff, buff + N - 1, n2v);
}

BOOST_AUTO_TEST_CASE ( parse_term_test ) {
  BOOST_CHECK_EQUAL(parse("x"), x);
  BOOST_CHECK_EQUAL(parse("y"), y);
  BOOST_CHECK_EQUAL(parse("z"), z);

  BOOST_CHECK_EQUAL(parse("true "), TRUE );
  BOOST_CHECK_EQUAL(parse("false"), FALSE);

  BOOST_CHECK_EQUAL(parse("0"), val(0));
  BOOST_CHECK_EQUAL(parse("1"), val(1));
  BOOST_CHECK_EQUAL(parse("2"), val(2));
  BOOST_CHECK_EQUAL(parse("10"), val(10));

  BOOST_CHECK_EQUAL(parse("pe"), pe);
  BOOST_CHECK_EQUAL(parse("p e"), p*E);
  BOOST_CHECK_EQUAL(parse("2pe"), val(2)*pe);

  BOOST_CHECK_EQUAL(parse("-x"), -x);
  BOOST_CHECK_EQUAL(parse("!x"), !x);

  BOOST_CHECK_EQUAL(parse("1+y"), val(1) + y);
  BOOST_CHECK_EQUAL(parse("x+2"), x + val(2));
  BOOST_CHECK_EQUAL(parse("x y"), x * y);

  BOOST_CHECK_EQUAL(parse("x+y"), x + y);
  BOOST_CHECK_EQUAL(parse("x-y"), x - y);
  BOOST_CHECK_EQUAL(parse("x*y"), x * y);
  BOOST_CHECK_EQUAL(parse("x y"), x * y);
  BOOST_CHECK_EQUAL(parse("x/y"), x / y);
  BOOST_CHECK_EQUAL(parse("x^y"), x ^ y);
  BOOST_CHECK_EQUAL(parse("x && y"), x && y);
  BOOST_CHECK_EQUAL(parse("x || y"), x || y);
  BOOST_CHECK_EQUAL(parse("x ==>  y"), imply(x, y));
  BOOST_CHECK_EQUAL(parse("x <==> y"), iff  (x, y));

  BOOST_CHECK_EQUAL(parse("x+y+z"), x + y + z);
  BOOST_CHECK_EQUAL(parse("x+y-z"), x + y - z);
  BOOST_CHECK_EQUAL(parse("x-y+z"), x - y + z);
  BOOST_CHECK_EQUAL(parse("-x-y+z"), -x - y + z);
  BOOST_CHECK_EQUAL(parse("-x-y+z"), (-x) - y + z);

  BOOST_CHECK_EQUAL(parse("(x)"), x);
  BOOST_CHECK_EQUAL(parse("(x+y)"), x + y);
  BOOST_CHECK_EQUAL(parse("(x-y)"), x - y);

  BOOST_CHECK_EQUAL(parse("abs (x)"), abs (x));
  BOOST_CHECK_EQUAL(parse("sqrt(x)"), sqrt(x));
  BOOST_CHECK_EQUAL(parse("sqr (x)"), sqr (x));
  BOOST_CHECK_EQUAL(parse("cube(x)"), cube(x));
  BOOST_CHECK_EQUAL(parse("log (x)"), log (x));
  BOOST_CHECK_EQUAL(parse("ln  (x)"), ln  (x));
  BOOST_CHECK_EQUAL(parse("lg  (x)"), lg  (x));
  BOOST_CHECK_EQUAL(parse("sin (x)"), sin (x));
  BOOST_CHECK_EQUAL(parse("cos (x)"), cos (x));
  BOOST_CHECK_EQUAL(parse("tan (x)"), tan (x));
  BOOST_CHECK_EQUAL(parse("cot (x)"), cot (x));

  BOOST_CHECK_EQUAL(parse("log  x"), log (x));
  BOOST_CHECK_EQUAL(parse("abs  -1 "), abs (-val(1)));
  BOOST_CHECK_EQUAL(parse("sqrt  2 "), sqrt( val(2)));
  BOOST_CHECK_EQUAL(parse("sqr  2x "), sqr (val(2))*x);
  BOOST_CHECK_EQUAL(parse("cube x^y"), cube(x)^y);
  BOOST_CHECK_EQUAL(parse("abs sin 1"), abs(sin(val(1))));
  BOOST_CHECK_EQUAL(parse("ln   x+y"), ln  (x)+y);
  BOOST_CHECK_EQUAL(parse("lg   x-y"), lg  (x)-y);
  BOOST_CHECK_EQUAL(parse("sin  -x-y"), sin (-x)-y);
  BOOST_CHECK_EQUAL(parse("cos  x/y"), cos(x)/y);
  BOOST_CHECK_EQUAL(parse("tan (x)/y"), tan(x)/y);
  BOOST_CHECK_EQUAL(parse("(cot x)/y"), cot(x)/y);
  BOOST_CHECK_EQUAL(parse("cot (x/y)"), cot(x/y));

  BOOST_CHECK_EQUAL(parse("- - x"), - - x);
  BOOST_CHECK_EQUAL(parse("sin(x)cos(y)"), sin(x)*cos(y));
  BOOST_CHECK_EQUAL(parse("sin(x)*cos(y)"), sin(x)*cos(y));
  BOOST_CHECK_EQUAL(parse("sin(x)cos(y)tan(z)"), sin(x)*cos(y)*tan(z));
  BOOST_CHECK_EQUAL(parse("sin(x)(cos(y)tan(z))"), sin(x)*(cos(y)*tan(z)));

  BOOST_CHECK_EQUAL(parse("x*y+z"), x * y + z);
  BOOST_CHECK_EQUAL(parse("2y-z"), val(2)* y - z);
  BOOST_CHECK_EQUAL(parse("x y+z"), x * y + z);
  BOOST_CHECK_EQUAL(parse("x(y+z)"), x*(y+z));
  BOOST_CHECK_EQUAL(parse("(y+z)*x"), (y+z)*x);
  BOOST_CHECK_EQUAL(parse("(x+y)*(y+z)"), (x+y)*(y+z));
  BOOST_CHECK_EQUAL(parse("(x+y)(y+z)"), (x+y)*(y+z));
  BOOST_CHECK_EQUAL(parse("sin (x+y)(y+z)"), sin(x+y)*(y+z));
  BOOST_CHECK_EQUAL(parse("1/sin(x)"), val(1)/sin(x));
  BOOST_CHECK_EQUAL(parse("x/sin x"), x/sin(x));

  BOOST_CHECK_EQUAL(parse("x >  y"), x >  y);
  BOOST_CHECK_EQUAL(parse("x >= y"), x >= y);
  BOOST_CHECK_EQUAL(parse("x <  y"), x <  y);
  BOOST_CHECK_EQUAL(parse("x <= y"), x <= y);
  BOOST_CHECK_EQUAL(parse("x == y"), eq(x, y));
  BOOST_CHECK_EQUAL(parse("x != y"), nq(x, y));

  BOOST_CHECK_EQUAL(parse("x >  y + z"), x >  y + z);
  BOOST_CHECK_EQUAL(parse("x >= y - z"), x >= y - z);
  BOOST_CHECK_EQUAL(parse("x / z <  y"), x / z <  y);
  BOOST_CHECK_EQUAL(parse("sin(x) <= y"), sin(x) <= y);
  BOOST_CHECK_EQUAL(parse("sin(x)cos(y) > z"), sin(x)*cos(y) > z);
  BOOST_CHECK_EQUAL(parse("tan(x+y < 3)"), tan(x+y<val(3))); // untyped term!

  BOOST_CHECK_EQUAL(parse("x^y^z"), x^(y^z));
  BOOST_CHECK_EQUAL(parse("x<==>y==>z"), iff(x,imply(y,z)));
  BOOST_CHECK_EQUAL(parse("x ==> y ==> z"), imply(x, imply(y,z)));
  BOOST_CHECK_EQUAL(parse("x >  12y + 23/3z && x < y z || sin(x)^3 < 5 ==> x <= 4 <==> true ==> false"), 
                          iff(
                          imply(x > val(12)*y + val(23)/val(3)*z && x < y*z || (sin(x)^val(3)) < val(5), x <= val(4)),
                          imply(TRUE, FALSE)));
}

BOOST_AUTO_TEST_SUITE_END()
