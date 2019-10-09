#ifndef CMN__MATH__LOG__HPP
#define CMN__MATH__LOG__HPP

#include "cmn/dbc.hpp"

namespace cmn::math {

/** @brief evaluates the natural logarithm using the CORDIC method.
  * This code is distributed under the GNU LGPL license. 
  * @author John Burkardt  */
template<std::uint8_t N = 100>
constexpr double nlog(double x) {
  requires_msg(x > 0, "invalid x (" << x << ")");

  constexpr std::size_t A_LENGTH = 25;
  double a[A_LENGTH] = {
    1.648721270700128, 
    1.284025416687742, 
    1.133148453066826, 
    1.064494458917859, 
    1.031743407499103, 
    1.015747708586686, 
    1.007843097206488, 
    1.003913889338348, 
    1.001955033591003, 
    1.000977039492417, 
    1.000488400478694, 
    1.000244170429748, 
    1.000122077763384, 
    1.000061037018933, 
    1.000030518043791, 
    1.0000152589054785, 
    1.0000076294236351, 
    1.0000038147045416, 
    1.0000019073504518, 
    1.0000009536747712, 
    1.0000004768372719, 
    1.0000002384186075, 
    1.0000001192092967, 
    1.0000000596046466, 
    1.0000000298023228 };

  constexpr double e = 2.718281828459045;
  double fx = 0.0;

  std::int_fast32_t k = 0;
  while ( e <= x ) {
    k++;
    x = x / e; }

  while ( x < 1.0 ) {
    k--;
    x = x * e;  }
  
  double ai = 0.0;
  double w[N] { };
  for (std::size_t i = 0; i < N; i++ ) {
    w[i] = 0.0;
    if ( i < A_LENGTH ) ai = a[i];
    else ai = 1.0 + ( ai - 1.0 ) / 2.0;
    if ( ai < x ) {
      w[i] = 1.0;
      x = x / ai;  } }

  x = x - 1.0;
  x = x
    * ( 1.0 - ( x / 2.0 ) 
    * ( 1.0 + ( x / 3.0 ) 
    * ( 1.0 -   x / 4.0 )));
  
  double poweroftwo = 0.5;
  for (std::size_t i = 0; i < N; i++ ) {
    x = x + w[i] * poweroftwo;
    poweroftwo = poweroftwo / 2.0; }

  fx = x + ( double ) ( k );

  return fx;
}

} // namespace cmn::math

#endif // CMN__MATH__LOG__HPP
