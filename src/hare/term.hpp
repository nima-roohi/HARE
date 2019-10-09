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

#ifndef HA__TERM__HPP
#define HA__TERM__HPP

#include "cmn/dbc.hpp"
#include "cmn/type_traits.hpp"
#include "cmn/misc/assoc.hpp"
#include "cmn/misc/op_type.hpp"
#include "cmn/misc/simple_shared_ptr.hpp"
#include "cmn/misc/utils.hpp"
#include "cmn/math/term/untyped_term.hpp"
#include "cmn/math/term/pretty_printer.hpp"

#include <gmpxx.h>
#include <ppl.hh>

#include <cstdlib>
#include <cstdint>
#include <stdexcept>
#include <functional>
#include <ios>
#include <limits>
#include <numeric>
#include <string>
#include <sstream>
#include <type_traits>
#include <utility>
#include <unordered_map>
#include <unordered_set>

namespace hare {

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

enum class symbol : std::uint8_t {
  CONST, VAR, 

  NEG,
  PLUS, MINUS, MULT, DIV, EXP,
  ABS, SQRT, SQR, CUBE,
  LOG, LN, LG,
  SIN, COS, TAN, COT,

  TRUE, FALSE,
  LT, LE, GT, GE, EQ, NQ,
  AND, OR, NOT, IMPLY, IFF,

  PI, E
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** @note other than \c CONST and \c VAR, the exact symbols are used in parsing. Therefore, we require the followings to be true for string of any symbol
  * other than \c CONST and \c VAR:
  *       1.  it should be non-empty
  *       1.  it should contain only english letters or only punchuations
  *       1.  if it has english letters it must be unique
  *       1.  text of unary  operators must be disjoint
  *       1.  text of binary operators must be disjoint */
template<typename Out>
inline Out& operator<<(Out& os, const symbol type) {
  switch(type) {
  case symbol::CONST : { os << "<val>"; return os; }
  case symbol::VAR   : { os << "<var>"; return os; }
  case symbol::PLUS  : { os << "+"    ; return os; }
  case symbol::MINUS : { os << "-"    ; return os; }
  case symbol::MULT  : { os << "*"    ; return os; }
  case symbol::DIV   : { os << "/"    ; return os; }
  case symbol::EXP   : { os << "^"    ; return os; }
  case symbol::NEG   : { os << "-"    ; return os; }
  case symbol::ABS   : { os << "abs"  ; return os; }
  case symbol::SQRT  : { os << "sqrt" ; return os; }
  case symbol::SQR   : { os << "sqr"  ; return os; }
  case symbol::CUBE  : { os << "cube" ; return os; }
  case symbol::LOG   : { os << "log"  ; return os; }
  case symbol::LN    : { os << "ln"   ; return os; }
  case symbol::LG    : { os << "lg"   ; return os; }
  case symbol::SIN   : { os << "sin"  ; return os; }
  case symbol::COS   : { os << "cos"  ; return os; }
  case symbol::TAN   : { os << "tan"  ; return os; }
  case symbol::COT   : { os << "cot"  ; return os; }
  case symbol::LT    : { os << "<"    ; return os; }
  case symbol::LE    : { os << "<="   ; return os; }
  case symbol::GT    : { os << ">"    ; return os; }
  case symbol::GE    : { os << ">="   ; return os; }
  case symbol::EQ    : { os << "=="   ; return os; }
  case symbol::NQ    : { os << "!="   ; return os; }
  case symbol::TRUE  : { os << "true" ; return os; }
  case symbol::FALSE : { os << "false"; return os; }
  case symbol::AND   : { os << "&&"   ; return os; }
  case symbol::OR    : { os << "||"   ; return os; }
  case symbol::NOT   : { os << "!"    ; return os; }
  case symbol::IMPLY : { os << "==>"  ; return os; }
  case symbol::IFF   : { os << "<==>" ; return os; }
  case symbol::PI    : { os << "pi"   ; return os; }
  case symbol::E     : { os << "e"    ; return os; }
  }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

using var_id   = std::uint8_t;
using var_name = std::string;
static_assert(sizeof(var_id) <= sizeof(Parma_Polyhedra_Library::dimension_type));

struct variable {
  using type   = variable;
  using id_t   = var_id  ;
  using name_t = var_name;

  static_assert(std::is_integral<id_t>::value);
  static_assert(std::is_unsigned<id_t>::value);

  variable(const id_t id)                      : id_(id), name_(name_t("x") + std::to_string(static_cast<unsigned>(id))) { }
  variable(const id_t id, const name_t&  name) : id_(id), name_(          name ) { }
  variable(const id_t id,       name_t&& name) : id_(id), name_(std::move(name)) { }

  variable(const type&  that) = default;
  variable(      type&& that) = default;
  
  variable& operator=(const type&  that) = default;
  variable& operator=(      type&& that) = default;

  bool operator==(const variable& that) const noexcept { return id_ == that.id_; }
  bool operator!=(const variable& that) const noexcept { return id_ != that.id_; }

  explicit operator id_t() const noexcept { return id_; }

  cmn::pbv_t<id_t>   id  () const noexcept { return id_  ; }
  cmn::pbv_t<name_t> name() const noexcept { return name_; }

  id_t&   id  () noexcept { return id_  ; }
  name_t& name() noexcept { return name_; }

private:
  id_t   id_;
  name_t name_;
};

template<typename Out>
Out& operator<<(Out& os, const variable& var) {
  os << var.name();
  return os; }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

using constant = mpq_class;
using arity_t  = std::uint32_t; 
using term     = cmn::math::term::untyped_term<variable, constant, symbol, arity_t, symbol::VAR, symbol::CONST>;
using term_ptr = cmn::misc::simple_shared_ptr<term>;
using var_set  = std::unordered_set<term_ptr>;
using id2var   = std::unordered_map<var_id  , term_ptr>;
using name2var = std::unordered_map<var_name, term_ptr>;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

inline var_set to_var_set(const id2var& i2v) {
  var_set res;
  std::transform(i2v.begin(), i2v.end(), std::inserter(res, res.begin()), [](const auto& kv) { return kv.second; } );
  return res;  }

inline var_set to_var_set(const name2var& n2v) {
  var_set res;
  std::transform(n2v.begin(), n2v.end(), std::inserter(res, res.begin()), [](const auto& kv) { return kv.second; } );
  return res;  }

inline id2var to_i2v(const var_set& vars) {
  id2var res;
  std::transform(vars.begin(), vars.end(), std::inserter(res, res.begin()), [](const auto& v) { return std::make_pair(v->variable().id(), v); } );
  return res;  }

inline name2var to_n2v(const var_set& vars) {
  name2var res;
  std::transform(vars.begin(), vars.end(), std::inserter(res, res.begin()), [](const auto& v) { return std::make_pair(v->variable().name(), v); } );
  return res;  }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** @brief function object for precedence of a term */
struct precedence { 
  std::uint64_t operator()(cmn::pbv_t<term> t) const noexcept { 
  constexpr std::uint64_t MULT_DIV = 20;
    switch(t.symbol()) {
    case symbol::PI    :
    case symbol::E     :
    case symbol::TRUE  :
    case symbol::FALSE :
    case symbol::VAR   : return  0;
    case symbol::CONST : return t.value().get_den()==1 ? 0 : MULT_DIV;

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
    case symbol::COT   : return 1;

    case symbol::EXP   : return 10;
    case symbol::MULT  :
    case symbol::DIV   : return MULT_DIV;

    case symbol::NEG   : return 30;

    case symbol::PLUS  :
    case symbol::MINUS : return 40;

    case symbol::LT    :
    case symbol::LE    :
    case symbol::GT    :
    case symbol::GE    :
    case symbol::EQ    :
    case symbol::NQ    : return 100;
    case symbol::NOT   : return 200;
    case symbol::AND   : return 201;
    case symbol::OR    : return 202;
    case symbol::IMPLY : return 203;
    case symbol::IFF   : return 204;
    }; } };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** @brief function object for associativity of a term */
struct assoc { 
  auto operator()(cmn::pbv_t<term> t) const noexcept { 
    switch(t.symbol()) {
    case symbol::AND   :
    case symbol::OR    :
    case symbol::PLUS  :
    case symbol::MULT  : return cmn::misc::associativity::BOTH;
    case symbol::MINUS :
    case symbol::DIV   : return cmn::misc::associativity::LEFT;
    case symbol::IMPLY :
    case symbol::EXP   : return cmn::misc::associativity::RIGHT;
    default            : return cmn::misc::associativity::NONE ;
    }; } };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** @brief function object for type of a term (infix/prefix) */
struct op_type { 
  auto operator()(cmn::pbv_t<term> t) const noexcept { 
    switch(t.symbol()) {
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
    case symbol::PLUS  :
    case symbol::MINUS :
    case symbol::MULT  :
    case symbol::DIV   :
    case symbol::EXP   : return cmn::misc::op_type::INFIX ;
    default            : return cmn::misc::op_type::PREFIX;
    }; } };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** @brief function for determining special terms (only for the purpose of pretty-printing) */
struct special { 
  auto operator()(cmn::pbv_t<term> t) const noexcept { 
    switch(t.symbol()) {
    case symbol::PI    :
    case symbol::E     :
    case symbol::FALSE : 
    case symbol::TRUE  : return true;
    default            : return false;
    };
} };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** @brief function for handling pretty-printing of special terms */
struct handler { 
  template<typename Out>
  Out& operator()(Out& out, cmn::pbv_t<term> t) const { 
    switch(t.symbol()) {
    case symbol::PI    : out << "pi"   ; return out;
    case symbol::E     : out << "e"    ; return out;
    case symbol::FALSE : out << "false"; return out;
    case symbol::TRUE  : out << "true" ; return out;
    default            : requires_msg(false, "input term is not a constant: (symbol: " << t.symbol() << ")")
                         throw std::invalid_argument("input term is not a constant");
    };
} };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename term_type>
struct default_constant_printer {
  template<typename Out>
  Out& operator()(Out& out, cmn::pbv_t<term_type> term) const { out << term.value(); return out; } };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

using pretty_printer = ::cmn::math::term::pretty_printer<
                         term, 
                         precedence, 
                         assoc, 
                         op_type,
                         ::cmn::math::term::default_symbol_printer  <term>,
                         ::cmn::math::term::default_variable_printer<term>,
                         ::cmn::math::term::default_constant_printer<term>,
                         ::cmn::math::term::default_group_opener    <term>,
                         ::cmn::math::term::default_group_closer    <term>,
                         ::cmn::math::term::default_separator       <term>,
                         special,
                         handler>;

template<typename Out> inline Out& operator<<(Out& os, const term&    t) { return pretty_printer::print(os,  t); }
template<typename Out> inline Out& operator<<(Out& os, const term_ptr t) { return pretty_printer::print(os, *t); }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

inline auto var(const var_id id)                        { return term_ptr(term::variable(variable(id))); }
inline auto var(const var_id id, const var_name&  name) { return term_ptr(term::variable(variable(id,           name ))); }
inline auto var(const var_id id,       var_name&& name) { return term_ptr(term::variable(variable(id, std::move(name)))); }

inline mpq_class str2mpq_class(const std::string& text) { 
  const auto str = cmn::misc::utils::replace_all<std::string>(text, " ", "");
  auto pos = str.find('.');
  if(pos == std::string::npos) return mpq_class(str);
  requires_msg(str.find('.', pos + 1) == std::string::npos, "invalid number " << text)
  const auto integer = cmn::misc::utils::replace_all<std::string>(str, ".", "");
  std::stringstream dbuff;
  const auto len = str.length();
  dbuff << "1";
  while(++pos < len) dbuff << "0";
  return mpq_class(integer, 10) / mpq_class(dbuff.str()); }

inline auto val(const mpq_class&    value) { return term_ptr(term::constant(                          value  )); }
inline auto val(      mpq_class&&   value) { return term_ptr(term::constant(                std::move(value) )); }
inline auto val(      std::string&& value) { return term_ptr(term::constant(str2mpq_class(            value ))); }
inline auto val(const std::string&  value) { return term_ptr(term::constant(str2mpq_class(            value ))); }
inline auto val(const char*         value) { return term_ptr(term::constant(str2mpq_class(std::string(value)))); }
inline auto val(const std::uint32_t value) { return term_ptr(term::constant(constant(value))); }
inline auto val(const std:: int32_t value) { return term_ptr(term::constant(constant(value))); }
inline auto val(const std::uint32_t num, const std::uint32_t denom) { return term_ptr(term::constant(constant(num, denom))); }
inline auto val(const std:: int32_t num, const std:: int32_t denom) { return term_ptr(term::constant(constant(num, denom))); }

const term_ptr ZERO  = val( 0);
const term_ptr ONE   = val( 1);
const term_ptr mONE  = val(-1);
const term_ptr TWO   = val( 2);
const term_ptr TEN   = val(10);
const term_ptr TENTH = val(1,10);

inline auto operator+(const term_ptr fst, const term_ptr snd) { return term_ptr(new term(symbol::PLUS , {fst, snd})); }
inline auto operator-(const term_ptr fst, const term_ptr snd) { return term_ptr(new term(symbol::MINUS, {fst, snd})); }
inline auto operator*(const term_ptr fst, const term_ptr snd) { return term_ptr(new term(symbol::MULT , {fst, snd})); }
inline auto operator/(const term_ptr fst, const term_ptr snd) { return term_ptr(new term(symbol::DIV  , {fst, snd})); }
inline auto operator^(const term_ptr fst, const term_ptr snd) { return term_ptr(new term(symbol::EXP  , {fst, snd})); }

inline auto operator-(const term_ptr op) { return term_ptr(new term(symbol::NEG, {op})); }
inline auto operator!(const term_ptr op) { return term_ptr(new term(symbol::NOT, {op})); }

inline auto abs (const term_ptr op) { return term_ptr(new term(symbol::ABS , {op})); }
inline auto sqrt(const term_ptr op) { return term_ptr(new term(symbol::SQRT, {op})); }
inline auto sqr (const term_ptr op) { return term_ptr(new term(symbol::SQR , {op})); }
inline auto cube(const term_ptr op) { return term_ptr(new term(symbol::CUBE, {op})); }
inline auto log (const term_ptr op) { return term_ptr(new term(symbol::LOG , {op})); }
inline auto ln  (const term_ptr op) { return term_ptr(new term(symbol::LN  , {op})); }
inline auto lg  (const term_ptr op) { return term_ptr(new term(symbol::LG  , {op})); }
inline auto sin (const term_ptr op) { return term_ptr(new term(symbol::SIN , {op})); }
inline auto cos (const term_ptr op) { return term_ptr(new term(symbol::COS , {op})); }
inline auto tan (const term_ptr op) { return term_ptr(new term(symbol::TAN , {op})); }
inline auto cot (const term_ptr op) { return term_ptr(new term(symbol::COT , {op})); }

inline auto operator< (const term_ptr fst, const term_ptr snd) { return term_ptr(new term(symbol::LT, {fst, snd})); }
inline auto operator<=(const term_ptr fst, const term_ptr snd) { return term_ptr(new term(symbol::LE, {fst, snd})); }
inline auto operator> (const term_ptr fst, const term_ptr snd) { return term_ptr(new term(symbol::GT, {fst, snd})); }
inline auto operator>=(const term_ptr fst, const term_ptr snd) { return term_ptr(new term(symbol::GE, {fst, snd})); }
inline auto eq(const term_ptr fst, const term_ptr snd) { return term_ptr(new term(symbol::EQ, {fst, snd})); }
inline auto nq(const term_ptr fst, const term_ptr snd) { return term_ptr(new term(symbol::NQ, {fst, snd})); }

inline auto operator&&(const term_ptr fst, const term_ptr snd) { return term_ptr(new term(symbol::AND  , {fst, snd})); }
inline auto operator||(const term_ptr fst, const term_ptr snd) { return term_ptr(new term(symbol::OR   , {fst, snd})); }
inline auto imply     (const term_ptr fst, const term_ptr snd) { return term_ptr(new term(symbol::IMPLY, {fst, snd})); }
inline auto iff       (const term_ptr fst, const term_ptr snd) { return term_ptr(new term(symbol::IFF  , {fst, snd})); }

const term_ptr TRUE  = term_ptr(new term(symbol::TRUE ));
const term_ptr FALSE = term_ptr(new term(symbol::FALSE));
const term_ptr PI    = term_ptr(new term(symbol::PI   ));
const term_ptr E     = term_ptr(new term(symbol::E    ));

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

} // namespace hare

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace std {

template<> 
struct hash<hare::variable> {
  using argument_type = hare::variable;
  using result_type   = size_t;
  result_type operator()(const argument_type& that) const { return that.id(); } };

template<> 
struct hash<mpz_t> {
  using argument_type = mpz_t;
  using result_type   = size_t;
  result_type operator()(const argument_type& that) const {
    const std::uint64_t size = std::abs(that->_mp_size);
    if(size == 0) return 0;
    result_type res = std::hash<mp_limb_t>()(that->_mp_d[0]);
    for (std::uint64_t i = 1; i < size; i++)
      res ^= std::hash<mp_limb_t>()(that->_mp_d[i]);
    return res;  }  };

template<> 
struct hash<mpq_t> {
  using argument_type = mpq_t;
  using result_type   = size_t;
  result_type operator()(const argument_type& that) const { 
    mpz_t val;
    mpz_init(val);
    mpq_get_num(val, that);
    const auto res = hash<mpz_t>()(val);
    mpq_get_den(val, that);
    return res ^ hash<mpz_t>()(val); } };

template<> 
struct hash<mpz_class> {
  using argument_type = mpz_class;
  using result_type   = size_t;
  result_type operator()(const argument_type& that) const { return hash<mpz_t>()({*that.get_mpz_t()}); } };

template<> 
struct hash<mpq_class> {
  using argument_type = mpq_class;
  using result_type   = size_t;
  result_type operator()(const argument_type& that) const { return hash<mpq_t>()({*that.get_mpq_t()}); } };

} // namespace std

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** OK!! What the hell this part of the code about !?
  * In short it is GMP libraries fault. In their C++ interface they only provide operator<< for std::ostream. This means, using what they have provided, I 
  * cannot send instances of mpq_class to logger of SPDLOG. Even worst GMP defines everything in the global namespace. So adding any new function to this
  * namespace pollute every peice of code in the project. 
  * SPDLOG library is not of the hook either. Their <tt>spdlog::details::line_logger</tt> defines an <tt>operator<<(const T&)</tt> with no restriction on \c T.
  * They say this is "Support user types which implements operator<<". But it is in direct conflict with any such operator that I have implemented. So I simply
  * commented that function and at least by the time of writing this comment, everything is working perfectly fine. The function is decrared/defined in two 
  * files: <tt>spdlog/details/line_logger_fwd.h</tt> and <tt>spdlog/details/line_logger_impl.h</tt>. */

template<typename Out, std::enable_if_t<!std::is_base_of<std::ios_base, Out>::value>* = nullptr>
inline Out& operator<<(Out& os, const mpz_t& num) { 
  std::stringstream buff;
  buff << num;
  os << buff.str();
  return os; }

template<typename Out, std::enable_if_t<!std::is_base_of<std::ios_base, Out>::value>* = nullptr>
inline Out& operator<<(Out& os, const mpq_t& num) { 
  std::stringstream buff;
  buff << num;
  os << buff.str();
  return os; }

template<typename Out, std::enable_if_t<!std::is_base_of<std::ostream, Out>::value>* = nullptr>
inline Out& operator<<(Out& os, const mpz_class& num) { const mpz_t c = {*num.get_mpz_t()}; return operator<<(os, c); }

template<typename Out, std::enable_if_t<!std::is_base_of<std::ostream, Out>::value>* = nullptr>
inline Out& operator<<(Out& os, const mpq_class& num) { const mpq_t c = {*num.get_mpq_t()}; return operator<<(os, c); }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif // HA__TERM__HPP
