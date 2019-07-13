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
#include "ha/utils_term.hpp"

#include <cmath>

using namespace ha;

BOOST_AUTO_TEST_SUITE ( term_utils_test )

const auto x   = var(0, "x");
const auto y   = var(1, "y");
const auto z   = var(2, "z");
const auto t = x*y;
const auto n0  = val( 0);
const auto nm1 = val(-1);
const auto nm2 = val(-2);
const auto n1  = val( 1);
const auto n2  = val( 2);
const auto n3  = val( 3);

BOOST_AUTO_TEST_CASE ( evaluate_test ) {
  BOOST_CHECK_EQUAL(evaluate(n0), mpq_class(0));
  BOOST_CHECK_EQUAL(evaluate(n1), mpq_class(1));
  BOOST_CHECK_EQUAL(evaluate(n2), mpq_class(2));
  BOOST_CHECK_EQUAL(evaluate(n1*n2)       , mpq_class(2));
  BOOST_CHECK_EQUAL(evaluate(n1*n2-n2)    , mpq_class(0));
  BOOST_CHECK_EQUAL(evaluate(n1*n2-n2*n2) , mpq_class(-2));
  BOOST_CHECK_EQUAL(evaluate(nm1*n2-n2*n2), mpq_class(-6));
  BOOST_CHECK_EQUAL(evaluate(nm2*n2-n2*n2), mpq_class(-8));
  BOOST_CHECK_EQUAL(evaluate(n1/n2)       , mpq_class(1,2));
  BOOST_CHECK_EQUAL(evaluate(n1/(n1+n2))  , mpq_class(1,3));
  BOOST_CHECK_EQUAL(evaluate(n1/(n1+n2)*nm2), mpq_class(-2,3));
}

BOOST_AUTO_TEST_CASE ( lcm_of_denominators_test ) {
  BOOST_CHECK_EQUAL(lcm_of_denominators(n0), mpz_class(1));
  BOOST_CHECK_EQUAL(lcm_of_denominators(n1), mpz_class(1));
  BOOST_CHECK_EQUAL(lcm_of_denominators(n2), mpz_class(1));
  BOOST_CHECK_EQUAL(lcm_of_denominators(n1*n2)       , mpz_class(1));
  BOOST_CHECK_EQUAL(lcm_of_denominators(n1*n2-n2)    , mpz_class(1));
  BOOST_CHECK_EQUAL(lcm_of_denominators(n1*n2-n2*n2) , mpz_class(1));
  BOOST_CHECK_EQUAL(lcm_of_denominators(nm1*n2-n2*n2), mpz_class(1));
  BOOST_CHECK_EQUAL(lcm_of_denominators(nm2*n2-n2*n2), mpz_class(1));
  BOOST_CHECK_EQUAL(lcm_of_denominators(n1/n2)       , mpz_class(2));
  BOOST_CHECK_EQUAL(lcm_of_denominators(n1/(n1+n2))  , mpz_class(3));
  BOOST_CHECK_EQUAL(lcm_of_denominators(n1/(n1+n2)*nm2), mpz_class(3));
  BOOST_CHECK_EQUAL(lcm_of_denominators(n1/(n1+n2)+(n1/(n1+n2))), mpz_class(3));
  BOOST_CHECK_EQUAL(lcm_of_denominators(n1/(n1+n2)*(n1/(n1+n2))), mpz_class(9));
  BOOST_CHECK_EQUAL(lcm_of_denominators(n1/(n2*n2)+(n1/(n1+n2))), mpz_class(12));
  BOOST_CHECK_EQUAL(lcm_of_denominators(n3/(n2*n2)+(n1/(n1+n2))), mpz_class(12));
  BOOST_CHECK_EQUAL(lcm_of_denominators(n2/(n2*n2)+(n1/(n1+n2))), mpz_class(12));
  BOOST_CHECK_EQUAL(lcm_of_denominators((x/(n2*n2))/(val(7)/(n1+n2))), mpz_class(28));
  BOOST_CHECK_EQUAL(lcm_of_denominators((x/(n2*n2))/(val(7)/n2)), mpz_class(28));
  BOOST_CHECK_EQUAL(lcm_of_denominators(PI), mpz_class( 50));
  BOOST_CHECK_EQUAL(lcm_of_denominators(E ), mpz_class(100));
  BOOST_CHECK_EQUAL(lcm_of_denominators(E+PI), mpz_class(100));
  BOOST_CHECK_EQUAL(lcm_of_denominators(E+PI<n1), mpz_class(100));
}

BOOST_AUTO_TEST_CASE ( negate_variables_test ) {
  BOOST_CHECK_EQUAL(negate_variables(x)     ,  -x);
  BOOST_CHECK_EQUAL(negate_variables(x+n2)  ,  -x+n2);
  BOOST_CHECK_EQUAL(negate_variables(x+y)   ,  (-x)+(-y));
  BOOST_CHECK_EQUAL(negate_variables(sin(x)),  sin(-x));
  BOOST_CHECK_EQUAL(negate_variables(x*y)   ,  (-x)*(-y));
  BOOST_CHECK_EQUAL(negate_variables(x*y*y) ,  (-x)*(-y)*(-y));
  BOOST_CHECK_EQUAL(negate_variables(t+t) ,  (-x)*(-y) + (-x)*(-y));
}

BOOST_AUTO_TEST_CASE ( is_linear_test ) {
  BOOST_CHECK( is_linear(PI));
  BOOST_CHECK( is_linear(E));
  BOOST_CHECK( is_linear(TRUE));
  BOOST_CHECK( is_linear(FALSE));
  BOOST_CHECK( is_linear(n0));
  BOOST_CHECK( is_linear(n1));
  BOOST_CHECK( is_linear(x));
  BOOST_CHECK( is_linear(-x));
  BOOST_CHECK( is_linear(x+x));
  BOOST_CHECK( is_linear(x-n2*x));
  BOOST_CHECK( is_linear(n2^n3^n2));

  BOOST_CHECK( is_linear(x >  y));
  BOOST_CHECK( is_linear(x >= y));
  BOOST_CHECK( is_linear(x <  y/n2));
  BOOST_CHECK( is_linear(x <= y + sqr(n2)));
  BOOST_CHECK( is_linear(eq(x, y)));
  BOOST_CHECK( is_linear(nq(x, y)));
  BOOST_CHECK(!is_linear(x >  y^n2));
  BOOST_CHECK(!is_linear(x >= y*x));
  BOOST_CHECK(!is_linear(n1/x <  y));
  BOOST_CHECK(!is_linear(x <= n1/x));
  BOOST_CHECK(!is_linear(eq(x+sqrt(x), y)));
  BOOST_CHECK(!is_linear(nq(x*y, y)));

  BOOST_CHECK(!is_linear(lg  (n1)));
  BOOST_CHECK(!is_linear(ln  (n1)));
  BOOST_CHECK(!is_linear(log (n1)));
  BOOST_CHECK(!is_linear(sin (n1)));
  BOOST_CHECK(!is_linear(cos (n1)));
  BOOST_CHECK(!is_linear(tan (n1)));
  BOOST_CHECK(!is_linear(cot (n1)));
  BOOST_CHECK(!is_linear(sqrt(n1)));
  BOOST_CHECK(!is_linear(lg  (x )));
  BOOST_CHECK(!is_linear(ln  (x )));
  BOOST_CHECK(!is_linear(log (x )));
  BOOST_CHECK(!is_linear(sin (x )));
  BOOST_CHECK(!is_linear(cos (x )));
  BOOST_CHECK(!is_linear(tan (x )));
  BOOST_CHECK(!is_linear(cot (x )));
  BOOST_CHECK(!is_linear(sqrt(x )));

  BOOST_CHECK(!is_linear(abs (x )));
  BOOST_CHECK(!is_linear(sqr (x )));
  BOOST_CHECK(!is_linear(cube(x )));
  BOOST_CHECK( is_linear(abs (n1)));
  BOOST_CHECK( is_linear(sqr (n1)));
  BOOST_CHECK( is_linear(cube(n1)));
}

BOOST_AUTO_TEST_CASE ( bound_of_expr_test ) {
  using D = double;
  using boost_interval = boost_interval_t<D>;
  using limits = std::numeric_limits<D>; 
  const std::unordered_map<term_ptr, boost_interval> bounds({ {x, to_boost_interval<D>(interval_t(constant(1), constant(2)))}, 
                                                              {y, to_boost_interval<D>(interval_t(constant(1), constant(2)))}, 
                                                              {z, to_boost_interval<D>(interval_t(constant(1), constant(3)))}  });
  #define CHECK(expr, inf_, sup_) {                                         \
    const auto actual = bound_of_expr<D>(bounds, expr);                     \
    const auto lower = actual.lower();                                      \
    const auto upper = actual.upper();                                      \
    const auto inf   = inf_;                                                \
    const auto sup   = sup_;                                                \
    const auto inf_check = std::abs(lower - inf) < 0.0001 ||                \
                          (limits::lowest() > lower &&                      \
                           limits::lowest() > inf);                         \
    const auto sup_check = std::abs(upper - sup) < 0.0001 ||                \
                          (limits::max() < upper &&                         \
                           limits::max() < sup);                            \
    BOOST_CHECK_MESSAGE(inf_check && sup_check,                             \
                        "expr: "      << #expr << ", " <<                   \
                        "expected: [" << inf   << ", " << sup   << "], "    \
                        "actual ["    << lower << ", " << upper << "]"); }
  CHECK(x, 1, 2)
  CHECK(y, 1, 2)
  CHECK(z, 1, 3)
  CHECK(-y, -2, -1)
  CHECK(x+y, 2, 4)
  CHECK(x-y, -1, 1)
  CHECK(x*y, 1, 4)
  CHECK(x/y, 0.5, 2)
  CHECK(x/(x-y), -limits::infinity(), limits::infinity())
  CHECK(sin(x), std::sin(1), 1)
  CHECK(cos(x), std::cos(2), std::cos(1))
  CHECK(tan(x), -limits::infinity(), limits::infinity())
  CHECK(PI, 3.14159265359, 3.14159265359)
  CHECK(E, 2.71828182846, 2.71828182846)
  CHECK(ln(E), 1, 1)
  CHECK(log(E), 0.4342944819 , 0.4342944819)
  CHECK(lg (E), 1.44269504089, 1.44269504089)
  CHECK(sqr(x), 1, 4)
  CHECK(sqrt(x), 1, std::sqrt(2))
  CHECK(cube(x), 1, 8)
  CHECK(abs(-x), 1, 2)
  CHECK(x^y, 1, 4)
  CHECK(x^(y^z), 1, 256)
  CHECK(ln(z-x-y), -limits::infinity(), 0)
  #undef CHECK
}

BOOST_AUTO_TEST_SUITE_END()
