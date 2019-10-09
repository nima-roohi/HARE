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

#ifndef HA__TERM_PARSER__HPP
#define HA__TERM_PARSER__HPP

#include "hare/term.hpp"

#include "cmn/dbc.hpp"
#include "cmn/logger.hpp"
#include "cmn/misc/op_type.hpp"
#include "cmn/misc/assoc.hpp"

#include <cctype>       // isspace, isdigit, isalpha, isalnum
#include <cstdint>
#include <functional>   // ptr_fun
#include <stack>
#include <string>

namespace ha::parser {

template<typename It, typename Handler>
void tokenize(const It begin, const It end, Handler& handler) {
  const auto skip  = [end](const auto begin) { return std::find_if_not(begin, end, [](const auto c) { return std::isspace(c); }); };
  const auto op    = [end](const auto begin) { return std::find_if_not(begin, end, [](const auto c) { return std::ispunct(c) && c != '.' && 
                                                                                                             c != '('        && c != ')' ; } ); };
  const auto ident = [end](const auto begin) { return std::find_if_not(begin, end, [](const auto c) { return std::isalnum(c) || c == '\''; } ); };
  const auto num   = [end](const auto begin) { return std::find_if_not(begin, end, [](const auto c) { return std::isdigit(c) || c == '.' ; } ); };
        auto iter  = skip(begin);
  while(iter != end) {
    const auto old_iter = iter;
    const char c = *iter;
         if(c == '(') handler.open (iter++);
    else if(c == ')') handler.close(iter++);
    else if(std::isalpha(c))             handler.identifier(old_iter, iter = ident(iter + 1));
    else if(std::isdigit(c) || c == '.') handler.number    (old_iter, iter = num  (iter + 1));
    else if(std::ispunct(c))             handler.operater  (old_iter, iter = op   (iter + 1));
    else {
      handler.error(iter);
      return; }
    iter = skip(iter); }
  handler.eoi(); }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename C, typename It, typename N2V>
struct default_handler {
  using Str     = std::basic_string<C>;
  using SStream = std::basic_stringstream<C>;

  default_handler(const It begin, const It end, const N2V& n2v) : begin(begin), end(end), n2v(n2v) { }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  /** I define the following as a macro so I have both the reusability and the actual line number! */
  #define raise_expected_error(from, to)                                                                                                              \
    switch(state) {                                                                                                                                   \
      case State::INIT   : requires_msg(false, "expected <num>, <ident>, <func>, <unary-op>, or '(' : "            << mark_position(from, to))      \
      case State::BINARY : requires_msg(false, "expected <num>, <ident>, <func>, <unary-op>, or '(' : "            << mark_position(from, to))      \
      case State::UNARY  : requires_msg(false, "expected <ident>, <func>, <unary-op>, <binary-op>, '(', or ')' : " << mark_position(from, to)) }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  term_ptr result() { 
    requires_msg(term_stack.size() == 1, "unexpected end of input (term_stack has " << term_stack.size() << " elements) input: " << Str(begin, end))
    requires_msg(smbl_stack.size() == 0, "unexpected end of input (smbl_stack has " << smbl_stack.size() << " elements) input: " << Str(begin, end)) 
    return term_stack.top(); }

  void eoi() { 
    while(!smbl_stack.empty()) {
      const auto top = smbl_stack.top(); smbl_stack.pop();
      if(top == symbol::CONST) raise_expected_error(end, end);
      apply_func(top, end, end); } }

  void error(const It pos) { raise_expected_error(pos, (pos < end ? pos + 1 : pos)); }

  void open(const It pos) { 
    opened = true;
    switch(state) {
      case State::INIT   : 
      case State::BINARY :                                 push_smbl(symbol::CONST, pos); break;
      case State::UNARY  : push_smbl(symbol::MULT, pos); push_smbl(symbol::CONST, pos); break; } 
    state = State::INIT; }
  
  void close(const It pos) { 
    if(opened) raise_expected_error(pos, pos + 1);
    switch(state) {
      case State::INIT   : 
      case State::BINARY : raise_expected_error(pos, pos + 1); 
      case State::UNARY  : push_smbl(symbol::VAR, pos); break; } }

  void number(const It begin, const It end) { 
    opened = false;
    constant(val(Str(begin, end)), begin, end); }


  void identifier(const It begin, const It end) {
    opened = false;
    static const auto consts = std::unordered_map<Str, symbol>({ symbol_str(symbol::TRUE), symbol_str(symbol::FALSE), symbol_str(symbol::PI  ), 
                                                                 symbol_str(symbol::E   ) } );
    static const auto funcs  = std::unordered_map<Str, symbol>({ symbol_str(symbol::ABS  ), symbol_str(symbol::SQRT), symbol_str(symbol::SQR ),
                                                                 symbol_str(symbol::CUBE ), symbol_str(symbol::LOG ), symbol_str(symbol::LN  ),
                                                                 symbol_str(symbol::LG   ), symbol_str(symbol::SIN ), symbol_str(symbol::COS ),
                                                                 symbol_str(symbol::TAN  ), symbol_str(symbol::COT ) } );
    if(state==State::UNARY) { push_smbl(symbol::MULT, begin, end); state = State::BINARY; }
    const auto name = Str(begin, end);
    const auto cnst = consts.find(name);
    if(cnst != consts.end()) { constant(term_ptr(new term(cnst->second)), begin, end); return; }
    const auto func = funcs.find(name);
    if(func != funcs .end()) { unary_func(func->second, begin, end); return; }
    const auto var  = n2v  .find(name);
    if(var  != n2v   .end()) { variable  (var ->second, begin, end); return; }
    requires_msg(false, "variable " << name << " is not defined: " << mark_position(begin, end)) }


  void operater(const It begin, const It end) { 
    opened = false;
    static const auto unaries  = std::unordered_map<Str, symbol>({ symbol_str(symbol::NEG), symbol_str(symbol::NOT) });
    static const auto binaries = std::unordered_map<Str, symbol>({ symbol_str(symbol::PLUS), symbol_str(symbol::MINUS), symbol_str(symbol::MULT),
                                                                   symbol_str(symbol::DIV ), symbol_str(symbol::EXP  ), symbol_str(symbol::AND ),
                                                                   symbol_str(symbol::OR  ), symbol_str(symbol::IMPLY), symbol_str(symbol::IFF ),
                                                                   symbol_str(symbol::LT  ), symbol_str(symbol::LE   ), symbol_str(symbol::EQ  ),
                                                                   symbol_str(symbol::GT  ), symbol_str(symbol::GE   ), symbol_str(symbol::NQ  ) } );
    const auto str    = Str(begin, end);
    const auto unary  = unaries .find(str);
    const auto binary = binaries.find(str);
    if(unary != unaries.end()) {
      if(state == State::UNARY && binary != binaries.end()) { 
        binary_func(binary->second, begin, end); 
        return; }
      unary_func(unary->second, begin, end); 
      return; }
    if(binary != binaries.end()) binary_func(binary->second, begin, end);
    else                         raise_expected_error(begin, end); }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

private:

  void constant(const term_ptr& t, const It begin, const It end) { 
    switch(state) {
      case State::UNARY  : raise_expected_error(begin, end);
      case State::INIT   :
      case State::BINARY : push_term(t); state = State::UNARY; break; } }

  void variable(const term_ptr& t, const It begin, const It end) { 
    switch(state) {
      case State::INIT   :
      case State::BINARY : state = State::UNARY;                push_term(t); break;
      case State::UNARY  : push_smbl(symbol::MULT, begin, end); push_term(t); break; } }

  void unary_func(const symbol smbl, const It begin, const It end) { push_smbl(smbl, begin, end); }

  void binary_func(const symbol smbl, const It begin, const It end) { 
    switch(state) {
      case State::INIT   : 
      case State::BINARY : raise_expected_error(begin, end);
      case State::UNARY  : push_smbl(smbl, begin, end); state = State::BINARY; break; } }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  Str mark_position(const It from, const It to) {
    const It first = from - begin > 20 ? from - 20 : begin;
    const It last  = end  - to    > 20 ? to   + 20 : end  ;
    SStream buff;
    buff << Str(first, from); buff << " {{{ ";
    buff << Str(from , to  ); buff << " }}} ";
    buff << Str(to   , last);
    return buff.str(); }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------
  
  static auto symbol_str(const symbol smbl) { 
    SStream buff;
    buff << smbl;
    return std::make_pair(buff.str(), smbl); }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------
  
  static arity_t arity(const symbol smbl) {
    switch(smbl) {
      case symbol::PI    : 
      case symbol::E     : 
      case symbol::TRUE  : 
      case symbol::FALSE : 
      case symbol::CONST : 
      case symbol::VAR   : return 0;
      case symbol::AND   : 
      case symbol::OR    : 
      case symbol::IMPLY : 
      case symbol::IFF   : 
      case symbol::LT    : 
      case symbol::LE    : 
      case symbol::GT    : 
      case symbol::GE    : 
      case symbol::EQ    : 
      case symbol::NQ    : 
      case symbol::PLUS  :
      case symbol::MINUS : 
      case symbol::MULT  : 
      case symbol::DIV   : 
      case symbol::EXP   : return 2;
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
      case symbol::COT   : return 1; }  }

  auto remove_operands(const arity_t arity, const It begin, const It end) {
    if(term_stack.size() < arity) raise_expected_error(begin, end);
    std::vector<term_ptr> res;
    for(arity_t i = 0; i < arity; i++) {
      res.push_back(term_stack.top());
      term_stack.pop(); }
    std::reverse(res.begin(), res.end());
    return res;  }

  void apply_func(const symbol smbl, const It begin, const It end) {
    auto ops  = remove_operands(arity(smbl), begin, end);                      
    auto t = term_ptr(new term(smbl, std::move(ops)));
    term_stack.push(std::move(t)); }

  void push_term(const term_ptr& t)               { term_stack.push(t); }
  void push_smbl(const symbol    smbl, const It pos) { push_smbl(smbl, pos, pos+1); }

  /** @note CONST is used for '(' and VAR is used for ')' */
  void push_smbl(const symbol smbl, const It begin, const It end) { 
    //print_stacks();
    if(smbl == symbol::CONST) {smbl_stack.push(smbl); return; }
    if(smbl == symbol::VAR) {
      while(!smbl_stack.empty()) {
        const auto top = smbl_stack.top(); smbl_stack.pop();
        asserts_msg(top != symbol::VAR, "error in code: " << mark_position(begin, end) )
        if(top == symbol::CONST) return;
        apply_func(top, begin, end); }
      raise_expected_error(begin, end); }

    if(op_type(smbl) == cmn::misc::op_type::POSTFIX) {
      if(term_stack.empty() || state != State::UNARY) raise_expected_error(begin, end);
      while(!smbl_stack.empty()) {
        const auto top = smbl_stack.top();
        if(top == symbol::CONST || precedence(top) <= precedence(smbl)) break; // if precedences are the same, postfix has higher priority
        smbl_stack.pop();
        apply_func(top, begin, end); }
      apply_func(smbl, begin, end); 
      return; }

    while(!smbl_stack.empty()) {
      const auto top = smbl_stack.top();
      if(arity(smbl) == 1) break;
      const auto ptop  = precedence(top );
      const auto psmbl = precedence(smbl);
      if(top == symbol::CONST || ptop > psmbl) break;
      const auto atop  = assoc(top );
      const auto asmbl = assoc(smbl);
      if(ptop == psmbl) {
        requires_msg(atop  != cmn::misc::associativity::NONE, "operator/function " << top << "is not associative: " << mark_position(begin, end))
        requires_msg(asmbl != cmn::misc::associativity::NONE, "operator/function " << top << "is not associative: " << mark_position(begin, end))
        requires_msg(atop  == cmn::misc::associativity::BOTH || asmbl == cmn::misc::associativity::BOTH || atop == asmbl, 
                     "operators/functions " << top << " and " << smbl << "has incompatible associativities: " << mark_position(begin, end))
        const auto asso_ = atop  == cmn::misc::associativity::BOTH ? asmbl : atop;
        const auto asso  = asso_ == cmn::misc::associativity::BOTH ? cmn::misc::associativity::LEFT : asso_;
        if(asso == cmn::misc::associativity::RIGHT) break; } 
      smbl_stack.pop();
      apply_func(top, begin, end); }
    smbl_stack.push(smbl); 

    //print_stacks();
    //std::cout << "-------------------------------------------" << std::endl;
  }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------
  
  std::uint64_t precedence      (const symbol smbl) { return smbl == symbol::CONST || smbl == symbol::VAR ? 0 : ha::precedence()(term(smbl)); }
  cmn::misc::associativity assoc(const symbol smbl) { return smbl == symbol::CONST || smbl == symbol::VAR ? 
                                                             cmn::misc::associativity::NONE               : 
                                                             ha::assoc()(term(smbl)); }
  cmn::misc::op_type op_type    (const symbol smbl) { return smbl == symbol::CONST || smbl == symbol::VAR ? 
                                                             cmn::misc::op_type::PREFIX                   : 
                                                             ha::op_type()(term(smbl)); }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------
  
  void print_stacks() const { 
    std::cout << "TERM STACK: "; print_stack(term_stack); 
    std::cout << "SMBL STACK: "; print_stack(smbl_stack); } 

  template<typename T>
  auto stack_to_vector(std::stack<T> stack) const {
    std::vector<T> res;
    while(!stack.empty()) { res.push_back(stack.top()); stack.pop(); }
    std::reverse(res.begin(), res.end());
    return res; }

  template<typename S>
  void print_stack(const S& stack) const { 
    for(const auto& e : stack_to_vector(stack)) std::cout << e << " "; 
    std::cout << std::endl; }

private:
  enum class State : std::uint8_t { INIT, UNARY, BINARY };

  const It   begin;
  const It   end  ;
  const N2V& n2v  ;

  State state  = State::INIT;
  bool  opened = false;

  std::stack<term_ptr> term_stack;
  std::stack<symbol>   smbl_stack;
  #undef raise_expected_error
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename It, typename N2V>
term_ptr parse_term(const It begin, const It end, const N2V& n2v) {
  log_trace << std::string(begin, end);
  default_handler<char, It, N2V> handler(begin, end, n2v);
  tokenize(begin, end, handler);
  return handler.result(); }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------


} // ha::parser

#endif // HA__TERM_PARSER__HPP

