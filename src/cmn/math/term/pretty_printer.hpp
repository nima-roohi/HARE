#ifndef CMN__MATH__TERM__PRETTY_PRINTER__HPP
#define CMN__MATH__TERM__PRETTY_PRINTER__HPP

#include "cmn/type_traits.hpp"
#include "cmn/misc/assoc.hpp"
#include "cmn/misc/op_type.hpp"
#include "cmn/math/term/untyped_term.hpp"

#include <cstdint>

namespace cmn::math::term {

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename term_type>
struct default_precedence {
  std::uint64_t operator()(pbv_t<term_type>) const noexcept { return 0; } };

template<typename term_type>
struct default_associativity {
  misc::associativity operator()(pbv_t<term_type>) const noexcept { return misc::associativity::NONE; } };

template<typename term_type>
struct default_op_type {
  misc::op_type operator()(pbv_t<term_type>) const noexcept { return misc::op_type::PREFIX; } };

template<typename term_type>
struct default_special {
  bool operator()(pbv_t<term_type>) const noexcept { return false; } };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename term_type>
struct default_symbol_printer {
  template<typename Out>
  Out& operator()(Out& out, pbv_t<term_type> term) const { out << term.symbol(); return out; } };

template<typename term_type>
struct default_variable_printer {
  template<typename Out>
  Out& operator()(Out& out, pbv_t<term_type> term) const { out << term.variable(); return out; } };

template<typename term_type>
struct default_constant_printer {
  template<typename Out>
  Out& operator()(Out& out, pbv_t<term_type> term) const { out << term.value(); return out; } };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename term_type>
struct default_group_opener {
  template<typename Out>
  Out& operator()(Out& out, pbv_t<term_type>) const { out << "("; return out;  }  };

template<typename term_type>
struct default_group_closer {
  template<typename Out>
  Out& operator()(Out& out, pbv_t<term_type>) const { out << ")"; return out;  }  };

template<typename term_type>
struct default_separator {
  template<typename Out>
  Out& operator()(Out& out, pbv_t<term_type>) const { out << ", "; return out;  }  };

template<typename term_type>
struct default_handler {
  template<typename Out>
  Out& operator()(Out& out, pbv_t<term_type>) const { requires(false) return out;  }  };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename term_type,
         typename precedence_t       = default_precedence      <term_type>,
         typename assoc_t            = default_associativity   <term_type>,
         typename op_type_t          = default_op_type         <term_type>,
         typename symbol_printer_t   = default_symbol_printer  <term_type>,
         typename variable_printer_t = default_variable_printer<term_type>,
         typename constant_printer_t = default_constant_printer<term_type>,
         typename group_opener_t     = default_group_opener    <term_type>,
         typename group_closer_t     = default_group_closer    <term_type>,
         typename separator_t        = default_separator       <term_type>,
         typename special_t          = default_special         <term_type>,
         typename handler_t          = default_handler         <term_type>>
struct pretty_printer {
  using type = pretty_printer;
  using S = typename term_type::symbol_type;
  using A = typename term_type::arity_type;

  static const precedence_t       precedence      ;
  static const assoc_t            associativity   ;
  static const op_type_t          ops_order       ;
  static const symbol_printer_t   symbol_printer  ;
  static const variable_printer_t variable_printer;
  static const constant_printer_t constant_printer;
  static const group_opener_t     group_opener    ;
  static const group_closer_t     group_closer    ;
  static const separator_t        separator       ;
  static const special_t          special         ;
  static const handler_t          handler         ;

  template<typename Out>
  static Out& print(Out& out, const term_type& term) {
    if(special(term)) return handler(out, term);
    switch(term.symbol()) {
      case term_type::variable_symbol: return variable_printer(out, term);
      case term_type::constant_symbol: return constant_printer(out, term); 
      default: /* no op */ break; };
    const auto arity = term.arity   ();
    const auto prec  = precedence   (term);
    const auto order = ops_order    (term);
    const auto assoc = associativity(term);
    if(arity <= 0) {
      group_opener  (out, term);
      symbol_printer(out, term);
      group_closer  (out, term);
      return out; }
    const auto prefix = order == misc::op_type::PREFIX || (arity == 1 && order == misc::op_type::INFIX);
    const auto suffix = order == misc::op_type::POSTFIX;
    const auto infix  = !prefix && !suffix;
    if(prefix) symbol_printer(out, term);
    if(!infix) group_opener  (out, term);
    for(A i = 0; i < arity; i++) {
      const auto& op = term[i];
      const auto cprec = precedence(*op);
      bool parenthesis = prec < cprec && arity > 1;
      if(!parenthesis && prec == cprec && order == misc::op_type::INFIX) {
        const auto assoc_combined = intersect(assoc, associativity(*op));
        if(assoc_combined == misc::associativity::NONE          ||
          (i> 0       && i < arity - 1                                && assoc_combined != misc::associativity::BOTH) ||
          (i==0       && assoc_combined != misc::associativity::LEFT  && assoc_combined != misc::associativity::BOTH) || 
          (i==arity-1 && assoc_combined != misc::associativity::RIGHT && assoc_combined != misc::associativity::BOTH) )
          parenthesis = true;  }
      if(i > 0) {
        if(infix) symbol_printer(out, term);
        else      separator     (out, term); }
      if(parenthesis) group_opener(out, term);
      print(out, *term[i]);
      if(parenthesis) group_closer(out, term);
    }
    if(!infix) group_closer  (out, term);
    if(suffix) symbol_printer(out, term);
    return out;
  }

private:
  static misc::associativity intersect(const misc::associativity parent, const misc::associativity child) {
    if( parent == misc::associativity::BOTH && child  == misc::associativity::BOTH )                                          return misc::associativity::BOTH ;
    if((parent == misc::associativity::BOTH || parent == misc::associativity::LEFT ) && child  == misc::associativity::LEFT ) return misc::associativity::LEFT ;
    if((parent == misc::associativity::BOTH || parent == misc::associativity::RIGHT) && child  == misc::associativity::RIGHT) return misc::associativity::RIGHT;
    if((child  == misc::associativity::BOTH || child  == misc::associativity::LEFT ) && parent == misc::associativity::LEFT ) return misc::associativity::LEFT ;
    if((child  == misc::associativity::BOTH || child  == misc::associativity::RIGHT) && parent == misc::associativity::RIGHT) return misc::associativity::RIGHT;
    return misc::associativity::NONE;
  }
};

template<typename T, typename P, typename A, typename O, typename S, typename V, typename C, typename GO, typename GC, typename SEP, typename SP, typename H> 
const P pretty_printer<T, P, A, O, S, V, C, GO, GC, SEP, SP, H>::precedence { };

template<typename T, typename P, typename A, typename O, typename S, typename V, typename C, typename GO, typename GC, typename SEP, typename SP, typename H> 
const A pretty_printer<T, P, A, O, S, V, C, GO, GC, SEP, SP, H>::associativity { };

template<typename T, typename P, typename A, typename O, typename S, typename V, typename C, typename GO, typename GC, typename SEP, typename SP, typename H> 
const O pretty_printer<T, P, A, O, S, V, C, GO, GC, SEP, SP, H>::ops_order { };

template<typename T, typename P, typename A, typename O, typename S, typename V, typename C, typename GO, typename GC, typename SEP, typename SP, typename H> 
const S pretty_printer<T, P, A, O, S, V, C, GO, GC, SEP, SP, H>::symbol_printer { };

template<typename T, typename P, typename A, typename O, typename S, typename V, typename C, typename GO, typename GC, typename SEP, typename SP, typename H> 
const V pretty_printer<T, P, A, O, S, V, C, GO, GC, SEP, SP, H>::variable_printer { };

template<typename T, typename P, typename A, typename O, typename S, typename V, typename C, typename GO, typename GC, typename SEP, typename SP, typename H> 
const C pretty_printer<T, P, A, O, S, V, C, GO, GC, SEP, SP, H>::constant_printer { };

template<typename T, typename P, typename A, typename O, typename S, typename V, typename C, typename GO, typename GC, typename SEP, typename SP, typename H> 
const GO pretty_printer<T, P, A, O, S, V, C, GO, GC, SEP, SP, H>::group_opener { };

template<typename T, typename P, typename A, typename O, typename S, typename V, typename C, typename GO, typename GC, typename SEP, typename SP, typename H> 
const GC pretty_printer<T, P, A, O, S, V, C, GO, GC, SEP, SP, H>::group_closer { };

template<typename T, typename P, typename A, typename O, typename S, typename V, typename C, typename GO, typename GC, typename SEP, typename SP, typename H> 
const SEP pretty_printer<T, P, A, O, S, V, C, GO, GC, SEP, SP, H>::separator { };

template<typename T, typename P, typename A, typename O, typename S, typename V, typename C, typename GO, typename GC, typename SEP, typename SP, typename H> 
const SP pretty_printer<T, P, A, O, S, V, C, GO, GC, SEP, SP, H>::special { };

template<typename T, typename P, typename A, typename O, typename S, typename V, typename C, typename GO, typename GC, typename SEP, typename SP, typename H> 
const H pretty_printer<T, P, A, O, S, V, C, GO, GC, SEP, SP, H>::handler { };

} // namespace cmn::math::term

#endif // CMN__MATH__TERM__PRETTY_PRINTER__HPP
