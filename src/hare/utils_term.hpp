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

#ifndef HA__UTILS_TERM__HPP
#define HA__UTILS_TERM__HPP

#include "hare/term.hpp"

#include "cmn/dbc.hpp"
#include "cmn/math/continuous_dynamic_interval.hpp"
#include "cmn/math/extended_number.hpp"

#include <gmpxx.h>

#include <boost/numeric/interval.hpp>
#include <boost/numeric/interval/transc.hpp>

#include <cmath>  // isnan

namespace hare {

using constant2  = ::cmn::math::extended_number<constant>;
using interval_t = ::cmn::math::continuous_dynamic_interval<constant>;

template<typename D = float>
using boost_interval_t = boost::numeric::interval<D, boost::numeric::interval_lib::policies<
                                                       boost::numeric::interval_lib::save_state<
                                                         boost::numeric::interval_lib::rounded_transc_std<D> >, 
                                                       boost::numeric::interval_lib::checking_base<D> > >;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<bool binary = false>
bool check_arity(const term_ptr& t) {
  const auto arity = t->arity();
  switch(t->symbol()) {
  case symbol::PI    :
  case symbol::E     :
  case symbol::TRUE  :
  case symbol::FALSE :
  case symbol::CONST :
  case symbol::VAR   : requires_msg(arity == 0, "arity " << arity << " is not supported for the input term " << t)
                       return arity == 0;
  case symbol::NOT   :
  case symbol::NEG   :
  case symbol::ABS   :
  case symbol::SQRT  :
  case symbol::SQR   :
  case symbol::CUBE  :
  case symbol::LOG   :
  case symbol::LN    :
  case symbol::LG    :
  case symbol::SIN   :
  case symbol::COS   :
  case symbol::TAN   :
  case symbol::COT   : requires_msg(arity == 1, "arity " << arity << " is not supported for the input term " << t)
                       return arity == 1;
  case symbol::LT    :
  case symbol::LE    :
  case symbol::GT    :
  case symbol::GE    :
  case symbol::EQ    :
  case symbol::NQ    :
  case symbol::IFF   : requires_msg(arity == 2, "arity " << arity << " is not supported for the input term " << t)
                       return arity == 2;
  case symbol::IMPLY :
  case symbol::AND   :
  case symbol::OR    :
  case symbol::PLUS  :
  case symbol::MINUS :
  case symbol::MULT  :
  case symbol::DIV   :
  case symbol::EXP   : requires_msg(!binary || arity == 2, "arity " << arity << " (binary=" << binary << ") is not supported for the input term " << t)
                       return !binary || arity == 2;
  }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

inline constant evaluate(const term_ptr& t) { 
  requires(check_arity<false>(t))
  requires_msg(is_ground_term(t), "input term is not ground " << t)
  const auto smbl = t->symbol();
  if(smbl == symbol::CONST) return t->value();
  std::vector<mpq_class> ops;
  for(const auto op : t->operands()) ops.push_back(evaluate(op));
  switch(t->symbol()) {
    case symbol::PI    : { static bool flag = false;
                           if(!flag) { flag = true; log_warn << "constant pi is replaced by 3.14"; }
                           auto res = mpq_class(314,100);
                           res.canonicalize(); 
                           return res; }
    case symbol::E     : { static bool flag = false;
                           if(!flag) { flag = true; log_warn << "constant e is replaced by 2.71"; }
                           return  mpq_class(271,100); }
    case symbol::ABS   : return  ops[0] < 0 ? -ops[0] : ops[0];
    case symbol::SQR   : return  ops[0] * ops[0];
    case symbol::CUBE  : return  ops[0] * ops[0] * ops[0];
    case symbol::NEG   : return -ops[0];
    case symbol::MULT  : return  std::accumulate(ops.begin(), ops.end(), mpq_class(1), std::multiplies<mpq_class>());
    case symbol::PLUS  : return  std::accumulate(ops.begin(), ops.end(), mpq_class(0), std::plus      <mpq_class>());
    case symbol::MINUS : if(ops.size() == 0) return 0;
                         return std::accumulate(ops.begin()+1, ops.end(), ops[0], std::minus<mpq_class>());
    case symbol::DIV   : if(ops.size() == 0) return 1;
                         return std::accumulate(ops.begin()+1, ops.end(), ops[0], [&t](const auto& fst, const auto& snd){ 
                             requires_msg(snd != 0, "division by zero! term = " << t)
                             return fst / snd; });
    default            : requires_msg(false, "evaluation of the input term is not supported. t = " << t)
                         throw std::invalid_argument("symbol did not match"); }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** @note denominators must be ground terms 
  * @note only logical, comparison, addition, multiplication, division, subtraction, and negation operators are supported 
  * @note it is guaranteed that if the input term is multiplied by the returned number, all constant in the term will be integer, but at the time of writing 
  *       this comment, it is not guaranteed that the returned number is the smallest such number (this may change in future). 
  * @note it is guaranteed that the return number is positive */
inline mpz_class lcm_of_denominators(const term_ptr& t) { 
  using result_t = mpz_class;
  switch(t->symbol()) {
    case symbol::VAR   : 
    case symbol::TRUE  :
    case symbol::FALSE : return 1;
    case symbol::CONST :
    case symbol::PI    :
    case symbol::E     : return evaluate(t).get_den();
    case symbol::MULT  : { result_t res = 1;
                           for(const auto op : t->operands()) 
                             res *= lcm_of_denominators(op); 
                           return res; }
    case symbol::DIV   : { const auto& ops  = t->operands();
                                 auto  iter = ops.begin  ();
                           const auto  end  = ops.end    ();
                           if(iter == end) return 1;
                           auto r1 = lcm_of_denominators(ops[0]);
                           constant den = 1;
                           while(++iter != end) den /= evaluate(*iter);
                           r1 *= den.get_den();
                           return r1; }
    case symbol::NEG   :
    case symbol::PLUS  :
    case symbol::MINUS :
    case symbol::LT    :
    case symbol::LE    :
    case symbol::GT    :
    case symbol::GE    :
    case symbol::EQ    :
    case symbol::NQ    :
    case symbol::NOT   :
    case symbol::AND   :
    case symbol::OR    :
    case symbol::IMPLY :
    case symbol::IFF   : { result_t res = 1;
                           for(const auto& op : t->operands())
                             res = lcm(res, lcm_of_denominators(op));
                           return res; } 
    default            : requires_msg(false, "input term is not supported: " << t)
                         throw std::invalid_argument("input term is not supported");
  }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

inline term_ptr negate_variables(const term_ptr& t) {
  const auto smbl = t->symbol();
  switch(smbl) {
  case symbol::VAR   : return -term_ptr(term::variable(t->variable()));
  case symbol::CONST : return  term_ptr(term::constant(t->value   ()));
  default :
    typename term::operands_type ops;
    ops.reserve(t->arity());
    for(const auto& op : t->operands())
      ops.push_back(negate_variables(op));
    return term_ptr(new term(smbl, std::move(ops))); }
}  

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

inline bool is_linear(const term_ptr& t) {
  requires(check_arity(t))
  switch(t->symbol()) {
  case symbol::SQRT  :
  case symbol::LOG   :
  case symbol::LN    :
  case symbol::LG    :
  case symbol::SIN   :
  case symbol::COS   :
  case symbol::TAN   :
  case symbol::COT   : 
                       return false;
  case symbol::PI    :
  case symbol::E     :
  case symbol::TRUE  :
  case symbol::FALSE :
  case symbol::CONST :
  case symbol::VAR   : 
                       return true;                               // can be removed without changing the functionality
  case symbol::ABS   :
  case symbol::SQR   :
  case symbol::CUBE  :
                       return is_ground_term(t->operands()[0]);   // can be removed without changing the functionality
  case symbol::LT    :
  case symbol::LE    :
  case symbol::GT    :
  case symbol::GE    :
  case symbol::EQ    :
  case symbol::NQ    :
  case symbol::AND   :
  case symbol::OR    :
  case symbol::IMPLY :
  case symbol::IFF   : 
  case symbol::NOT   :
  case symbol::PLUS  :
  case symbol::MINUS :
  case symbol::NEG   :
                       return std::all_of(t->operands().begin(), t->operands().end(), [](const auto& op) { return is_linear(op); });
  case symbol::EXP   : 
                       return std::all_of(t->operands().begin(), t->operands().end(), [](const auto& op) { return is_ground_term(op); });
  case symbol::MULT  : {
                       const auto begin = t->operands().begin();
                       const auto end   = t->operands().end  ();
                       if(std::any_of(begin, end, [](const auto& op) { return !is_linear(op); })) return false;
                       bool seen_non_ground = false;
                       for(auto iter = begin; iter != end; iter++)
                         if(!is_ground_term(*iter)) {
                           if(seen_non_ground) return            false;
                           else                seen_non_ground = true ; }
                       return true; }
  case symbol::DIV   : {
                       auto iter = t->operands().begin();
                       auto end  = t->operands().end  ();
                       if(iter == end)       return true ;
                       if(!is_linear(*iter)) return false;
                       return std::all_of(++iter, end, [](const auto& op) { return is_ground_term(op); }); }
  }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename D>
auto to_cmn_math_interval(const boost_interval_t<D>& that) { 
  using limits = std::numeric_limits<D>;
  auto min = that.lower(); if(std::isnan(min)) min = -limits::infinity();
  auto max = that.upper(); if(std::isnan(max)) max =  limits::infinity();
  if(min < limits::lowest() && limits::max() < max) return interval_t::universal();
  if(min < limits::lowest()) return interval_t::le(val(std::to_string(max))->value());
  if(max > limits::max   ()) return interval_t::ge(val(std::to_string(min))->value());
  return interval_t(val(std::to_string(min))->value(), val(std::to_string(max))->value()); }

template<typename D>
auto to_boost_interval(const interval_t& that) {
  using limits = std::numeric_limits<D>;
  D inf = that.is_left_bounded () ? static_cast<D>(that.inf().value().get_d()) : -limits::infinity();
  D sup = that.is_right_bounded() ? static_cast<D>(that.sup().value().get_d()) :  limits::infinity();
       if(inf >  limits::lowest()) inf =  std::nextafter(inf, limits::lowest()); 
  else if(inf == limits::lowest()) inf = -limits::infinity();
       if(sup <  limits::max   ()) sup =  std::nextafter(sup, limits::max()); 
  else if(sup == limits::max   ()) sup =  limits::infinity();
  return boost_interval_t<D>(inf, sup); } 

template<typename D>
boost_interval_t<D> bound_of_expr(const std::unordered_map<term_ptr, boost_interval_t<D>>& var_bounds, const term_ptr& t) {
  using interval = boost_interval_t<D>;
  namespace numeric = boost::numeric;
  requires(check_arity<false>(t))

  if(t->is_variable()) {
    const auto iter = var_bounds.find(t);
    requires_msg(iter != var_bounds.end(), "no bound for variable " << t << " is defined")
    return iter->second; }
  if(t->is_constant())
    return to_boost_interval<D>(interval_t(evaluate(t)));

  const auto& children = t->operands();
  std::vector<interval> ops;
  ops.reserve(children.size());
  for(const auto& child : children)
    ops.push_back(bound_of_expr<D>(var_bounds, child));

  switch(t->symbol()) {
    case symbol::PI    : return numeric::interval_lib::pi<interval>();
    case symbol::E     : return numeric::exp(interval(1));
    case symbol::PLUS  : return ops.empty() ? interval(0) : std::accumulate(ops.begin() + 1, ops.end(), ops[0], std::plus      <interval>());
    case symbol::MINUS : return ops.empty() ? interval(0) : std::accumulate(ops.begin() + 1, ops.end(), ops[0], std::minus     <interval>());
    case symbol::MULT  : return ops.empty() ? interval(1) : std::accumulate(ops.begin() + 1, ops.end(), ops[0], std::multiplies<interval>());
    case symbol::DIV   : return ops.empty() ? interval(1) : std::accumulate(ops.begin() + 1, ops.end(), ops[0], std::divides   <interval>());
    case symbol::NEG   : return -std::move(ops[0]);
    case symbol::SIN   : return numeric::sin(std::move(ops[0]));
    case symbol::COS   : return numeric::cos(std::move(ops[0]));
    case symbol::TAN   : return numeric::tan(std::move(ops[0]));
    case symbol::COT   : return numeric::cos(std::move(ops[0])) / numeric::sin(std::move(ops[0])); //< the library does not provide a direct implementation
    case symbol::EXP   : return std::accumulate(ops.rbegin(), ops.rend(), interval(1), 
                                                [](auto& exp, auto& base) { return numeric::exp(std::move(exp)*numeric::log(std::move(base))); } ); 
    case symbol::ABS   : return numeric::abs   (std::move(ops[0]));
    case symbol::SQRT  : return numeric::sqrt  (std::move(ops[0]));
    case symbol::SQR   : return numeric::square(std::move(ops[0]));
    case symbol::CUBE  : return numeric::pow   (std::move(ops[0]), 3);
    case symbol::LOG   : return numeric::log(std::move(ops[0])) / numeric::log(interval(10));
    case symbol::LN    : return numeric::log(std::move(ops[0])); 
    case symbol::LG    : return numeric::log(std::move(ops[0])) / numeric::log(interval(2));

    case symbol::CONST : 
    case symbol::VAR   : 
    case symbol::LT    : 
    case symbol::LE    : 
    case symbol::GT    : 
    case symbol::GE    : 
    case symbol::EQ    : 
    case symbol::NQ    :
    case symbol::TRUE  : 
    case symbol::FALSE : 
    case symbol::AND   :
    case symbol::OR    :
    case symbol::NOT   :
    case symbol::IMPLY : 
    case symbol::IFF   : requires_msg(false, "input term is not arithmetic " << t)
                         throw std::invalid_argument("input term is not arithmetic");  }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

} // namespace hare

#endif // HA__UTILS_TERM__HPP
