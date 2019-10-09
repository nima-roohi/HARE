#ifndef CMN__MATH__CONTINUOUS_DYNAMIC_INTERVAL__HPP
#define CMN__MATH__CONTINUOUS_DYNAMIC_INTERVAL__HPP

#include "cmn/dbc.hpp"
#include "cmn/type_traits.hpp"
#include "cmn/math/extended_number.hpp"

#include <algorithm>  // sort
#include <sstream>    // stringstream
#include <tuple>
#include <utility>    // move, forward

namespace cmn::math {

enum class interval_bound_kind : unsigned char { OPEN = 0, CLOSED = 1 };

template<typename Out>
inline Out& operator<<(Out& os, const interval_bound_kind kind) {
  switch(kind) {
  case interval_bound_kind::OPEN   : { os << "open"   ; return os; }
  case interval_bound_kind::CLOSED : { os << "closed" ; return os; }
  }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename N,
         typename CP = constant_provider<N>>
class continuous_dynamic_interval {

public:

  using type  = continuous_dynamic_interval;
  using ntype = extended_number<N,CP>;
  using Kind  = interval_bound_kind;
  
  static constexpr const bool necc = noexcept_copy_constructor_t<ntype>::value;
  static constexpr const bool nemc = noexcept_move_constructor_t<ntype>::value;
  static constexpr const bool nec  = necc && nemc;
  static constexpr const bool necmp = noexcept(std::declval<ntype>() <  std::declval<ntype>()) &&
                                      noexcept(std::declval<ntype>() >  std::declval<ntype>()) &&
                                      noexcept(std::declval<ntype>() <= std::declval<ntype>()) &&
                                      noexcept(std::declval<ntype>() >= std::declval<ntype>()) &&
                                      noexcept(std::declval<ntype>() == std::declval<ntype>()) &&
                                      noexcept(std::declval<ntype>() != std::declval<ntype>()) &&
                                      noexcept(std::declval<ntype>().compare_to(std::declval<ntype>())) &&
                                      noexcept(std::declval<ntype>().is_one ())                &&
                                      noexcept(std::declval<ntype>().is_zero())                &&
                                      noexcept(std::declval<ntype>().is_positive())            &&
                                      noexcept(std::declval<ntype>().is_negative())            &&
                                      noexcept(std::declval<ntype>().is_infinite())            &&
                                      noexcept(std::declval<ntype>().is_positive_infinite())   &&
                                      noexcept(std::declval<ntype>().is_negative_infinite()) ;
  static constexpr const bool nearith = noexcept( std::declval<ntype>() +  std::declval<ntype>()) &&
                                        noexcept( std::declval<ntype>() -  std::declval<ntype>()) &&
                                        noexcept( std::declval<ntype>() *  std::declval<ntype>()) &&
                                        noexcept( std::declval<ntype>() /  std::declval<ntype>()) &&
                                        noexcept( std::declval<ntype>() += std::declval<ntype>()) &&
                                        noexcept( std::declval<ntype>() -= std::declval<ntype>()) &&
                                        noexcept( std::declval<ntype>() *= std::declval<ntype>()) &&
                                        noexcept( std::declval<ntype>() /= std::declval<ntype>()) && 
                                        noexcept(-std::declval<ntype>())                          &&
                                        noexcept( std::declval<ntype>().negate ())                &&
                                        noexcept( std::declval<ntype>().inverse())                ;

private:
  ntype inf_;
  ntype sup_;
  Kind lkind : 1;
  Kind rkind : 1;

public:
 
  //------- C O N S T R U C T O R S ---------------------------------------------------------------------------------------------------------------------------

  continuous_dynamic_interval(const Kind left_kind, const ntype&  inf, const ntype&  sup, const Kind right_kind) noexcept(necc) : 
    inf_(          inf ), sup_(          sup ), lkind(left_kind), rkind(right_kind) { requires_msg(invariant(), to_string()) } 
  
  continuous_dynamic_interval(const Kind left_kind,       ntype&& inf, const ntype&  sup, const Kind right_kind) noexcept(nec)  : 
    inf_(std::move(inf)), sup_(          sup ), lkind(left_kind), rkind(right_kind) { requires_msg(invariant(), to_string()) } 
  
  continuous_dynamic_interval(const Kind left_kind, const ntype&  inf,       ntype&& sup, const Kind right_kind) noexcept(nec)  : 
    inf_(          inf ), sup_(std::move(sup)), lkind(left_kind), rkind(right_kind) { requires_msg(invariant(), to_string()) } 
  
  continuous_dynamic_interval(const Kind left_kind,       ntype&& inf,       ntype&& sup, const Kind right_kind) noexcept(nemc) : 
    inf_(std::move(inf)), sup_(std::move(sup)), lkind(left_kind), rkind(right_kind) { requires_msg(invariant(), to_string()) } 
  
  continuous_dynamic_interval(const ntype&  inf, const ntype&  sup) noexcept(necc) : type(Kind::CLOSED, inf           , sup           , Kind::CLOSED) { }
  continuous_dynamic_interval(      ntype&& inf, const ntype&  sup) noexcept(nec ) : type(Kind::CLOSED, std::move(inf), sup           , Kind::CLOSED) { }
  continuous_dynamic_interval(const ntype&  inf,       ntype&& sup) noexcept(nec ) : type(Kind::CLOSED, inf           , std::move(sup), Kind::CLOSED) { }
  continuous_dynamic_interval(      ntype&& inf,       ntype&& sup) noexcept(nemc) : type(Kind::CLOSED, std::move(inf), std::move(sup), Kind::CLOSED) { }
  
  explicit continuous_dynamic_interval(                   ) noexcept(necc) : type(Kind::CLOSED, CP::ZERO, CP::ZERO, Kind::CLOSED) { }
  explicit continuous_dynamic_interval(const ntype&  value) noexcept(necc) : type(Kind::CLOSED, value, value, Kind::CLOSED) { }
  explicit continuous_dynamic_interval(      ntype&& value) noexcept(nec ) : inf_(value), sup_(std::move(value)), lkind(Kind::CLOSED), rkind(Kind::CLOSED) { } 

  //------- D E F A U L T - M E T H O D S ---------------------------------------------------------------------------------------------------------------------
  
  continuous_dynamic_interval(const continuous_dynamic_interval& ) = default; 
  continuous_dynamic_interval(      continuous_dynamic_interval&&) = default; 
  continuous_dynamic_interval& operator=(const continuous_dynamic_interval& ) = default; 
  continuous_dynamic_interval& operator=(      continuous_dynamic_interval&&) = default; 
  
  //------- S T A T I C - C R E A T O R S ---------------------------------------------------------------------------------------------------------------------
  
  static type le(const ntype& max) noexcept(necc) { return type(Kind::OPEN  , ntype::NEG_INF, max           , Kind::CLOSED); }
  static type lt(const ntype& sup) noexcept(necc) { return type(Kind::OPEN  , ntype::NEG_INF, sup           , Kind::OPEN  ); }
  static type ge(const ntype& min) noexcept(necc) { return type(Kind::CLOSED, min           , ntype::POS_INF, Kind::OPEN  ); }
  static type gt(const ntype& inf) noexcept(necc) { return type(Kind::OPEN  , inf           , ntype::POS_INF, Kind::OPEN  ); }
  static type eq(const ntype& num) noexcept(necc) { return type(num); }
  static type universal() noexcept(necc) { return type(Kind::OPEN, ntype::NEG_INF, ntype::POS_INF, Kind::OPEN); }
  static type empty    () noexcept(necc) { return type(Kind::OPEN, ntype::ONE    , ntype::ZERO   , Kind::OPEN); }

  //------- B O R D E R - T E S T S ---------------------------------------------------------------------------------------------------------------------------
  
  constexpr bool is_bounded        () const noexcept(noexcept(inf_.is_finite  ())) { return inf_.is_finite  () && sup_.is_finite  (); }
  constexpr bool is_unbounded      () const noexcept(noexcept(inf_.is_infinite())) { return inf_.is_infinite() || sup_.is_infinite(); }
  constexpr bool is_left_bounded   () const noexcept(noexcept(inf_.is_finite  ())) { return inf_.is_finite  (); }
  constexpr bool is_left_unbounded () const noexcept(noexcept(inf_.is_infinite())) { return inf_.is_infinite(); }
  constexpr bool is_right_bounded  () const noexcept(noexcept(sup_.is_finite  ())) { return sup_.is_finite  (); }
  constexpr bool is_right_unbounded() const noexcept(noexcept(sup_.is_infinite())) { return sup_.is_infinite(); }
  constexpr bool is_closed         () const noexcept { return lkind == Kind::CLOSED && rkind == Kind::CLOSED; }
  constexpr bool is_open           () const noexcept { return lkind == Kind::OPEN   && rkind == Kind::OPEN  ; }
  constexpr bool is_left_closed    () const noexcept { return lkind == Kind::CLOSED; }
  constexpr bool is_right_closed   () const noexcept { return rkind == Kind::CLOSED; }
  constexpr bool is_left_open      () const noexcept { return lkind == Kind::OPEN  ; }
  constexpr bool is_right_open     () const noexcept { return rkind == Kind::OPEN  ; }
  constexpr bool is_half_open      () const noexcept { return lkind == Kind::CLOSED ^ rkind == Kind::CLOSED; }
  constexpr bool is_half_closed    () const noexcept { return lkind == Kind::CLOSED ^ rkind == Kind::CLOSED; }
  
  //------- M E M B E R S H I P - T E S T S -------------------------------------------------------------------------------------------------------------------

  bool is_empty    () const noexcept(necmp) {
    const auto cmp = inf_.compare_to(sup_);
    return cmp > 0 ? true  :
           cmp < 0 ? false :
           lkind == Kind::OPEN || rkind == Kind::OPEN;  }
  bool is_nonempty () const noexcept(noexcept(     is_empty   ())) { return !is_empty(); }
  bool is_universal() const noexcept(noexcept(inf_.is_infinite())) { return inf_.is_infinite() && sup_.is_infinite(); }
  bool is_singleton() const noexcept(necmp)                        { return is_closed() && inf_ == sup_; }

  bool contains(pbv_t<N> value) const noexcept(ntype::necmp) {
    const auto inf_cmp = inf_.compare_to(value);
    if( inf_cmp >  0 || (inf_cmp == 0 && lkind == Kind::OPEN)) return false;
    const auto sup_cmp = sup_.compare_to(value);
    if( sup_cmp <  0 || (sup_cmp == 0 && rkind == Kind::OPEN)) return false;
    return true;  }

  bool contains(const ntype& value) const noexcept(necmp) {
    if(value.is_infinite()) return false;
    return contains(value.value()); }

  bool contains_zero    () const noexcept(noexcept(contains(ntype::ZERO))) { return contains(ntype::ZERO); }
  bool contains_positive() const noexcept(necmp) { return is_nonempty() && sup_ > CP::ZERO; }
  bool contains_negative() const noexcept(necmp) { return is_nonempty() && inf_ < CP::ZERO; }

  //------- S U B S E T S - A N D - E Q U A L I T Y -----------------------------------------------------------------------------------------------------------
  
  bool is_subset_of(const type& that) const noexcept(necmp) {
    if(     is_empty()) return true ;
    if(that.is_empty()) return false;
    const auto inf_cmp = inf_.is_infinite() && that.inf_.is_infinite() ? 0 : inf_.compare_to(that.inf_);
    const auto sup_cmp = sup_.is_infinite() && that.sup_.is_infinite() ? 0 : sup_.compare_to(that.sup_);
    return 0 <= inf_cmp && sup_cmp <= 0 &&
          (0 < inf_cmp || lkind == Kind::OPEN || that.lkind == Kind::CLOSED) &&
          (0 > sup_cmp || rkind == Kind::OPEN || that.rkind == Kind::CLOSED) ;  }

  bool is_superset_of(const type& that) const noexcept(necmp) { return that.is_subset_of(*this); }

  bool operator==(const type& that) const noexcept(necmp) { return  is_subset_of(that) &&  that.is_subset_of(*this); }
  bool operator!=(const type& that) const noexcept(necmp) { return !is_subset_of(that) || !that.is_subset_of(*this); }
  
  //------- O R D E R I N G ----------------------------------------------------------------------------------------------------------------------------------- 
  
  bool operator<=(pbv_t<N> that) const noexcept(necmp) { return is_empty() || sup_ <= that; }
  bool operator>=(pbv_t<N> that) const noexcept(necmp) { return is_empty() || inf_ >= that; }

  bool operator< (pbv_t<N> that) const noexcept(necmp) { 
    if(is_empty()) return true;
    if(rkind == Kind::CLOSED) return sup_ <  that;
    else                      return sup_ <= that; }

  bool operator> (pbv_t<N> that) const noexcept(necmp) { 
    if(is_empty()) return true;
    if(lkind == Kind::CLOSED) return inf_ >  that;
    else                      return inf_ >= that; }

  bool operator<=(const ntype& that) const noexcept(necmp) { return is_empty() || that.is_positive_infinity() || sup_ <= that; }
  bool operator>=(const ntype& that) const noexcept(necmp) { return is_empty() || that.is_negative_infinity() || inf_ >= that; }

  bool operator< (const ntype& that) const noexcept(necmp) { 
    if(is_empty() || that.is_positive_infinity()) return true;
    if(rkind == Kind::CLOSED) return sup_ <  that;
    else                      return sup_ <= that; }

  bool operator> (const ntype& that) const noexcept(necmp) { 
    if(is_empty() || that.is_negative_infinity()) return true;
    if(lkind == Kind::CLOSED) return inf_ >  that;
    else                      return inf_ >= that; }

  /* IMPORTANT: A <= B && B <= A does NOT necessarily imply A == B */
  
  bool operator<=(const type& that) const noexcept(necmp) { return  is_empty() || that.is_empty() || sup_ <= that.inf_; }
  bool operator>=(const type& that) const noexcept(necmp) { return  is_empty() || that.is_empty() || inf_ >= that.sup_; }

  bool operator< (const type& that) const noexcept(necmp) { 
    if(is_empty() || that.is_empty()) return true;
    if(rkind == Kind::OPEN) return sup_ <= that.inf_;
    const auto cmp = sup_.compare_to(that.inf_);
    return cmp < 0 || (cmp == 0 && that.lkind == Kind::OPEN); }

  bool operator> (const type& that) const noexcept(necmp) { 
    if(is_empty() || that.is_empty()) return true;
    if(lkind == Kind::OPEN) return inf_ >= that.sup_;
    const auto cmp = inf_.compare_to(that.sup_);
    return cmp > 0 || (cmp == 0 && that.rkind == Kind::OPEN); }

  //------- I N T E R S E C T I O N ---------------------------------------------------------------------------------------------------------------------------

  type&& operator&=(const type& that) noexcept(necmp && necc) {
    const auto inf_cmp = inf_.is_infinite() ? -1 : inf_.compare_to(that.inf_);
    const auto sup_cmp = sup_.is_infinite() ?  1 : sup_.compare_to(that.sup_);
         if(inf_cmp <  0) { lkind = that.lkind; inf_ = that.inf_; }
    else if(inf_cmp == 0 && lkind == Kind::CLOSED && that.lkind == Kind::OPEN) { lkind = Kind::OPEN; } ;
         if(sup_cmp >  0) { rkind = that.rkind; sup_ = that.sup_; }
    else if(sup_cmp == 0 && rkind == Kind::CLOSED && that.rkind == Kind::OPEN) { rkind = Kind::OPEN; } ;
    ensures(invariant());
    return std::forward<type>(*this); }

  type   operator&(const type&  that) const &  noexcept(necmp && necc) {
    const auto inf_cmp = inf_.is_infinite() ? -1 : inf_.compare_to(that.inf_);
    const auto sup_cmp = sup_.is_infinite() ?  1 : sup_.compare_to(that.sup_);
    Kind  res_lkind, res_rkind;
    const ntype &res_inf = inf_cmp > 0 ? ({ res_lkind =      lkind;                        inf_; }) : 
                           inf_cmp < 0 ? ({ res_lkind = that.lkind;                   that.inf_; }) :
                                         ({ res_lkind = intersect(lkind, that.lkind);      inf_; }) ;
    const ntype &res_sup = sup_cmp < 0 ? ({ res_rkind =      rkind;                        sup_; }) : 
                           sup_cmp > 0 ? ({ res_rkind = that.rkind;                   that.sup_; }) :
                                         ({ res_rkind = intersect(rkind, that.rkind);      sup_; }) ;
    return type(res_lkind, res_inf, res_sup, res_rkind); }

  type&& operator&(const type&  that)       && noexcept(necmp && necc) { return      operator&=( that); }
  type&& operator&(      type&& that)       && noexcept(necmp && necc) { return      operator&=( that); }
  type&& operator&(      type&& that) const &  noexcept(necmp && necc) { return that.operator&=(*this); }
  
  //------- I N V E R S I O N ---------------------------------------------------------------------------------------------------------------------------------

  type inverse() const noexcept(nearith && necmp && nec) { 
    requires(!contains_zero())
    if(is_empty()) return empty();
    const auto res_inf = sup_ == ntype::ZERO ? ntype::NEG_INF : sup_.inverse();
    const auto res_sup = inf_ == ntype::ZERO ? ntype::POS_INF : inf_.inverse();
    return type(rkind, res_inf, res_sup, lkind); }
  
  type&& inverse() noexcept(nearith && necmp && nec) {
    requires(!contains_zero())
    if(is_empty()) return std::forward<type>(*this);
    ntype inf_cpy = inf_;
    inf_ = sup_   .is_zero() ? ntype::NEG_INF : std::move(sup_   ).inverse();
    sup_ = inf_cpy.is_zero() ? ntype::POS_INF : std::move(inf_cpy).inverse();
    const Kind t = lkind; lkind = rkind; rkind = t;
    ensures(invariant());
    return std::forward<type>(*this); }
  
  //------- N E G A T I O N -----------------------------------------------------------------------------------------------------------------------------------
 
  type&& negate()       noexcept(nearith) {
    const auto t = lkind; lkind = rkind; rkind = t;
    inf_.negate();
    sup_.negate();
    std::swap(inf_, sup_); 
    ensures(invariant());
    return std::forward<type>(*this); }

  type   operator-() const &  { return type(rkind, -sup_, -inf_, lkind); }
  type   operator-() const && { return this->operator-(); } 
  type&& operator-()       && { return negate(); }
  
  //------- N U M B E R - A D D I T I O N ---------------------------------------------------------------------------------------------------------------------
  
  type   operator+ (pbv_t<N>     value) const noexcept(nearith && nec) { return type(lkind, inf_ + value, sup_ + value, rkind); }
  type&& operator+=(pbv_t<N>     value)       noexcept(nearith       ) { inf_ += value; sup_ += value; ensures(invariant()) return std::forward<type>(*this); }
  type   operator+ (const ntype& value) const noexcept(nearith && nec) { return value.is_finite() ? operator+ (value.value()) : empty     (); } 
  type&& operator+=(const ntype& value)       noexcept(nearith && nec) { return value.is_finite() ? operator+=(value.value()) : make_empty(); } 

  //------- N U M B E R - S U B T R A C T I O N ---------------------------------------------------------------------------------------------------------------
  
  type   operator- (pbv_t<N>     value) const noexcept(nearith && nec) { return type(lkind, inf_ - value, sup_ - value, rkind); }
  type&& operator-=(pbv_t<N>     value)       noexcept(nearith       ) { inf_ -= value; sup_ -= value; ensures(invariant()) return std::forward<type>(*this); }
  type   operator- (const ntype& value) const noexcept(nearith && nec) { return value.is_finite() ? operator- (value.value()) : empty     (); } 
  type&& operator-=(const ntype& value)       noexcept(nearith && nec) { return value.is_finite() ? operator-=(value.value()) : make_empty(); } 
  
  //------- N U M B E R - M U L T I P L I C A T I O N ---------------------------------------------------------------------------------------------------------
  
  type   operator*(pbv_t<N> value) const noexcept(nearith && ntype::necmp && nec) { 
    if(is_empty()) return empty();
    const auto  cmp = ntype::ZERO.compare_to(value);
    return cmp < 0 ? type(lkind, inf_ * value, sup_ * value, rkind) :
           cmp > 0 ? type(rkind, sup_ * value, inf_ * value, lkind) :
           type(ntype::ZERO); }
  
  type&& operator*=(pbv_t<N> value)      noexcept(nearith && ntype::necmp && nec) { 
    if(is_empty()) return std::forward<type>(*this);
    const auto  cmp = ntype::ZERO.compare_to(value);
    if(cmp < 0) {
      inf_ *= value;
      sup_ *= value;
    } else if(cmp > 0) {
      inf_ *= value;
      sup_ *= value;
      const Kind t = lkind; lkind = rkind; rkind = t;
      std::swap(inf_ , sup_ );
    } else make_zero(); 
    ensures(invariant());
    return std::forward<type>(*this); }
  
  type   operator* (const ntype& value) const noexcept(nearith && ntype::necmp && nec) { 
    requires(value.is_finite() || !contains_zero())
    return value.is_finite() ? operator* (value.value()) : empty(); }
  
  type&& operator*=(const ntype& value)       noexcept(nearith && ntype::necmp && nec) { 
    requires(value.is_finite() || !contains_zero())
    return value.is_finite() ? operator*=(value.value()) : make_empty(); }
  
  //------- N U M B E R - D I V I S I O N ---------------------------------------------------------------------------------------------------------------------
  
  type operator/(pbv_t<N> value) const noexcept(nearith && ntype::necmp && nec) { 
    requires(value != CP::ZERO)
    if(is_empty()) return empty();
    return value > CP::ZERO ? type(lkind, inf_ / value, sup_ / value, rkind) :
                              type(rkind, sup_ / value, inf_ / value, lkind) ; }

  type&& operator/=(pbv_t<N> value) noexcept(nearith && ntype::necmp && nec) { 
    requires(value != CP::ZERO)
    if(is_empty()) return std::forward<type>(*this);
    inf_ /= value;
    sup_ /= value;
    if(value < CP::ZERO) {
      const Kind t = lkind; lkind = rkind; rkind = t;
      std::swap(inf_ , sup_); }
    ensures(invariant());
    return std::forward<type>(*this); }

  type   operator/ (const ntype& value) const noexcept(nearith && ntype::necmp && nec) { 
    requires(value.is_nonzero())
    return value.is_finite() ? operator/ (value.value()) : 
           contains_zero  () ? type(ntype::ZERO)         :
                               empty()                   ; }

  type&& operator/=(const ntype& value) noexcept(nearith && ntype::necmp && nec) { 
    requires(value.is_nonzero()) 
    return value.is_finite() ? operator/=(value.value()) : 
           contains_zero  () ? make_zero ()              :
                               make_empty()              ; } 
  
  //------- I N T E R V A L - A D D I T I O N -----------------------------------------------------------------------------------------------------------------
  
  type&& operator+=(const type& that) noexcept(nearith && ntype::necmp && nec) {
    if(     is_empty()) return std::forward<type>(*this);
    if(that.is_empty()) return make_empty();
    inf_ += that.inf_;
    sup_ += that.sup_;
    if(that.lkind == Kind::OPEN) lkind = Kind::OPEN;
    if(that.rkind == Kind::OPEN) rkind = Kind::OPEN; 
    ensures(invariant());
    return std::forward<type>(*this); }

  type operator+(const type& that) const & noexcept(nearith && ntype::necmp && nec) {
    if(is_empty() || that.is_empty()) return empty();
    return type(intersect(lkind, that.lkind),
                inf_ + that.inf_,
                sup_ + that.sup_,
                intersect(rkind, that.rkind));  }

  type&& operator+(const type&  that)       && { return operator+=(that); }
  type&& operator+(      type&& that)       && { return operator+=(that); }
  type&& operator+(      type&& that) const &  { return that += *this; }
  
  //------- I N T E R V A L - S U B T R A C T I O N -----------------------------------------------------------------------------------------------------------
  
  type&& operator-=(const type& that)       noexcept(nearith && ntype::necmp && nec) {
    if(     is_empty()) return std::forward<type>(*this);
    if(that.is_empty()) return make_empty();
    inf_ -= that.sup_;
    sup_ -= that.inf_;
    lkind = intersect(lkind, that.rkind);
    rkind = intersect(rkind, that.lkind);
    ensures(invariant());
    return std::forward<type>(*this); }
  
  type operator-(const type& that) const & noexcept(nearith && ntype::necmp && nec) {
    if(is_empty() || that.is_empty()) return empty();
    return type(intersect(lkind, that.rkind),
                inf_ - that.sup_,
                sup_ - that.inf_,
                intersect(rkind, that.lkind));  }

  type&& operator-(const type&  that)       && { return operator-=(that); }
  type&& operator-(      type&& that)       && { return operator-=(that); }
  type&& operator-(      type&& that) const &  { return that.negate() += *this; }

  //------- I N T E R V A L - M U L T I P L I C A T I O N -----------------------------------------------------------------------------------------------------

  type&& operator*=(const type& that) noexcept(nearith && ntype::necmp && nec) {
    if(is_empty() || that.is_empty()) return make_empty();
    return mult_internal(
             that, 
             [this](const Kind new_lkind, const ntype& new_inf, const ntype& new_sup, const Kind new_rkind) -> type&& {
               this->sup_  = new_sup; 
               this->inf_  = new_inf;
               this->lkind = new_lkind;
               this->rkind = new_rkind; 
               ensures(this->invariant());
               return std::forward<type>(*this); }); }

  type operator*(const type& that) const & noexcept(nearith && ntype::necmp && nec) {
    if(is_empty() || that.is_empty()) return empty();
    return mult_internal(
             that, 
             [](const Kind new_lkind, const ntype& new_inf, const ntype& new_sup, const Kind new_rkind) -> type {
               return type(new_lkind, new_inf, new_sup, new_rkind); }); }

  type&& operator*(const type&  that)       && { return operator*=(that); }
  type&& operator*(      type&& that)       && { return operator*=(that); }
  type&& operator*(      type&& that) const &  { return that *= *this; }

  //------- I N T E R V A L - D I V I S I O N -----------------------------------------------------------------------------------------------------------------
  
  type&& operator/=(const type& that) noexcept(nearith && ntype::necmp && nec) {
    requires(!that.contains_zero())
    if(is_empty() || that.is_empty()) return make_empty();
    return div_internal(
             that, 
             [this](const Kind new_lkind, const ntype& new_inf, const ntype& new_sup, const Kind new_rkind) -> type&& {
               this->sup_  = new_sup; 
               this->inf_  = new_inf;
               this->lkind = new_lkind;
               this->rkind = new_rkind; 
               ensures(this->invariant());
               return std::forward<type>(*this); }); }

  type operator/(const type& that) const & noexcept(nearith && ntype::necmp && nec) {
    requires(!that.contains_zero())
    if(is_empty() || that.is_empty()) return empty();
    return div_internal(
             that, 
             [](const Kind new_lkind, const ntype& new_inf, const ntype& new_sup, const Kind new_rkind) -> type {
               return type(new_lkind, new_inf, new_sup, new_rkind); }); }

  type&& operator/(const type&  that)       && { return operator/=(that); }
  type&& operator/(      type&& that)       && { return operator/=(that); }
  type&& operator/(      type&& that) const &  { return that.inverse() *= *this; }

  //----------------------------------------------------------------------------------------------------------------------------------------------------------- 
  
  type&& make_empty() noexcept(ntype::necc) {
    lkind = Kind::OPEN;
    rkind = Kind::OPEN;
    inf_ = ntype::ONE;
    sup_ = ntype::ZERO;  
    ensures(invariant());
    return std::forward<type>(*this); }
  
  type&& make_universal() noexcept(ntype::necc) {
    lkind = Kind::OPEN;
    rkind = Kind::OPEN;
    inf_ = ntype::NEG_INF;
    sup_ = ntype::POS_INF;  
    ensures(invariant());
    return std::forward<type>(*this); }
  
  type&& make_zero() noexcept(ntype::necc) {
    lkind = Kind::CLOSED;
    rkind = Kind::CLOSED;
    inf_ = ntype::ZERO;
    sup_ = ntype::ZERO;  
    ensures(invariant());
    return std::forward<type>(*this); }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  type chull(const type& that) const {
    if(     is_empty()) return  that;
    if(that.is_empty()) return *this;
    const auto lhs = is_left_unbounded () || inf() < that.inf();
    const auto rhs = is_right_unbounded() || sup() > that.sup();
    return type(lhs ? left_kind () : that.left_kind (),
                lhs ? inf       () : that.inf       (),
                rhs ? sup       () : that.sup       (),
                rhs ? right_kind() : that.right_kind()); };

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  bool invariant() const noexcept(necmp) {
    return !inf_.is_positive_infinite() &&
           !sup_.is_negative_infinite() &&
           (inf_.is_finite() || lkind == Kind::OPEN) &&
           (sup_.is_finite() || rkind == Kind::OPEN); }
  
  //------- G E T T E R S -------------------------------------------------------------------------------------------------------------------------------------
  
  const ntype&  inf() const &  noexcept { return inf_; }
  const ntype&  sup() const &  noexcept { return sup_; }
        ntype&& inf()       && noexcept { return std::move(inf_); }
        ntype&& sup()       && noexcept { return std::move(sup_); }
  constexpr Kind left_kind () const noexcept { return lkind; }
  constexpr Kind right_kind() const noexcept { return rkind; }

  //------- T O - S T R I N G ---------------------------------------------------------------------------------------------------------------------------------
  
  template<typename Out>
  Out& operator<<(Out& os) const noexcept(noexcept(os << "" << inf_)) {
    os << (lkind == Kind::CLOSED ? "[" : "(") 
       << inf_ << "," << sup_
       << (rkind == Kind::CLOSED ? "]" : ")"); 
    return os;  }

  std::string to_string() const {
    std::stringstream ss;
    operator<<(ss);
    return ss.str();  }

private:

  static constexpr Kind intersect(Kind k1, Kind k2) noexcept { return k1 == Kind::CLOSED && k2 == Kind::CLOSED ? Kind::CLOSED : Kind::OPEN; }
  
  template<typename C>
  decltype(auto) mult_internal(const type& that, const C& creator) const noexcept(nearith && ntype::necmp && nec) {
    using pair = std::tuple<ntype, Kind>;
    static const auto times     = [](const ntype& fst, const ntype& snd)-> auto { return fst.is_zero() || snd.is_zero() ? ntype::ZERO : fst * snd; };
    static const auto intersect = [](const ntype& fst, const ntype& snd, const Kind kfst, const Kind ksnd)-> Kind { 
      return (fst.is_zero()        && kfst == Kind::CLOSED) ||
             (snd.is_zero()        && ksnd == Kind::CLOSED) ||
             (kfst == Kind::CLOSED && ksnd == Kind::CLOSED) ?  Kind::CLOSED : Kind::OPEN ; };
    static const auto pair_comparator = [](const pair& fst, const pair& snd) -> bool { 
      const auto cmp = std::get<0>(fst).different_infinity(std::get<0>(snd)) ? std::get<0>(fst).compare_to(std::get<0>(snd)) : 0;
      return cmp <  0 || (cmp == 0 && std::get<1>(fst) == Kind::CLOSED && std::get<1>(snd) == Kind::OPEN); };
    std::array<pair, 4> candidates = {{
      pair(times(inf_, that.inf_), intersect(inf_, that.inf_, lkind, that.lkind)),
      pair(times(inf_, that.sup_), intersect(inf_, that.sup_, lkind, that.rkind)),
      pair(times(sup_, that.inf_), intersect(sup_, that.inf_, rkind, that.lkind)),
      pair(times(sup_, that.sup_), intersect(sup_, that.sup_, rkind, that.rkind))}};
    std::sort(candidates.begin(), candidates.end(), pair_comparator);
    const unsigned char last = !std::get<0>(candidates[3]).is_positive_infinite()        &&
                                std::get<0>(candidates[2]) == std::get<0>(candidates[3]) && 
                                std::get<1>(candidates[2]) == Kind::CLOSED               ?
                                2 : 3;
    return creator(std::get<1>(candidates[0   ]), std::get<0>(candidates[0   ]), 
                   std::get<0>(candidates[last]), std::get<1>(candidates[last])); } 
  
  template<typename C>
  decltype(auto) div_internal(const type& that, const C& creator) const noexcept(nearith && ntype::necmp && nec) {
    requires(!that.contains_zero())
    using pair = std::tuple<ntype, Kind>;
    static const auto div = [](const ntype& fst, const ntype& snd, const bool left_end)-> auto { 
      return fst.is_zero() ?  ntype::ZERO                                                     : 
             snd.is_zero() ? (left_end ^ fst.is_positive() ? ntype::NEG_INF : ntype::POS_INF) :
                              fst / snd                                                       ; };
    static const auto intersect = [](const ntype& fst, const Kind kfst, const Kind ksnd)-> Kind { 
      return (fst.is_zero()        && kfst == Kind::CLOSED) ||
             (kfst == Kind::CLOSED && ksnd == Kind::CLOSED) ?  Kind::CLOSED : Kind::OPEN ; };
    static const auto pair_comparator = [](const pair& fst, const pair& snd) -> bool { 
      const auto cmp = std::get<0>(fst).different_infinity(std::get<0>(snd)) ? std::get<0>(fst).compare_to(std::get<0>(snd)) : 0;
      return cmp <  0 || (cmp == 0 && std::get<1>(fst) == Kind::CLOSED && std::get<1>(snd) == Kind::OPEN); };
    std::array<pair, 4> candidates = {{
      pair(div(inf_, that.inf_, true ), intersect(inf_, lkind, that.lkind)),
      pair(div(inf_, that.sup_, false), intersect(inf_, lkind, that.rkind)),
      pair(div(sup_, that.inf_, true ), intersect(sup_, rkind, that.lkind)),
      pair(div(sup_, that.sup_, false), intersect(sup_, rkind, that.rkind))}};
    std::sort(candidates.begin(), candidates.end(), pair_comparator);
    const unsigned char last = !std::get<0>(candidates[3]).is_positive_infinite()        &&
                                std::get<0>(candidates[2]) == std::get<0>(candidates[3]) && 
                                std::get<1>(candidates[2]) == Kind::CLOSED               ?
                                2 : 3;
    return creator(std::get<1>(candidates[0   ]), std::get<0>(candidates[0   ]), 
                   std::get<0>(candidates[last]), std::get<1>(candidates[last])); } 

};

template<typename Out, typename N, typename CP> 
Out& operator<<(Out& os, const continuous_dynamic_interval<N,CP>& interval) noexcept(noexcept(interval << os)) { return interval << os; }

template<typename N, typename CP> bool operator< (pbv_t<N> fst, const continuous_dynamic_interval<N,CP>& snd) noexcept(noexcept(snd >  fst)) {return snd >  fst;}
template<typename N, typename CP> bool operator<=(pbv_t<N> fst, const continuous_dynamic_interval<N,CP>& snd) noexcept(noexcept(snd >= fst)) {return snd >= fst;}
template<typename N, typename CP> bool operator> (pbv_t<N> fst, const continuous_dynamic_interval<N,CP>& snd) noexcept(noexcept(snd <  fst)) {return snd <  fst;}
template<typename N, typename CP> bool operator>=(pbv_t<N> fst, const continuous_dynamic_interval<N,CP>& snd) noexcept(noexcept(snd <= fst)) {return snd <= fst;}
template<typename N, typename CP> bool operator< (const extended_number<N,CP>&fst, const continuous_dynamic_interval<N,CP>&snd) noexcept(noexcept(snd> fst)){return snd> fst;}
template<typename N, typename CP> bool operator<=(const extended_number<N,CP>&fst, const continuous_dynamic_interval<N,CP>&snd) noexcept(noexcept(snd>=fst)){return snd>=fst;}
template<typename N, typename CP> bool operator> (const extended_number<N,CP>&fst, const continuous_dynamic_interval<N,CP>&snd) noexcept(noexcept(snd< fst)){return snd< fst;}
template<typename N, typename CP> bool operator>=(const extended_number<N,CP>&fst, const continuous_dynamic_interval<N,CP>&snd) noexcept(noexcept(snd<=fst)){return snd<=fst;}

} // namespace cmn::math

#endif // CMN__MATH__CONTINUOUS_DYNAMIC_INTERVAL__HPP
