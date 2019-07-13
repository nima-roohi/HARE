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

#ifndef HA__UTILS_PPL__HPP
#define HA__UTILS_PPL__HPP

#include "ha/term.hpp"
#include "ha/utils_term.hpp"

#include "cmn/dbc.hpp"

#include <gmpxx.h>
#include <ppl.hh>

#include <algorithm>
#include <exception>
#include <functional>
#include <numeric>      // accumulate
#include <type_traits>
#include <vector>

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace ha::ppl_utils {

namespace ppl = Parma_Polyhedra_Library;

using NNCPoly = ppl::NNC_Polyhedron;
using CPoly   = ppl::C_Polyhedron  ;
using BDShape = ppl::BD_Shape<mpq_class>;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

inline ppl::Linear_Expression expr_of(const ppl::Constraint& cs) {
  ppl::Linear_Expression res;
  for(ppl::dimension_type i = cs.space_dimension(); i-- > 0;) {
    const auto var = ppl::Variable(i);
    res += cs.coefficient(var) * var; }
  return res; }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename P>
bool is_closed_poly(const P& poly) { return poly.is_topologically_closed(); }

template<typename P>
bool is_open_poly(const P& poly) {
  return poly.is_universe() || poly.is_empty() ||
         std::all_of(poly.constraints().begin(), poly.constraints().end(), [](const auto& cs) {
           return cs.type() == ppl::Constraint::Type::STRICT_INEQUALITY; }); }

template<typename P>
CPoly closed_poly(const P& poly) {
  ppl::Constraint_System css(poly.constraints().representation());
  css.set_space_dimension(poly.space_dimension());
  // use minimiezd_constraints instead of constraints so if the input is (x<0 && x>0) 
  // it will be removed by the minimization and we return a correct answer.
  for(const auto& cs : poly.minimized_constraints())
    cs.is_strict_inequality() ? css.insert(expr_of(cs) + cs.inhomogeneous_term() >= 0)
                              : css.insert(cs) ; 
  return CPoly(std::move(css)); }

template<typename P>
NNCPoly open_poly(const P& poly) {
  ppl::Constraint_System css(poly.constraints().representation());
  css.set_space_dimension(poly.space_dimension());
  if(poly.constraints().has_equalities()) {
    css.insert(ppl::Constraint::zero_dim_false());
    return NNCPoly(std::move(css)); }
  for(const auto& cs : poly.constraints())
    cs.is_nonstrict_inequality() ? css.insert(expr_of(cs) + cs.inhomogeneous_term() > 0)
                                 : css.insert(cs) ; 
  return NNCPoly(std::move(css)); }

// template<> bool  is_closed_poly(const CPoly& /* poly */) { return true; }
// template<> CPoly closed_poly   (const CPoly&    poly   ) { return poly; }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** @brief a \c ppl variable mapper to switch the first half of coordinates with the second half */
struct shift_dimensions {
  using D = ppl::dimension_type;
  /** @param size dimension of each point */
  explicit shift_dimensions(cmn::pbv_t<D> size) : size_(size) { 
    requires_msg( size      >= 0, "Size (" << size << ") must be non_negative") 
    requires_msg((size & 1) == 0, "Size (" << size << ") must be an even number") }

  bool has_empty_codomain()  const {                     return size_ <= 0; }
  D    max_in_codomain   ()  const { requires(size_ > 0) return size_ - 1 ; }
  bool maps(const D i, D& j) const {
    j = i < size_/2 ? i + size_/2 : i - size_/2;
    return true; }
private:
  const D size_; };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

struct ppl_var_hash {
  using argument_type = ppl::Variable;
  using result_type   = std::size_t;
  result_type operator()(const argument_type& that) const { return static_cast<result_type>(that.id()); } };

struct ppl_var_equal {
  using result_type          = bool;
  using first_argument_type  = ppl::Variable;
  using second_argument_type = ppl::Variable;
  result_type operator()(const first_argument_type& fst, const second_argument_type& snd) const { return fst.id() == snd.id(); } };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename Out>
Out& print_var_name(Out& out, const ppl::dimension_type id, const id2var& i2v) {
  const auto iter = i2v.find(id);
  if(iter == i2v.end()) {
    const auto size = i2v.size();
    if(id == 0 || size == 0 || id < size) out << "(x" << id << ")";
    else                                  print_var_name(out, id - size, i2v) << "'"; 
  } else out << iter->second;
  return out; }

template<typename Out>
Out& print_constraint(Out& out, const ppl::Constraint& cs, const id2var& i2v, const char* eq_smbl, const char* mult_smbl) {
  const auto dim = cs.space_dimension();
  bool first = true;
  bool nothing_printed = true;
  for(ppl::dimension_type i = 0; i < dim; i++) {
    const auto var  = ppl::Variable ( i );
    const auto coef = cs.coefficient(var);
    if(coef == 0) continue;
    nothing_printed = false;
    if(first) {
           if(coef == -1) out << "-";
      else if(coef !=  1) out << coef << mult_smbl; }
    else if(coef ==  1) out << " + ";
    else if(coef == -1) out << " - ";
    else if(coef >   0) out << " + " <<  coef << mult_smbl;
    else                out << " - " << -coef << mult_smbl; 
    print_var_name(out, var.id(), i2v);
    first = false; }
  if(nothing_printed) out << "0";
  switch(cs.type()) {
  case ppl::Constraint::Type::EQUALITY            : out << " " << eq_smbl << " "; break;
  case ppl::Constraint::Type::STRICT_INEQUALITY   : out << " >  "; break;
  case ppl::Constraint::Type::NONSTRICT_INEQUALITY: out << " >= "; break; }
  out << -cs.inhomogeneous_term(); 
  return out;  }

template<typename Out, typename P>
Out& print_polyhedron(Out& out, const P& poly, const id2var& i2v, const char* eq_smbl, const char* mult_smbl, const char* sep) {
  namespace utils = cmn::misc::utils;
  bool first = true;
  for(const auto& cs : poly.constraints()) {
    if(first) first = false; 
    else      out << sep;
    print_constraint(out, cs, i2v, eq_smbl, mult_smbl); }
  return out;  }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

inline ppl::Constraint negate_variables(const ppl::Constraint& cs) {
  const auto expr = expr_of(cs);
  const auto coef = cs.inhomogeneous_term();
  switch(cs.type()) {
  case ppl::Constraint::Type::EQUALITY            : return std::move(expr) == std::move(coef);
  case ppl::Constraint::Type::STRICT_INEQUALITY   : return std::move(expr) <  std::move(coef);
  case ppl::Constraint::Type::NONSTRICT_INEQUALITY: return std::move(expr) <= std::move(coef);  }
}

template<typename P>
P negate_variables(const P& poly) {
  ppl::Constraint_System css(poly.constraints().representation());
  css.set_space_dimension(poly.space_dimension());
  for(const auto& cs : poly.constraints())
    css.insert(negate_variables(cs));
  return P(std::move(css)); }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

inline ppl::Linear_Expression to_linear_expr(const term_ptr& t, const mpz_class& coef) {
  requires(check_arity<true>(t))
  const auto  smbl  = t->symbol();
  if(smbl == symbol::CONST || smbl == symbol::PI || smbl == symbol::E || is_ground_term(t)) {
    mpq_class res = coef * evaluate(t);
    asserts_msg(res.get_den() == 1, "t=" << t << ", coef=" << coef << ", res=" << res)
    return ppl::Linear_Expression(std::move(res).get_num()); }
  if(smbl == symbol::VAR  ) return coef * ppl::Linear_Expression(ppl::Variable(t->variable().id()));
  const auto& ops = t->operands();
  if(ops.empty()) return ppl::Linear_Expression(mpz_class(0)); // no coef is necessary
  switch(smbl) {
    case symbol::ABS   :
    case symbol::SQR   :
    case symbol::CUBE  : requires_msg(is_ground_term(t     ),                           "input term is non-linear: " << t) break;
    case symbol::DIV   : requires_msg(is_ground_term(ops[1]),                           "input term is non-linear: " << t) break;
    case symbol::MULT  : requires_msg(is_ground_term(ops[0]) || is_ground_term(ops[1]), "input term is non-linear: " << t) break;
    default            : ;/* no op */ }
  switch(smbl) {
    case symbol::NEG   : return -to_linear_expr(ops[0], coef);
    case symbol::PLUS  : { auto res = to_linear_expr(ops[0], coef);
                           std::for_each(ops.begin()+1, ops.end(), [&res, &coef](const auto& op) { res += to_linear_expr(op, coef); });
                           return res; }
    case symbol::MINUS : { auto res = to_linear_expr(ops[0], coef);
                           std::for_each(ops.begin()+1, ops.end(), [&res, &coef](const auto& op) { res -= to_linear_expr(op, coef); });
                           return res; }
    case symbol::MULT  : if(is_ground_term(ops[0])) {
                           mpq_class fst = evaluate(ops[0]);
                           mpz_class new_coef = static_cast<mpq_class>(coef / fst.get_den()).get_num();
                           return std::move(fst).get_num() * to_linear_expr(ops[1], std::move(new_coef)); }
                         return to_linear_expr(ops[1] * ops[0], coef);
    case symbol::DIV   : { mpq_class den = evaluate(ops[1]);
                           requires_msg(den != 0, "division by zero! term=" << t)
                           mpq_class inversed = 1 / std::move(den);
                           return to_linear_expr(val(std::move(inversed)) * ops[0], coef); }
    case symbol::ABS   :
    case symbol::SQR   :
    case symbol::CUBE  : { mpq_class res = evaluate(t) * coef;
                           asserts_msg(res.get_den() == 1, "error in code, term = " << t << "res = " << res)
                           return ppl::Linear_Expression(std::move(res).get_num()); }
    default            : requires_msg(false, "operator " << t->symbol() << "is not supported: term = " << t)
                         throw std::invalid_argument("symbol did not match");   }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

inline ppl::Constraint to_constraint(const term_ptr& t) { 
  log_trace << "t: " << t;
  if(t == FALSE) return ppl::Constraint::zero_dim_false     ();
  if(t == TRUE ) return ppl::Constraint::zero_dim_positivity();
  mpz_class lcm = lcm_of_denominators(t);
  log_trace << "LCM: " << lcm;
  requires_msg(t->arity() == 2, "arity (" << t->arity() << ") of the input term is not 2. t = " << t)
  const auto& ops = t->operands();
  switch(t->symbol()) {
    case symbol::LT : return to_linear_expr(ops[0], lcm) <  to_linear_expr(ops[1], lcm); 
    case symbol::LE : return to_linear_expr(ops[0], lcm) <= to_linear_expr(ops[1], lcm); 
    case symbol::GT : return to_linear_expr(ops[0], lcm) >  to_linear_expr(ops[1], lcm); 
    case symbol::GE : return to_linear_expr(ops[0], lcm) >= to_linear_expr(ops[1], lcm); 
    case symbol::EQ : return to_linear_expr(ops[0], lcm) == to_linear_expr(ops[1], lcm); 
    default: requires_msg(false, "invalid operator (" << t->symbol() << ") t = " << t)
  }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

inline void to_constraint_system(const term_ptr& t, ppl::Constraint_System& res) {
  switch(t->symbol()) {
    case symbol::AND   : for(const auto& op : t->operands()) to_constraint_system(op, res); return;
    case symbol::TRUE  :
    case symbol::FALSE :
    case symbol::LT    :
    case symbol::LE    :
    case symbol::GT    :
    case symbol::GE    :
    case symbol::EQ    : res.insert(to_constraint(t)); return;
    default            : requires_msg(false, "invalid operator (" << t->symbol() << ") t = " << t) }
}

inline auto to_constraint_system(const term_ptr& t, const ppl::Representation representation = ppl::Representation::DENSE) {
  ppl::Constraint_System res(representation);
  to_constraint_system(t, res);
  return res; }

template<typename P>
P to_polyhedron(const term_ptr& t, const ppl::dimension_type dim, const ppl::Representation representation = ppl::Representation::DENSE) {
  auto css = to_constraint_system(t, representation);
  css.set_space_dimension(dim);
  return P(css); }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

inline term_ptr to_term(const ppl::Constraint& cs, const id2var& i2v) { 
  const auto dim = cs.space_dimension();
  std::vector<term_ptr> ops;
  for(std::remove_const_t<decltype(dim)> i = 0; i < dim; i++) {
    const auto var  = i2v.find(i);
    requires_msg(var != i2v.end(), "variable id " << i << " is not defined");
    const auto coef = cs.coefficient(ppl::Variable(i));
    if(coef == 0) continue; 
    if(coef == 1) ops.push_back(var->second);
    else          ops.push_back(var->second * val(coef)); }
  ops.push_back(val(cs.inhomogeneous_term()));
  auto expr = ops.size() == 1 ? ops[0] : term_ptr(new term(symbol::PLUS, ops));
  switch(cs.type()) {
  case ppl::Constraint::Type::EQUALITY            : return eq(expr, val(0)); break;
  case ppl::Constraint::Type::STRICT_INEQUALITY   : return expr >   val(0) ; break;
  case ppl::Constraint::Type::NONSTRICT_INEQUALITY: return expr >=  val(0) ; break; } 
}

inline term_ptr to_term(const ppl::Constraint_System& css, const id2var& i2v) { 
  std::vector<term_ptr> ops;
  for(const auto cs : css) ops.push_back(to_term(cs, i2v));
  return term_ptr(new term(symbol::AND, std::move(ops))); }

inline term_ptr to_term(const CPoly   poly, const id2var& i2v) { return to_term(poly.constraints(), i2v); }
inline term_ptr to_term(const NNCPoly poly, const id2var& i2v) { return to_term(poly.constraints(), i2v); }
inline term_ptr to_term(const BDShape poly, const id2var& i2v) { return to_term(poly.constraints(), i2v); }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename P>
auto bound_of_expr(const ppl::Linear_Expression& expr, const P& poly) {
  mpz_class num1, num2, den1, den2;
  bool has_min, has_max;
  const bool has_inf = poly.minimize(expr, num1, den1, has_min);
  const bool has_sup = poly.maximize(expr, num2, den2, has_max);
  const auto inf = has_inf ? constant2(constant(num1, den1)) : constant2::NEG_INF;
  const auto sup = has_sup ? constant2(constant(num2, den2)) : constant2::POS_INF;
  const auto lkind = has_inf && has_min ? ::cmn::math::interval_bound_kind::CLOSED
                                        : ::cmn::math::interval_bound_kind::OPEN ;
  const auto rkind = has_sup && has_max ? ::cmn::math::interval_bound_kind::CLOSED
                                        : ::cmn::math::interval_bound_kind::OPEN ;
  return interval_t(lkind, inf, sup, rkind); }

template<typename Iter>
auto bound_of_expr(const ppl::Linear_Expression& expr, Iter iter, const Iter end) {
  if(iter == end) return interval_t::universal();
  auto res = bound_of_expr(expr, *iter);
  while(++iter != end)
    res = res.chull(bound_of_expr(expr, *iter)); }

template<typename P> auto bound_of_var(const ppl::Variable&      var, const P& poly) { return bound_of_expr(var                 , poly); }
template<typename P> auto bound_of_var(const ppl::dimension_type id , const P& poly) { return bound_of_var (ppl::Variable(id)   , poly); }
template<typename P> auto bound_of_var(const term_ptr&           var, const P& poly) { return bound_of_var (var->variable().id(), poly); }

template<typename Iter> auto bound_of_var(const ppl::Variable&      var, Iter iter, const Iter end) { return bound_of_expr(var                , iter, end); }
template<typename Iter> auto bound_of_var(const ppl::dimension_type id , Iter iter, const Iter end) { return bound_of_var(ppl::Variable(id)   , iter, end); }
template<typename Iter> auto bound_of_var(const term_ptr&           var, Iter iter, const Iter end) { return bound_of_var(var->variable().id(), iter, end); }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** Another unfortunate/unknown behavior in Linux. Using "remove_space_dimensions" directly on Polyhedron gives me undefined behavior. */
template<typename P, typename Set>
void remove_space_dimensions(P& poly, const Set& vars) {
#ifdef __linux__
  using D = ppl::dimension_type;
  const D dim = poly.space_dimension();
  std::unordered_map<D, D> map { dim };
  for(D i = 0; i < dim; i++)
    if(vars.find(i) == vars.end()) // i.e. dimension i should not be removed
      map.insert(std::pair<D,D>(i, map.size()));
  struct mapper {
    mapper(const std::unordered_map<D,D>& map) : map(map) { }
    bool has_empty_codomain()  const { return map.empty();     }
    D    max_in_codomain   ()  const { return map.size () - 1; }
    bool maps(const D i, D& j) const {
      const auto it = map.find(i);
      if(it != map.end()) {
        j = it->second; 
        return true;
      } else return false; } 
    const std::unordered_map<D,D>& map;
  };
  poly.map_space_dimensions(mapper(map));
#else
  poly.remove_space_dimensions(vars);
#endif
}


//-------------------------------------------------------------------------------------------------------------------------------------------------------------

} // namespace ha::ppl_utils

#endif // HA__UTILS_PPL__HPP
