#ifndef CMN__MPL__ARITH__HPP
#define CMN__MPL__ARITH__HPP

#include <boost/mpl/bool.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/integral_c.hpp>
#include <boost/mpl/plus.hpp>
#include <boost/mpl/times.hpp>

#include <cstdint>
#include <limits>
#include <type_traits>

namespace cmn {
namespace mpl {

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename fst, typename snd>
struct is_safe_plus {
private:
  using fst_t = decltype(fst::value);
  using snd_t = decltype(snd::value);
  using both_small = boost::mpl::bool_<(sizeof(fst_t) < sizeof(int)) && (sizeof(snd_t) < sizeof(int))>;
public:
  static_assert(std::is_integral<fst_t>::value, "only integral types are supported");
  static_assert(std::is_integral<snd_t>::value, "only integral types are supported");
  static_assert(both_small::value || std::is_unsigned<fst_t>::value, "current implementation only support the case where both operands are unsigned or both are smaller than int");
  static_assert(both_small::value || std::is_unsigned<snd_t>::value, "current implementation only support the case where both operands are unsigned or both are smaller than int");
  using type = boost::mpl::bool_<(fst::value + snd::value >= fst::value)>;
};

template<typename fst, typename snd>
using is_safe_plus_t = typename is_safe_plus<fst, snd>::type;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename...ops> struct safe_plus { };

template<>                                            struct safe_plus<>                  : boost::mpl::integral_c<std::uint8_t, 0>::type { };
template<typename op>                                 struct safe_plus<op>                : op { };
template<typename fst, typename snd, typename...rest> struct safe_plus<fst, snd, rest...> : safe_plus<typename safe_plus<fst, snd>::type, rest...>::type { };

template<typename fst, typename snd> struct safe_plus<fst, snd> {
  static_assert(is_safe_plus_t<fst, snd>::value, "unsafe plus (most likely overflow error)");
  using type = typename boost::mpl::plus<fst, snd>::type;
};

/** unfortunately <tt>Placeholder Expression</tt> mechanism does not support variadic template argument. So sometimes meta-function class of \c safe_plus is 
  * necessary.*/
struct safe_plus_f {
  template<typename...ops>
  struct apply : safe_plus<ops...>::type { };
};

template<typename num>
using safe_inc_t = 
  typename safe_plus<
    num, 
    boost::mpl::integral_c<
      typename boost::mpl::if_<
        std::is_signed<decltype(num::value)>,
        std::int8_t,
        std::uint8_t
      >::type,
      1>
  >::type;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename fst, typename snd>
struct is_safe_times {
private:
  using both_unsigned = boost::mpl::bool_<std::is_unsigned  <decltype(fst::value)>::value && std::is_unsigned  <decltype(snd::value)>::value>;
  using both_signed   = boost::mpl::bool_<std::is_signed    <decltype(fst::value)>::value && std::is_signed    <decltype(snd::value)>::value>;
  using both_char     = boost::mpl::bool_<std::is_same<char, decltype(fst::value)>::value && std::is_same<char, decltype(snd::value)>::value>;
public:
  static_assert(std::is_integral<decltype(fst::value)>::value, "only integral types are supported");
  static_assert(std::is_integral<decltype(snd::value)>::value, "only integral types are supported");
  static_assert(both_char::value || both_signed::value || both_unsigned::value, 
                "current implementation is defined only for the cases where both operands are signed, both are unsigned, or they are both are char");
  using type = typename boost::mpl::bool_<
  (!both_unsigned::value || fst::value == 0 || std::numeric_limits<decltype(fst::value * snd::value)>::max() / fst::value >= snd::value) &&
  (!both_signed::value || fst::value == 0 || snd::value == 0 || (
    (!(fst::value > 0 && snd::value > 0) || std::numeric_limits<decltype(fst::value * snd::value)>::max() / fst::value >= snd::value) &&
    (!(fst::value < 0 && snd::value < 0) || std::numeric_limits<decltype(fst::value * snd::value)>::max() / fst::value <= snd::value) &&
    (!(fst::value > 0 && snd::value < 0) || std::numeric_limits<decltype(fst::value * snd::value)>::min() / fst::value <= snd::value) &&
    (!(fst::value < 0 && snd::value > 0) || std::numeric_limits<decltype(fst::value * snd::value)>::min() / snd::value <= fst::value)))
    >::type;
};

template<typename fst, typename snd> 
using is_safe_times_t = typename is_safe_times<fst, snd>::type;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename...ops> struct safe_times { };

template<>                                            struct safe_times<>                  : boost::mpl::integral_c<std::uint8_t, 1>::type { };
template<typename op>                                 struct safe_times<op>                : op { };
template<typename fst, typename snd, typename...rest> struct safe_times<fst, snd, rest...> : safe_times<typename safe_times<fst, snd>::type, rest...>::type { };

template<typename fst, typename snd> struct safe_times<fst, snd> {
  static_assert(is_safe_times_t<fst, snd>::value, "unsafe times (most likely overflow error)");
  using type = typename boost::mpl::times<fst, snd>::type;
};

/** unfortunately <tt>Placeholder Expression</tt> mechanism does not support variadic template argument. So sometimes meta-function class of \c safe_times is 
  * necessary.*/
struct safe_times_f {
  template<typename...ops>
  struct apply : safe_times<ops...>::type { };
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------


} // namespace cmn
} // namespace mpl

#endif // CMN__MPL__ARITH__HPP

