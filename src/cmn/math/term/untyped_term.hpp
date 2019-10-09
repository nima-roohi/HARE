#ifndef CMN__MATH__TERM__UNTYPED_TERM__HPP
#define CMN__MATH__TERM__UNTYPED_TERM__HPP

#include "cmn/dbc.hpp"
#include "cmn/type_traits.hpp"

#include "cmn/misc/simple_shared_ptr.hpp"

#include <cstddef>             // size_t
#include <cstdint>
#include <cstring>             // strlen
#include <functional>          // hash
#include <initializer_list>
#include <stack>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>
#include <unordered_set>
#include <utility>             // declval, move, forward

namespace cmn::math::term  {

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename V,       //< type of variable names
         typename C,       //< type of constant values
         typename S,       //< type of function symbols 
         typename A,       //< type of function arity   
         S variable_smbl,  //< unique symbol for variables 
         S constant_smbl>  //< unique symbol for constants 
struct untyped_term {
  using type          = untyped_term;
  using operand_type  = misc::simple_shared_ptr<type>;
  using operands_type = std::vector<operand_type>;

  static_assert(!std::is_same<A, bool>::value);
  static_assert( std::is_integral<A>::value);
  static_assert( std::is_unsigned<A>::value);
  static_assert(sizeof(A) <= sizeof(typename operands_type::size_type));
  static_assert(std::is_integral<S>::value || std::is_enum<S>::value);
  static_assert(variable_smbl != constant_smbl);

  using variable_type = V;
  using constant_type = C;
  using symbol_type   = S;
  using arity_type    = A;
  static constexpr S variable_symbol = variable_smbl;
  static constexpr S constant_symbol = constant_smbl;

private:
  template<typename T>
  static constexpr bool is_expt_free_cmp() { return noexcept(std::declval<T>()==std::declval<T>() && std::declval<T>() != std::declval<T>()); }
  static constexpr bool exp_free_cmp = noexcept(is_expt_free_cmp<S>() && is_expt_free_cmp<C>() && is_expt_free_cmp<V>());

  static constexpr const bool necc = noexcept_copy_constructor_t<V>::value && noexcept_copy_constructor_t<C>::value && noexcept_copy_constructor_t<A>::value;
  static constexpr const bool nemc = noexcept_move_constructor_t<V>::value && noexcept_move_constructor_t<C>::value && noexcept_move_constructor_t<A>::value;
  static constexpr const bool nec  = necc && nemc;

  untyped_term(const S symbol, void* /*dummy*/) : smbl(symbol) { requires_msg(!is_function(), "Invalid Symbol: " << symbol);  }

public:
  
  untyped_term(const type& that) noexcept(necc) : smbl(that.smbl) {
    switch(smbl) {
      case variable_symbol: new (&var) V(that.var); break;
      case constant_symbol: new (&val) C(that.val); break;
      default:              new (&ops) operands_type(that.ops);  };  }

  untyped_term(type&& that) noexcept(nemc) : smbl(std::move(that.smbl)) {
    switch(smbl) {
      case variable_symbol: new (&var) V(std::move(that.var)); break;
      case constant_symbol: new (&val) C(std::move(that.val)); break;
      default:              new (&ops) operands_type(std::move(that.ops));  };  }

  type& operator=(const type& that) noexcept(necc) {
    if(this == &that) return *this;
    clear();
    smbl = that.smbl;
    switch(smbl) {
      case variable_symbol: new (&var) V(that.var); break;
      case constant_symbol: new (&val) C(that.val); break;
      default:              new (&ops) operands_type(that.ops);  };  
    return *this;  }

  type& operator=(type&& that) noexcept(nemc) {
    if(this == &that) return *this;
    clear();
    smbl = std::move(that.smbl);
    switch(smbl) {
      case variable_symbol: new (&var) V(std::move(that.var)); break;
      case constant_symbol: new (&val) C(std::move(that.val)); break;
      default:              new (&ops) operands_type(std::move(that.ops));  };  
    return *this;  }

  template<typename CC, std::enable_if_t<std::is_assignable<C&, CC>::value && std::is_fundamental<C>::value>* = nullptr>
  static untyped_term* constant(const CC value) {
    auto res = new untyped_term(constant_symbol, nullptr);
    new (&res->val) C(value); 
    return res; }

  template<typename CC, std::enable_if_t<std::is_assignable<C&, CC>::value && !std::is_fundamental<C>::value>* = nullptr>
  static untyped_term* constant(CC&& value) noexcept(std::is_lvalue_reference<CC>::value ? necc : nemc) {
    auto res = new untyped_term(constant_symbol, nullptr);
    new (&res->val) C(std::forward<CC>(value)); 
    return res; }

//  template<typename VV, std::enable_if_t<std::is_assignable<V&, VV>::value && std::is_fundamental<V>::value>* = nullptr>
//  static untyped_term* variable(const VV name) noexcept(necc) {
//    auto res = new untyped_term(variable_symbol, nullptr);
//    new (&res->var) V(name); 
//    return res; }

  template<typename VV>//, std::enable_if_t<std::is_assignable<V&, VV>::value && !std::is_fundamental<V>::value>* = nullptr>
  static untyped_term* variable(VV&& name) noexcept(std::is_lvalue_reference<VV>::value ? necc : nemc) {
    auto res = new untyped_term(variable_symbol, nullptr);
    new (&res->var) V(std::forward<VV>(name)); 
    return res; }

  explicit untyped_term(const S symbol) noexcept(nemc) : smbl(symbol) { 
    requires(is_function()) 
    new (&ops) operands_type(); } 

  template<typename OO, std::enable_if_t<std::is_same<operands_type, std::remove_reference_t<std::remove_const_t<OO>>>::value>* = nullptr>
  explicit untyped_term(const S symbol, OO&& operands) noexcept(std::is_lvalue_reference<OO>::value ? necc : nemc) : smbl(symbol) {
    requires(is_function()) 
    new (&ops) operands_type(std::forward<OO>(operands));  }

  explicit untyped_term(const S symbol, const std::initializer_list<operand_type>& operands) noexcept(necc) : smbl(symbol) {
    requires(is_function()) 
    new (&ops) operands_type(operands);  }

  explicit untyped_term(S&& symbol, std::initializer_list<operand_type>&& operands) noexcept(nemc) : smbl(symbol) { 
    requires(is_function()) 
    new (&ops) operands_type(std::move(operands));  }

public:
  ~untyped_term() { clear(); }

  bool is_variable() const { return smbl == variable_symbol; }
  bool is_constant() const { return smbl == constant_symbol; }
  bool is_function() const { return smbl != variable_symbol && smbl != constant_symbol; }

  A arity() const noexcept { 
    switch(smbl) {
      case variable_symbol:
      case constant_symbol: return 0;
      default: return ops.size(); };  }

  pbv_t<S> symbol  () const noexcept { return smbl; }
  pbv_t<V> variable() const noexcept { requires(is_variable()) return var; }
  pbv_t<C> value   () const noexcept { requires(is_constant()) return val; }

  pbv_t<operands_type> operands()                const noexcept { requires(is_function()) return ops       ; }

  pbv_t<operand_type>  operator[](const A index) const noexcept { requires(is_function()) return ops[index]; }
  pbv_t<operand_type>  operand_at(const A index) const { 
    requires(is_function()) 
    if(index >= arity())
      throw std::out_of_range("Index (" + std::to_string(index) + ") out of range");
    return ops[index]; }

  operands_type& operands()               noexcept { requires(is_function()) return ops       ; }

  operand_type& operator[](const A index) noexcept { requires(is_function()) return ops[index]; }
  operand_type& operand_at(const A index) noexcept { 
    requires(is_function()) 
    if(index >= arity())
      throw std::out_of_range("Index (" + std::to_string(index) + ") out of range");
    return ops[index]; }

  bool operator!=(const type& that) const noexcept(exp_free_cmp) { return !operator==(that); }
  bool operator==(const type& that) const noexcept(exp_free_cmp) { 
    if(smbl != that.smbl) return false;
    switch(smbl) {
      case variable_symbol: return var == that.var;
      case constant_symbol: return val == that.val;
      default:              return ops == that.ops;  }  }

private:

  template<typename T, std::enable_if_t< std::is_fundamental<T>::value>* = nullptr> void destroy(T& /*obj*/) {           }
  template<typename T, std::enable_if_t<!std::is_fundamental<T>::value>* = nullptr> void destroy(T&   obj  ) { obj.~T(); }

  void clear() {
    switch(smbl) {
      case variable_symbol: destroy(var); break;
      case constant_symbol: destroy(val); break;
      default:              destroy(ops);  };  }

private:
  S smbl;
  union {
    V             var;
    C             val;     
    operands_type ops;
  };
}; 

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename V, typename C, typename S, typename A, S variable_symbol, S constant_symbol> 
auto variables_of(const misc::simple_shared_ptr<untyped_term<V,C,S,A,variable_symbol,constant_symbol>>& term) {
  using elem_type = std::remove_const_t<std::remove_reference_t<decltype(term)>>;
  std::unordered_set<elem_type> res   { };
  std::stack        <elem_type> stack { };
  stack.push(term);
  while(!stack.empty()) {
    const auto top = stack.top(); stack.pop();
    switch(top->symbol()) {
      case constant_symbol: break;
      case variable_symbol: res.insert(top); break;
      default:
        const auto end = top->operands().cend();
        for(auto it = top->operands().cbegin(); it != end; stack.push(*it), it++);
    } };
  return res; }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename V, typename C, typename S, typename A, S variable_symbol, S constant_symbol> 
auto constants_of(const misc::simple_shared_ptr<untyped_term<V,C,S,A,variable_symbol,constant_symbol>>& term) {
  using elem_type = std::remove_const_t<std::remove_reference_t<decltype(term)>>;
  std::unordered_set<elem_type> res   { };
  std::stack        <elem_type> stack { };
  stack.push(term);
  while(!stack.empty()) {
    const auto top = stack.top(); stack.pop();
    switch(top->symbol()) {
      case variable_symbol: break;
      case constant_symbol: res.insert(top); break;
      default:
        const auto end = top->operands().cend();
        for(auto it = top->operands().cbegin(); it != end; stack.push(*it), it++);
    } };
  return res; }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename V, typename C, typename S, typename A, S variable_symbol, S constant_symbol> 
bool is_ground_term(const misc::simple_shared_ptr<untyped_term<V,C,S,A,variable_symbol,constant_symbol>>& term) {
  using elem_type = std::remove_const_t<std::remove_reference_t<decltype(term)>>;
  std::stack<elem_type> stack { };
  stack.push(term);
  while(!stack.empty()) {
    const auto top = stack.top(); stack.pop();
    switch(top->symbol()) {
      case variable_symbol: return false;
      case constant_symbol: break;
      default:
        const auto end = top->operands().cend();
        for(auto it = top->operands().cbegin(); it != end; stack.push(*it), it++);
    } };
  return true; }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename type, typename enabled = void> 
struct symbol_hash_impl { };

template<typename S> 
struct symbol_hash_impl<S, std::enable_if_t< std::is_pointer<S>::value>> {
  using argument_type = S;
  using result_type   = std::size_t;
  result_type operator()(pbv_t<argument_type> that) const { 
    const auto end = that + std::strlen(that);
    result_type res = 0;
    for(auto it = that; it != end; res ^= std::hash<std::remove_const_t<std::remove_pointer_t<S>>>()(*it), it++);
    return res; } };

template<> 
struct symbol_hash_impl<std::string, void> {
  using argument_type = std::string;
  using result_type   = std::size_t;
  result_type operator()(const argument_type& that) const { return std::hash<argument_type>()(that);  }  };

template<typename S> 
struct symbol_hash_impl<S, std::enable_if_t<!std::is_pointer<S>::value>> {
  using argument_type = S;
  using result_type   = std::size_t;
  result_type operator()(const argument_type& that) const { return std::hash<S>()(that);  }  };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

} // namespace cmn::math::term

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace std {

template<typename V, typename C, typename S, typename A, S variable_symbol, S constant_symbol> 
struct hash<::cmn::math::term::untyped_term<V, C, S, A, variable_symbol, constant_symbol>> {
  using argument_type = ::cmn::math::term::untyped_term<V, C, S, A, variable_symbol, constant_symbol>;
  using result_type   = size_t;
  result_type operator()(const argument_type& that) const { 
    auto res = cmn::math::term::symbol_hash_impl<S>()(that.symbol());
    switch(that.symbol()) {
      case variable_symbol: return res ^ cmn::math::term::symbol_hash_impl<V>()(that.variable());
      case constant_symbol: return res ^ cmn::math::term::symbol_hash_impl<C>()(that.value   ());
      default:
        const auto end = that.operands().cend();
        for(auto it = that.operands().cbegin(); it != end; res ^= operator()(**it), it++); 
        return res; } } 
};

} // namespace std

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif // CMN__MATH__TERM__UNTYPED_TERM__HPP
