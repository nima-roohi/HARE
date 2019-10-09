#ifndef CMN__MATH__EXTENDED_NUMBER__HPP
#define CMN__MATH__EXTENDED_NUMBER__HPP

#include "cmn/dbc.hpp"
#include "cmn/type_traits.hpp"  // pbv_t

#include <utility>              // move, forward

namespace cmn::math {

template<typename N>
struct constant_provider {
  static const N ZERO;
  static const N ONE ;
};
template<typename N> const N constant_provider<N>::ZERO = static_cast<N>(0);
template<typename N> const N constant_provider<N>::ONE  = static_cast<N>(1);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

enum class extended_number_kind : unsigned char { FINITE, POS_INF, NEG_INF };

template<typename Out>
inline Out& operator<<(Out& os, const extended_number_kind kind) {
  switch(kind) {
  case extended_number_kind::FINITE  : { os << "finite"            ; return os; }
  case extended_number_kind::POS_INF : { os << "positive_infinity" ; return os; }
  case extended_number_kind::NEG_INF : { os << "negative_infinity" ; return os; }
  }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename N,
         typename CP = constant_provider<N>>
struct extended_number {

  using type = extended_number;
  using Kind = extended_number_kind;

  static constexpr const bool necc = noexcept_copy_constructor_t<N>::value;
  static constexpr const bool nemc = noexcept_move_constructor_t<N>::value;
  static constexpr const bool nec  = necc && nemc;
  static constexpr const bool necmp = noexcept(std::declval<N>() <  std::declval<N>()) &&
                                      noexcept(std::declval<N>() >  std::declval<N>()) &&
                                      noexcept(std::declval<N>() <= std::declval<N>()) &&
                                      noexcept(std::declval<N>() >= std::declval<N>()) &&
                                      noexcept(std::declval<N>() == std::declval<N>()) &&
                                      noexcept(std::declval<N>() != std::declval<N>()) ;
  static const type ZERO;
  static const type ONE ;
  static const type POS_INF;
  static const type NEG_INF;

private:
  Kind kind_ ;
  N    value_;

public:

  //------- C O N S T R U C T O R E S -------------------------------------------------------------------------------------------------------------------------

  template<typename M>
  extended_number(Kind kind, M&& value, typename std::enable_if<!std::is_fundamental<M>::value>::type*) noexcept(nemc): kind_(kind), value_(std::move(value)) { 
    static_assert(std::is_same<N,M>::value);  
    requires_msg(kind == Kind::FINITE, "Invalid kind: " << kind)
  }

  template<typename M> 
  extended_number(M && value, typename std::enable_if<!std::is_fundamental<M>::value>::type*) noexcept(nemc): kind_(Kind::FINITE), value_(std::move(value)) { 
    static_assert(std::is_same<N,M>::value);  }

  explicit extended_number(Kind kind, pbv_t<N> value) noexcept(necc): kind_(kind)        , value_(value   ) { requires_msg(kind == Kind::FINITE, "Invalid kind: " << kind) }
  explicit extended_number(Kind kind                ) noexcept(necc): kind_(kind)        , value_(CP::ZERO) { requires_msg(kind != Kind::FINITE, "Invalid kind: " << kind) }
  explicit extended_number(                         ) noexcept(necc): kind_(Kind::FINITE), value_(CP::ZERO) { }
           extended_number(           pbv_t<N> value) noexcept(necc): kind_(Kind::FINITE), value_(value   ) { }

  //------- D E F A U L T - M E T H O D S ---------------------------------------------------------------------------------------------------------------------
  
  extended_number(const extended_number& ) = default; 
  extended_number(      extended_number&&) = default; 
  extended_number& operator=(const extended_number& ) = default; 
  extended_number& operator=(      extended_number&&) = default; 
  
  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  constexpr bool is_finite           () const noexcept { return kind_ == Kind::FINITE ; }
  constexpr bool is_infinite         () const noexcept { return kind_ != Kind::FINITE ; }
  constexpr bool is_negative_infinite() const noexcept { return kind_ == Kind::NEG_INF; }
  constexpr bool is_positive_infinite() const noexcept { return kind_ == Kind::POS_INF; }
  bool is_positive() const noexcept(necmp) { return kind_ == Kind::POS_INF || value_ >  CP::ZERO; }
  bool is_negative() const noexcept(necmp) { return kind_ == Kind::NEG_INF || value_ <  CP::ZERO; }
  bool is_zero    () const noexcept(necmp) { return kind_ == Kind::FINITE  && value_ == CP::ZERO; }
  bool is_nonzero () const noexcept(necmp) { return kind_ != Kind::FINITE  || value_ != CP::ZERO; }
  bool is_one     () const noexcept(necmp) { return kind_ == Kind::FINITE  && value_ == CP::ONE ; }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------
 
  /** @return \c true iff at least one of \c this or \c that is finite, or if they are both infinite then they are both positive or both negative. */
  constexpr bool same_infinity(const type& that) const noexcept {
    return (kind_ != Kind::POS_INF || that.kind_ != Kind::NEG_INF) &&
           (kind_ != Kind::NEG_INF || that.kind_ != Kind::POS_INF); }

  /** @return \c true iff at least one of \c this or \c that is finite, or if they are both infinite then they have oppsite sign. */
  constexpr bool different_infinity(const type& that) const noexcept {
    return (kind_ != Kind::POS_INF || that.kind_ != Kind::POS_INF) &&
           (kind_ != Kind::NEG_INF || that.kind_ != Kind::NEG_INF); }

  //------- O R D E R I N G - M E T H O D S -------------------------------------------------------------------------------------------------------------------
  
  signed char compare_to(pbv_t<N> that) const noexcept(necmp) {
    return kind_ == Kind::NEG_INF ? -1 :
           kind_ == Kind::POS_INF ? +1 :
           value_ <  that         ? -1 :
           value_ == that         ?  0 : 
                                     1 ;  }

  signed char compare_to(const type& that) const noexcept(necmp) { 
    requires(different_infinity(that))
    return that.kind_ == Kind::NEG_INF ? +1 : 
           that.kind_ == Kind::POS_INF ? -1 :
           compare_to(that.value_);  }

  bool operator< (pbv_t<N> that) const noexcept(necmp) { return kind_ == Kind::NEG_INF || (kind_ == Kind::FINITE && value_ <  that); }
  bool operator> (pbv_t<N> that) const noexcept(necmp) { return kind_ == Kind::POS_INF || (kind_ == Kind::FINITE && value_ >  that); }
  bool operator<=(pbv_t<N> that) const noexcept(necmp) { return kind_ == Kind::NEG_INF || (kind_ == Kind::FINITE && value_ <= that); }
  bool operator>=(pbv_t<N> that) const noexcept(necmp) { return kind_ == Kind::POS_INF || (kind_ == Kind::FINITE && value_ >= that); }
  bool operator==(pbv_t<N> that) const noexcept(necmp) { return kind_ == Kind::FINITE  && value_ == that; }
  bool operator!=(pbv_t<N> that) const noexcept(necmp) { return kind_ != Kind::FINITE  || value_ != that; }

  bool operator< (const type& that) const noexcept(necmp) { 
    requires(different_infinity(that)) 
    return  that.kind_ == Kind::POS_INF ||
           (that.kind_ != Kind::NEG_INF && operator< (that.value_)); }

  bool operator> (const type& that) const noexcept(necmp) { 
    requires(different_infinity(that)) 
    return  that.kind_ == Kind::NEG_INF ||
           (that.kind_ != Kind::POS_INF && operator> (that.value_)); }

  bool operator<=(const type& that) const noexcept(necmp) { 
    requires(different_infinity(that)) 
    return  that.kind_ == Kind::POS_INF || 
           (that.kind_ != Kind::NEG_INF && operator<=(that.value_)); }

  bool operator>=(const type& that) const noexcept(necmp) { 
    requires(different_infinity(that)) 
    return  that.kind_ == Kind::NEG_INF || 
           (that.kind_ != Kind::POS_INF && operator>=(that.value_)); }

  bool operator==(const type& that) const noexcept(necmp) { 
    requires(different_infinity(that)) 
    return  kind_ == that.kind_ && value_ == that.value_; }

  bool operator!=(const type& that) const noexcept(necmp) { 
    requires(different_infinity(that)) 
    return kind_ != that.kind_ || value_ != that.value_; }

  //------- N E G A T I O N ----------------------------------------------------------------------------------------------------------------------------------- 

  type&& negate() noexcept(noexcept(value_ = std::move(-std::move(value_)))) { 
    switch(kind_) {
      case Kind::POS_INF: kind_  = Kind::NEG_INF; break;
      case Kind::NEG_INF: kind_  = Kind::POS_INF; break;
      default           : value_ = std::move(-std::move(value_)); }
    return std::forward<type>(*this); }

  type operator-() const &  noexcept(nec && noexcept(type(-value_))) { 
    return kind_ == Kind::POS_INF ? NEG_INF :
           kind_ == Kind::NEG_INF ? POS_INF :
           type(-value_); }

  type   operator-() const && { return this->operator-(); } 
  type&& operator-()       && { return negate(); }

  //------- I N V E R S E ------------------------------------------------------------------------------------------------------------------------------------- 
  
  type inverse() const noexcept(nec && noexcept(type(CP::ONE/value_))) { 
    if(kind_ != Kind::FINITE) return ZERO;
    return type(CP::ONE/value_); }

  type&& inverse() noexcept(necc && noexcept(value_ = CP::ONE / value_)) { 
    if(kind_ != Kind::FINITE) {
      kind_  = Kind::FINITE;
      value_ = CP::ZERO; }
    else value_ = CP::ONE / std::move(value_);
    return std::forward<type>(*this); }
  
  //------- A D D I T I O N - W I T H - N ---------------------------------------------------------------------------------------------------------------------
  
  type&& operator+=(pbv_t<N> that)          { if(kind_ == Kind::FINITE) value_ += that; return std::forward<type>(*this); }
  type   operator+ (pbv_t<N> that) const &  { return kind_ != Kind::FINITE ? *this : type(value_ + that); }
  type   operator+ (pbv_t<N> that) const && { return this->operator+ (that); }
  type&& operator+ (pbv_t<N> that)       && { return this->operator+=(that); }
  
  //------- S U B T R A C T I O N - W I T H - N ---------------------------------------------------------------------------------------------------------------
  
  type&& operator-=(pbv_t<N> that)          { if(kind_ == Kind::FINITE) value_ -= that; return std::forward<type>(*this); }
  type   operator- (pbv_t<N> that) const &  { return kind_ != Kind::FINITE ? *this : type(value_ - that); }
  type   operator- (pbv_t<N> that) const && { return this->operator- (that); }
  type&& operator- (pbv_t<N> that)       && { return this->operator-=(that); }
  
  //------- M U L T I P L I C A T I O N - W I T H - N ---------------------------------------------------------------------------------------------------------

  type&& operator*=(pbv_t<N> that) noexcept(noexcept(value_ *= that) && necmp) { 
    requires(kind_ == Kind::FINITE || that != CP::ZERO)
    if(kind_ != Kind::FINITE) {
      if(that < CP::ZERO)
        kind_ = kind_ == Kind::POS_INF ? Kind::NEG_INF : Kind::POS_INF;
    } else value_ *= that;
    return std::forward<type>(*this); }
  
  type operator*(pbv_t<N> that) const & noexcept(nec && necmp && noexcept(type(value_ * that))) { 
    requires(kind_ == Kind::FINITE || that != CP::ZERO)
    return kind_ == Kind::POS_INF ? (that > CP::ZERO ? POS_INF : NEG_INF) :
           kind_ == Kind::NEG_INF ? (that < CP::ZERO ? POS_INF : NEG_INF) :
           type(value_ * that); }

  type   operator*(pbv_t<N> that) const && { return this->operator* (that); }
  type&& operator*(pbv_t<N> that)       && { return this->operator*=(that); }

  //------- D I V I S I O N - W I T H - N ---------------------------------------------------------------------------------------------------------------------
  
  type&& operator/=(pbv_t<N> that) { 
    requires(that != CP::ZERO)
    if(kind_ != Kind::FINITE) {
      if(that < CP::ZERO)
        kind_ = kind_ == Kind::POS_INF ? Kind::NEG_INF : Kind::POS_INF;
    } else value_ /= that;
    return std::forward<type>(*this); }

  type operator/(pbv_t<N> that) const &  { 
    requires(that != CP::ZERO)
    return kind_ == Kind::POS_INF ? (that > CP::ZERO ? POS_INF : NEG_INF) :
           kind_ == Kind::NEG_INF ? (that < CP::ZERO ? POS_INF : NEG_INF) :
           type(value_ / that); }

  type   operator/(pbv_t<N> that) const && { return this->operator/ (that); }
  type&& operator/(pbv_t<N> that)       && { return this->operator/=(that); }

  //------- A D D I T I O N - W I T H - T Y P E ---------------------------------------------------------------------------------------------------------------
  
  type&& operator+=(const type& that) { 
    requires(same_infinity(that))
    if(that.kind_ == Kind::FINITE) 
      return operator+=(that.value_);
    kind_ = that.kind_;
    return std::forward<type>(*this); }

  type   operator+(const type&  that) const & { 
    requires(same_infinity(that))
    return that.kind_ == Kind::FINITE ? operator+(that.value_) : that; }
  
  type&& operator+(const type&  that)       && { return operator+=(that); }
  type&& operator+(      type&& that)       && { return operator+=(that); }
  type&& operator+(      type&& that) const &  { return that += *this;    }

  //------- S U B T R A C T I O N - W I T H - T Y P E ---------------------------------------------------------------------------------------------------------

  type&& operator-=(const type& that) { 
    requires(different_infinity(that))
    if(that.kind_ != Kind::FINITE) {
      kind_ = that.kind_ == Kind::POS_INF ? Kind::NEG_INF : Kind::POS_INF;
      return std::forward<type>(*this); }
    return operator-=(that.value_); }

  type   operator-(const type&  that) const & { 
    requires(different_infinity(that))
    return that.kind_ != Kind::FINITE ? -that : operator-(that.value_); }

  type&& operator-(const type&  that)       && { return operator-=(that); }
  type&& operator-(      type&& that)       && { return operator-=(that); }
  type&& operator-(      type&& that) const &  { return that.negate() += *this; }

  //------- M U L T I P L I C A T I O N - W I T H - T Y P E --------------------------------------------------------------------------------------------------- 

  type&& operator*=(const type& that) { 
    requires(kind_ == Kind::FINITE || !that.is_zero())
    switch(that.kind_) {
      case Kind::POS_INF : kind_ = is_positive() ? Kind::POS_INF : Kind::NEG_INF; break;
      case Kind::NEG_INF : kind_ = is_negative() ? Kind::POS_INF : Kind::NEG_INF; break;
      default            : return operator*=(that.value_); }
    return std::forward<type>(*this); }

  type   operator*(const type&  that) const &  noexcept(nec && necmp && noexcept(operator*(that.value_))) { 
    requires(     kind_ == Kind::FINITE || !that.is_zero())
    requires(that.kind_ == Kind::FINITE || !     is_zero())
    return that.kind_ == Kind::POS_INF ? (is_positive() ? POS_INF : NEG_INF) :
           that.kind_ == Kind::NEG_INF ? (is_negative() ? POS_INF : NEG_INF) :
           operator*(that.value_); }

  type&& operator*(const type&  that)       && { return operator*=(that); }
  type&& operator*(      type&& that)       && { return operator*=(that); }
  type&& operator*(      type&& that) const &  { return that *= *this; }

  //------- D I V I S I O N - W I T H - T Y P E --------------------------------------------------------------------------------------------------------------- 
  
  type&& operator/=(const type& that) { 
    requires(!that.is_zero())
    requires(kind_ == Kind::FINITE || that.kind_ == Kind::FINITE)
    if(that.kind_ != Kind::FINITE) return std::forward<type>(operator=(ZERO));
    return operator/=(that.value_); }

  type   operator/(const type&  that) const & { 
    requires(!that.is_zero())
    requires(kind_ == Kind::FINITE || that.kind_ == Kind::FINITE)
    return that.kind_ != Kind::FINITE ? ZERO : operator/(that.value_); }

  type&& operator/(const type&  that)       && { return operator/=(that); }
  type&& operator/(      type&& that)       && { return operator/=(that); }
  type&& operator/(      type&& that) const &  { return that.inverse() *= *this; }

  //------- G E T T E R S -------------------------------------------------------------------------------------------------------------------------------------
  
  constexpr Kind     kind () const    noexcept { return kind_ ; }
            pbv_t<N> value() const &  noexcept { return value_; }
            N&&      value()       && noexcept { return std::move(value_); }
  
  //------- T O - S T R I N G ---------------------------------------------------------------------------------------------------------------------------------
  
  template<typename Out>
  Out& operator<<(Out& os) const noexcept(noexcept(os << "-inf" << value_)) {
    switch(kind_) {
      case Kind::POS_INF : os <<  "inf"; return os;
      case Kind::NEG_INF : os << "-inf"; return os;
      default            : os << value_; return os; } }

};
template<typename N, typename CP> const extended_number<N,CP> extended_number<N,CP>::ZERO    = extended_number<N,CP>(CP::ZERO);
template<typename N, typename CP> const extended_number<N,CP> extended_number<N,CP>::ONE     = extended_number<N,CP>(CP::ONE );
template<typename N, typename CP> const extended_number<N,CP> extended_number<N,CP>::POS_INF = extended_number<N,CP>(extended_number<N,CP>::Kind::POS_INF);
template<typename N, typename CP> const extended_number<N,CP> extended_number<N,CP>::NEG_INF = extended_number<N,CP>(extended_number<N,CP>::Kind::NEG_INF);

template<typename Out, typename N, typename CP> 
Out& operator<<(Out& os, const extended_number<N,CP>& value) noexcept(noexcept(value << os)) { return value << os; }

template<typename N, typename CP> bool operator< (pbv_t<N> fst, const extended_number<N,CP>& snd) noexcept(noexcept(snd >  fst)) { return snd >  fst; }
template<typename N, typename CP> bool operator<=(pbv_t<N> fst, const extended_number<N,CP>& snd) noexcept(noexcept(snd >= fst)) { return snd >= fst; }
template<typename N, typename CP> bool operator> (pbv_t<N> fst, const extended_number<N,CP>& snd) noexcept(noexcept(snd <  fst)) { return snd <  fst; }
template<typename N, typename CP> bool operator>=(pbv_t<N> fst, const extended_number<N,CP>& snd) noexcept(noexcept(snd <= fst)) { return snd <= fst; }
template<typename N, typename CP> bool operator==(pbv_t<N> fst, const extended_number<N,CP>& snd) noexcept(noexcept(snd == fst)) { return snd == fst; }
template<typename N, typename CP> bool operator!=(pbv_t<N> fst, const extended_number<N,CP>& snd) noexcept(noexcept(snd != fst)) { return snd != fst; }

} // namespace cmn::math

#endif // CMN__MATH__EXTENDED_NUMBER__HPP
