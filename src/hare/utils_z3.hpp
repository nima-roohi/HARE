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

#ifndef HA__UTILS_Z3__HPP
#define HA__UTILS_Z3__HPP

#include "ha/performance_counter.hpp"
#include "ha/term.hpp"
#include "ha/utils_term.hpp"

#include "cmn/dbc.hpp"

#include <gmpxx.h>
#include <z3.h>
#include <z3++.h>

#include <cctype>      // isspace
#include <exception>
#include <functional>
#include <ios>
#include <numeric>
#include <sstream>
#include <type_traits>
#include <utility>
#include <vector>

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace z3 {

template<typename Out, std::enable_if_t<!std::is_base_of<std::ios, Out>::value>* = nullptr>
inline Out& operator<<(Out& os, const expr& e) { 
  std::stringstream buff;
  buff << e;
  os << buff.str();
  return os; }

template<typename Out, std::enable_if_t<!std::is_base_of<std::ios, Out>::value>* = nullptr>
inline Out& operator<<(Out& os, const solver& s) { 
  std::stringstream buff;
  buff << s;
  os << buff.str();
  return os; }

} // namespace z3

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace ha::z3_utils {

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

inline z3::expr to_arith_expr(const term_ptr t, z3::context& ctx) {
  requires(check_arity<false>(t))
  const auto accumulate_l2r = [](const auto& ops, const auto& id, z3::context& ctx, const auto& op) {
    if(ops.empty()) return to_arith_expr(id, ctx);
    z3::expr res = to_arith_expr(ops[0], ctx);
    const auto end = ops.end();
    for(auto iter = ops.begin() + 1; iter != end; iter++)
      res = op(std::move(res), to_arith_expr(*iter, ctx)); 
  return res; };
  const auto accumulate_r2l = [](const auto& ops, const auto& id, z3::context& ctx, const auto& op) {
    if(ops.empty()) return to_arith_expr(id, ctx);
    z3::expr res = to_arith_expr(ops[ops.size()-1], ctx);
    const auto end = ops.rend();
    for(auto iter = ops.rbegin() + 1; iter != end; iter++)
      res = op(to_arith_expr(*iter, ctx), std::move(res)); 
  return res; };
  switch(t->symbol()) { 
    case symbol::PI    : { static bool flag = false; 
                           if(!flag) { flag = true; log_warn << "constant pi is replaced by 3.14 in Z3"; }
                           return ctx.real_val(314, 100); }
    case symbol::E     : { static bool flag = false; 
                           if(!flag) { flag = true; log_warn << "constant e is replaced by 2.71 in Z3"; }
                           return ctx.real_val(271, 100); }
    case symbol::CONST : { std::stringstream buff; buff << t; return ctx.real_val(buff.str().c_str()); }
    case symbol::VAR   : return  ctx.real_const(t->variable().name().c_str());
    case symbol::NEG   : return -to_arith_expr(t->operator[](0), ctx);
    case symbol::PLUS  : return accumulate_l2r(t->operands(), val(0), ctx, std::plus      <z3::expr>());
    case symbol::MINUS : return accumulate_l2r(t->operands(), val(0), ctx, std::minus     <z3::expr>());
    case symbol::MULT  : return accumulate_l2r(t->operands(), val(1), ctx, std::multiplies<z3::expr>());
    case symbol::DIV   : return accumulate_l2r(t->operands(), val(1), ctx, std::divides   <z3::expr>());
    case symbol::EXP   : return accumulate_r2l(t->operands(), val(1), ctx, [&ctx](const auto& fst, const auto& snd) { return z3::pw(fst, snd); }); 
    case symbol::SQR   : { const auto op = to_arith_expr(t->operator[](0), ctx); return op * op;      }
    case symbol::CUBE  : { const auto op = to_arith_expr(t->operator[](0), ctx); return op * op * op; }
    case symbol::ABS   : 
    case symbol::SQRT  : 
    case symbol::LOG   : 
    case symbol::LN    : 
    case symbol::LG    : 
    case symbol::SIN   : 
    case symbol::COS   : 
    case symbol::TAN   : 
    case symbol::COT   : requires_msg(false, "function " << t->symbol() << " is not supported yet! t = " << t)
    default            : requires_msg(false, "function " << t->symbol() << " is not arithmetic, t = " << t)
                         throw std::invalid_argument("input term is not arithmetic");
  }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

inline z3::expr to_bool_expr(const term_ptr t, z3::context& ctx) { 
  requires(check_arity<false>(t))
  const auto accumulate_l2r = [](const auto& ops, const auto& id, z3::context& ctx, const auto& op) {
    if(ops.empty()) return to_bool_expr(id, ctx);
    z3::expr res = to_bool_expr(ops[0], ctx);
    const auto end = ops.end();
    for(auto iter = ops.begin() + 1; iter != end; iter++)
      res = op(std::move(res), to_bool_expr(*iter, ctx)); 
  return res; };
  const auto accumulate_r2l = [](const auto& ops, const auto& id, z3::context& ctx, const auto& op) {
    if(ops.empty()) return to_bool_expr(id, ctx);
    z3::expr res = to_bool_expr(ops[ops.size()-1], ctx);
    const auto end = ops.rend();
    for(auto iter = ops.rbegin() + 1; iter != end; iter++)
      res = op(to_bool_expr(*iter, ctx), std::move(res)); 
  return res; };
  switch(t->symbol()) { 
    case symbol::TRUE  : return ctx.bool_val(true );
    case symbol::FALSE : return ctx.bool_val(false);
    case symbol::VAR   : return ctx.bool_const(t->variable().name().c_str());
    case symbol::NOT   : return !to_bool_expr(t->operator[](0), ctx);
    case symbol::AND   : return accumulate_l2r(t->operands(), TRUE , ctx, [&ctx](const auto& fst, const auto& snd) { return  fst && snd; });
    case symbol::OR    : return accumulate_l2r(t->operands(), FALSE, ctx, [&ctx](const auto& fst, const auto& snd) { return  fst || snd; });
    case symbol::IMPLY : return accumulate_r2l(t->operands(), FALSE, ctx, [&ctx](const auto& fst, const auto& snd) { return !fst || snd; });
    case symbol::IFF   : { const auto op1 = to_bool_expr(t->operator[](0), ctx); 
                           const auto op2 = to_bool_expr(t->operator[](1), ctx); 
                           return (!op1 || op2) && (op1 || !op2); }
    case symbol::LT    : return to_arith_expr(t->operator[](0), ctx) <  to_arith_expr(t->operator[](1), ctx);
    case symbol::LE    : return to_arith_expr(t->operator[](0), ctx) <= to_arith_expr(t->operator[](1), ctx);
    case symbol::GT    : return to_arith_expr(t->operator[](0), ctx) >  to_arith_expr(t->operator[](1), ctx);
    case symbol::GE    : return to_arith_expr(t->operator[](0), ctx) >= to_arith_expr(t->operator[](1), ctx);
    case symbol::EQ    : return to_arith_expr(t->operator[](0), ctx) == to_arith_expr(t->operator[](1), ctx);
    case symbol::NQ    : return to_arith_expr(t->operator[](0), ctx) != to_arith_expr(t->operator[](1), ctx);
    default            : requires_msg(false, "function " << t->symbol() << " is not boolean, t = " << t)
                         throw std::invalid_argument("input term is not boolean");
  }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

inline mpq_class parse_ground_expr(const char* buff, 
                                         typename std::string::size_type from, 
                                         typename std::string::size_type to  ,
                                   const typename std::string::size_type len ) {
  while(from < to && std::isspace(buff[from])) from++;
  while(from < to && std::isspace(buff[to-1])) to--  ;
  requires_msg(from < to, "from=" << from << ", to=" << to << ", len=" << len << ", str=" << std::string(buff, buff + len));
  const char c = buff[from];
  if(std::isdigit(c))               return str2mpq_class(std::string(buff + from, buff + to)); 
  if(c == '-')                      return - parse_ground_expr(buff, from + 1, to    , len);
  if(c == '(' && buff[to-1] == ')') return   parse_ground_expr(buff, from + 1, to - 1, len);
  // assume there is a binary operation on the top
  from++;
  while(from < to && std::isspace(buff[from])) from++;
  typename std::string::size_type level = 0;
  auto mid = from;
  while(mid < to) {
    const char c = buff[mid];
    if(level == 0 && std::isspace(c)) break;
    if(c == '(') level++; else
    if(c == ')') level--; 
    mid++; }
  requires_msg(from  < mid, "from=" << from << ", to=" << to << ", len=" << len << ", mid=" << mid << ", str=" << std::string(buff, buff + len));
  requires_msg(mid+1 <  to, "from=" << from << ", to=" << to << ", len=" << len << ", mid=" << mid << ", str=" << std::string(buff, buff + len));
  auto fst = parse_ground_expr(buff, from , mid, len);
  auto snd = parse_ground_expr(buff, mid+1,  to, len);
  switch(c) {
    case '-' : return fst - snd;
    case '+' : return fst + snd;
    case '*' : return fst * snd;
    case '/' : return fst / snd;
    default :
      requires_msg(false, "invalid input: from=" << from << ", to=" << to << ", len=" << len << ", str=" << std::string(buff, buff + len));
  }
}

inline term_ptr to_term(const z3::expr& expr, const name2var& n2v) {
  static std::unordered_map<Z3_decl_kind, symbol> z3_smbl( { {Z3_OP_TRUE, symbol::TRUE}, {Z3_OP_FALSE   , symbol::FALSE}, {Z3_OP_UMINUS , symbol::NEG  }, 
                                                             {Z3_OP_ADD , symbol::PLUS}, {Z3_OP_SUB     , symbol::MINUS}, {Z3_OP_MUL    , symbol::MULT }, 
                                                             {Z3_OP_DIV , symbol::DIV }, {Z3_OP_POWER   , symbol::EXP  }, {Z3_OP_LT     , symbol::LT   }, 
                                                             {Z3_OP_LE  , symbol::LE  }, {Z3_OP_GT      , symbol::GT   }, {Z3_OP_GE     , symbol::GE   }, 
                                                             {Z3_OP_EQ  , symbol::EQ  }, {Z3_OP_DISTINCT, symbol::NQ   }, {Z3_OP_AND    , symbol::AND  }, 
                                                             {Z3_OP_OR  , symbol::OR  }, {Z3_OP_NOT     , symbol::NOT  }, {Z3_OP_IMPLIES, symbol::IMPLY}, 
                                                             {Z3_OP_IFF , symbol::IFF } } );
  if(expr.is_app()) {
    const z3::func_decl decl = expr.decl ();
    const auto          kind = decl.decl_kind();
    if(kind == Z3_OP_FPA_RM_NEAREST_TIES_TO_EVEN /* need this when compile in Ubuntu Z3 4.4.0 */ || kind == Z3_OP_UNINTERPRETED) {
      std::stringstream buff; 
      buff << expr; 
      const auto v = n2v.find(buff.str()); 
      requires_msg(v != n2v.end(), "undefined variable " << expr)
      return v->second; }
    const auto arity = decl.arity();
    if(arity == 0) {
      std::stringstream buff; 
      buff << expr; 
      auto str = buff.str();
      return str == "true"  ? TRUE  :
             str == "false" ? FALSE :
             val(parse_ground_expr(str.c_str(), 0, str.length(), str.length()));
    } else {
      std::vector<term_ptr> ops;
      ops.reserve(arity);
      for(std::remove_const_t<std::remove_reference_t<decltype(arity)>> i = 0; i < arity; i++)
        ops.push_back(to_term(expr.arg(i), n2v));
      const auto smbl = z3_smbl.find(kind);
      requires_msg(smbl != z3_smbl.end(), "unsupported expression! decl_kind=" << decl.decl_kind() << ", expr=" << expr)
      return term_ptr(new term(smbl->second, std::move(ops))); 
    }
  }
  requires_msg(false, "conversion of the input expression is not supported! expr = " << expr)
  throw std::invalid_argument("unsupported expression");
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

inline bool is_satisfiable(z3::solver& solver) {
  switch(solver.check()) {
  case z3::check_result::unsat   : return false;
  case z3::check_result::sat     : return true ;
  case z3::check_result::unknown : requires_msg(false, "could not decide satisfiability of the expression: " << solver)
                                   throw std::runtime_error("could not decide satisfiability"); } }

inline bool is_unsatisfiable(z3::solver& solver) { return !is_satisfiable(solver); }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

} // namespace ha::z3_utils

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif // HA__UTILS_Z3__HPP
