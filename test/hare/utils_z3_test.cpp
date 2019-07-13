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

#include "ha/term.hpp"
#include "ha/term_parser.hpp"
#include "ha/utils_z3.hpp"

#include <z3++.h>

using namespace ha;
using namespace ha::parser;
using namespace ha::z3_utils;

BOOST_AUTO_TEST_SUITE ( z3_utils_test )

z3::context ctx;

const auto n0  = val( 0);
const auto nm1 = val(-1);
const auto n1  = val( 1);
const auto n2  = val( 2);
const auto x = var(0, "x");
const auto y = var(1, "y");
const auto z = var(2, "z");
const name2var n2v({ {"x",x}, {"y",y}, {"z",z} });

template<std::size_t N> auto parse(const char(&buff)[N]) { return parse_term(buff, buff + N - 1, n2v); }
template<std::size_t N> auto arith(const char(&buff)[N]) { return to_arith_expr(parse(buff), ctx); }
template<std::size_t N> auto boool(const char(&buff)[N]) { return to_bool_expr (parse(buff), ctx); }
                        auto term (const z3::expr& expr) { return to_term      (expr       , n2v); }

const auto zx = ctx.real_const("x");
const auto zy = ctx.real_const("y");
const auto zz = ctx.real_const("z");

const auto zn0  = ctx.real_val( 0);
const auto zn1  = ctx.real_val( 1);
const auto znm1 = ctx.real_val(-1);
const auto zn2  = ctx.real_val( 2);

const auto T  = ctx.bool_val(true);
const auto F  = ctx.bool_val(false);
const auto bx = ctx.bool_const("x");
const auto by = ctx.bool_const("y");
const auto bz = ctx.bool_const("z");

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE ( term_to_arith_test ) {
  const auto test = [](const auto& str, const auto& expected) -> std::string { 
    const auto actual = arith(str);
    const auto expr   = actual - expected;
    const auto smpl   = expr.simplify();
    std::stringstream buff;
    buff << smpl;
    if(buff.str() == "0.0") return "";
    std::stringstream res;
    res << "input: " << str << ", result: " << actual << ", expected: " << expected << ", simplified: " << smpl;
    return res.str();  };
#define CHECK(act, exp) {             \
const auto res = test(act, exp);      \
BOOST_CHECK_MESSAGE(res == "", res);  }

  CHECK("x", zx)
  CHECK("y", zy)
  CHECK("z", zz)

  CHECK(" 0", zn0 )
  CHECK(" 1", zn1 )
  CHECK("-1", znm1)
  CHECK(" 2", zn2 )

  CHECK(" -x",   -zx)
  CHECK("x+y", zx+zy)
  CHECK("x-y", zx-zy)
  CHECK("x*y", zx*zy)
  CHECK("x/y", zx/zy)
  CHECK("x^y", z3::pw(zx,zy))
  CHECK("x+y-z", zx+zy-zz)
  CHECK("x(y-z)", zx*(zy-zz))
  CHECK("(x+y)(y-z)", (zx+zy)*(zy-zz))

  CHECK("sqr (x+y)", (zx+zy)*(zx+zy))
  CHECK("cube(x+y)", (zx+zy)*(zx+zy)*(zx+zy))

#undef CHECK
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE ( term_to_bool_test ) {
  const auto test = [](const auto& str, const auto& expected) -> std::string { 
    const auto actual = boool(str);
    const auto expr   = actual && !expected;
          auto solver = z3::solver(ctx); solver.add(expr);
    const auto res    = solver.check();
    switch(res) {
      case z3::unsat   : return "";
      case z3::sat     : 
      case z3::unknown : std::stringstream buff;
                         buff << "input: " << str << ", result: " << actual << ", expected: " << expected << ", res: " << res;
                         return buff.str(); } };
#define CHECK(act, exp) {             \
const auto res = test(act, exp);      \
BOOST_CHECK_MESSAGE(res == "", res);  }

  CHECK("true" , T)
  CHECK("false", F)
  CHECK("x", bx)
  CHECK("y", by)
  CHECK("z", bz)

  CHECK(" !x",   !bx)
  CHECK("x&&y", bx && by)
  CHECK("x||y", bx || by)
  CHECK("x==>y", !bx || by)
  CHECK("x<==>y", (!bx || by) && (!by || bx))
  CHECK("x==>y==>z",  (!bx || !by || bz))
  CHECK("x && y || z", bx && by || bz);
  CHECK("x && (y || z)", bx && (by || bz));

#undef CHECK
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE ( expr_to_term_test ) {

  BOOST_CHECK_EQUAL(term(zx), x);
  BOOST_CHECK_EQUAL(term(zy), y);
  BOOST_CHECK_EQUAL(term(zz), z);

  BOOST_CHECK_EQUAL(term(zn0 ), n0 );
  BOOST_CHECK_EQUAL(term(zn1 ), n1 );
  BOOST_CHECK_EQUAL(term(znm1), nm1);
  BOOST_CHECK_EQUAL(term(zn2 ), n2 );

  BOOST_CHECK_EQUAL(term( -zx),   -x);
  BOOST_CHECK_EQUAL(term(zx+zy), x+y);
  BOOST_CHECK_EQUAL(term(zx-zy), x-y);
  BOOST_CHECK_EQUAL(term(zx*zy), x*y);
  BOOST_CHECK_EQUAL(term(zx/zy), x/y);
  BOOST_CHECK_EQUAL(term(z3::pw(zx,zy)), x^y);
  BOOST_CHECK_EQUAL(term(zx+zy-zz), x+y-z);
  BOOST_CHECK_EQUAL(term(zx*(zy-zz)), x*(y-z));
  BOOST_CHECK_EQUAL(term((zx+zy)*(zy-zz)), (x+y)*(y-z));

  BOOST_CHECK_EQUAL(term((zx+zy)*(zx+zy)), (x+y)*(x+y));
  BOOST_CHECK_EQUAL(term((zx+zy)*(zx+zy)*(zx+zy)), (x+y)*(x+y)*(x+y));

  BOOST_CHECK_EQUAL(term(T), TRUE );
  BOOST_CHECK_EQUAL(term(F), FALSE);
  BOOST_CHECK_EQUAL(term(zx), x);
  BOOST_CHECK_EQUAL(term(zy), y);
  BOOST_CHECK_EQUAL(term(zz), z);
  BOOST_CHECK_EQUAL(term(bx), x);
  BOOST_CHECK_EQUAL(term(by), y);
  BOOST_CHECK_EQUAL(term(bz), z);

  BOOST_CHECK_EQUAL(term(!bx), !x);
  BOOST_CHECK_EQUAL(term( bx && by),    x && y);
  BOOST_CHECK_EQUAL(term( bx || by),    x || y);
  BOOST_CHECK_EQUAL(term(!bx || by),   !x || y);
  BOOST_CHECK_EQUAL(term((!bx || by) && (!by || bx)),   (!x || y) && (!y || x));
  BOOST_CHECK_EQUAL(term(!bx || !by || bz),   !x || !y || z );
  BOOST_CHECK_EQUAL(term( bx &&  by || bz) ,   x &&  y || z );
  BOOST_CHECK_EQUAL(term( bx && (by || bz)),   x && (y || z));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
