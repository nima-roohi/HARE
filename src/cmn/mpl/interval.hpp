#ifndef CMN__MPL__INTERVAL__HPP
#define CMN__MPL__INTERVAL__HPP

#include "cmn/mpl/dec.hpp"
#include "cmn/mpl/inc.hpp"
#include "cmn/mpl/is_zero.hpp"
#include "cmn/mpl/zero_of.hpp"

#include <boost/mpl/and.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/integral_c.hpp>
#include <boost/mpl/integral_c_tag.hpp>
#include <boost/mpl/less.hpp>
#include <boost/mpl/less_equal.hpp>
#include <boost/mpl/list.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/pair.hpp>
#include <boost/mpl/plus.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/sort.hpp>
#include <boost/mpl/times.hpp>

#include <cstdint>
#include <type_traits>

namespace cmn::mpl {

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

struct interval_tag;

template<bool lclosed_, typename inf_, typename sup_, bool rclosed_>
struct interval {
  static_assert(std::is_same<typename inf_::value_type, typename sup_::value_type>::value, "value-type of infimum and supremum must be the same");
  using type  = interval;
  using tag   = interval_tag;
  using inf   = inf_;
  using sup   = sup_;
  static constexpr bool lclosed = lclosed_;
  static constexpr bool rclosed = rclosed_;
  using left_open    = boost::mpl::bool_<!lclosed>; 
  using left_closed  = boost::mpl::bool_< lclosed>; 
  using right_open   = boost::mpl::bool_<!rclosed>; 
  using right_closed = boost::mpl::bool_< rclosed>; 
};
template<bool lclosed, typename inf, typename sup, bool rclosed> constexpr bool interval<lclosed,inf,sup,rclosed>::lclosed;
template<bool lclosed, typename inf, typename sup, bool rclosed> constexpr bool interval<lclosed,inf,sup,rclosed>::rclosed;

template<typename inf, typename sup> using closed_interval       = interval<true , inf, sup, true >;
template<typename inf, typename sup> using open_interval         = interval<false, inf, sup, false>;
template<typename inf, typename sup> using left_open_interval    = interval<false, inf, sup, true >;
template<typename inf, typename sup> using left_closed_interval  = interval<true , inf, sup, false>;
template<typename inf, typename sup> using right_open_interval   = interval<true , inf, sup, false>;
template<typename inf, typename sup> using right_closed_interval = interval<false, inf, sup, true >;
template<typename num> using singleton_interval = interval<true, num, num, true>;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename interval> struct is_interval : boost::mpl::false_ { };
template<typename interval> struct inf_of          { };
template<typename interval> struct sup_of          { };
template<typename interval> struct is_left_closed  { };
template<typename interval> struct is_right_closed { };
template<typename interval> struct is_left_open    { };
template<typename interval> struct is_right_open   { };

template<bool lclosed, typename inf, typename sup, bool rclosed> struct is_interval<interval<lclosed,inf,sup,rclosed>> : boost::mpl::true_ { };
template<bool lclosed, typename inf, typename sup, bool rclosed> struct inf_of         <interval<lclosed,inf,sup,rclosed>> : inf { };
template<bool lclosed, typename inf, typename sup, bool rclosed> struct sup_of         <interval<lclosed,inf,sup,rclosed>> : sup { };
template<bool lclosed, typename inf, typename sup, bool rclosed> struct is_left_open   <interval<lclosed,inf,sup,rclosed>> : boost::mpl::bool_<!lclosed> { };
template<bool lclosed, typename inf, typename sup, bool rclosed> struct is_left_closed <interval<lclosed,inf,sup,rclosed>> : boost::mpl::bool_< lclosed> { };
template<bool lclosed, typename inf, typename sup, bool rclosed> struct is_right_open  <interval<lclosed,inf,sup,rclosed>> : boost::mpl::bool_<!rclosed> { };
template<bool lclosed, typename inf, typename sup, bool rclosed> struct is_right_closed<interval<lclosed,inf,sup,rclosed>> : boost::mpl::bool_< rclosed> { };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename interval, typename enabled = void>
struct is_empty_interval { };

template<typename inf, typename sup>
struct is_empty_interval<interval<true,inf,sup,true>, std::enable_if_t<std::is_integral<typename inf::value_type>::value>> : 
  boost::mpl::less<sup, inf> { };

template<typename inf, typename sup>
struct is_empty_interval<interval<true,inf,sup,false>, std::enable_if_t<std::is_integral<typename inf::value_type>::value>> : 
  boost::mpl::less_equal<sup, inf> { };

template<typename inf, typename sup>
struct is_empty_interval<interval<false,inf,sup,true>, std::enable_if_t<std::is_integral<typename inf::value_type>::value>> : 
  boost::mpl::less_equal<sup, inf> { };

template<typename inf, typename sup>
struct is_empty_interval<interval<false,inf,sup,false>, std::enable_if_t<std::is_integral<typename inf::value_type>::value>> : 
  boost::mpl::or_<
    boost::mpl::less_equal<sup, inf>,
    boost::mpl::bool_<inf::value + 1 == sup::value>
  > { };

template<typename inf, typename sup>
struct is_empty_interval<interval<true,inf,sup,true>, std::enable_if_t<std::is_floating_point<typename inf::value_type>::value>> : 
  boost::mpl::less<sup, inf> { };

template<typename inf, typename sup>
struct is_empty_interval<interval<true,inf,sup,false>, std::enable_if_t<std::is_floating_point<typename inf::value_type>::value>> : 
  boost::mpl::less_equal<sup, inf> { };

template<typename inf, typename sup>
struct is_empty_interval<interval<false,inf,sup,true>, std::enable_if_t<std::is_floating_point<typename inf::value_type>::value>> : 
  boost::mpl::less_equal<sup, inf> { };

template<typename inf, typename sup>
struct is_empty_interval<interval<false,inf,sup,false>, std::enable_if_t<std::is_floating_point<typename inf::value_type>::value>> : 
  boost::mpl::less_equal<sup, inf> { };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename interval, typename enabled = void>
struct interval_size { };

template<typename inf, typename sup>
struct interval_size<interval<true,inf,sup,true>, std::enable_if_t<std::is_integral<typename inf::value_type>::value>> : 
  boost::mpl::eval_if<
    is_empty_interval<interval<true,inf,sup,true>>,
    zero_of<typename inf::value_type>,
    boost::mpl::integral_c<decltype(sup::value - inf::value + 1), sup::value - inf::value + 1>
  >::type { };

template<typename inf, typename sup>
struct interval_size<interval<true,inf,sup,false>, std::enable_if_t<std::is_integral<typename inf::value_type>::value>> : 
  boost::mpl::eval_if<
    is_empty_interval<interval<true,inf,sup,false>>,
    zero_of<typename inf::value_type>,
    boost::mpl::integral_c<decltype(sup::value - inf::value), sup::value - inf::value>
  >::type { };

template<typename inf, typename sup>
struct interval_size<interval<false,inf,sup,true>, std::enable_if_t<std::is_integral<typename inf::value_type>::value>> : 
  boost::mpl::eval_if<
    is_empty_interval<interval<false,inf,sup,true>>,
    zero_of<typename inf::value_type>,
    boost::mpl::integral_c<decltype(sup::value - inf::value), sup::value - inf::value>
  >::type { };

template<typename inf, typename sup>
struct interval_size<interval<false,inf,sup,false>, std::enable_if_t<std::is_integral<typename inf::value_type>::value>> : 
  boost::mpl::eval_if<
    is_empty_interval<interval<false,inf,sup,false>>,
    zero_of<typename inf::value_type>,
    boost::mpl::integral_c<decltype(sup::value - inf::value - 1), sup::value - inf::value - 1>
  >::type { };

template<typename I>
struct interval_size<I, std::enable_if_t<std::is_floating_point<typename inf_of<I>::type::value_type>::value>> {
  using value_type = typename inf_of<I>::type;
  static constexpr value_type value = sup_of<I>::type::value - inf_of<I>::type::value;
};
template<typename I>
constexpr typename inf_of<I>::type interval_size<I, std::enable_if_t<std::is_floating_point<typename inf_of<I>::type::value_type>::value>>::value;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** The smaller lower bound comes first. If lower bounds are equal, the closed bound comes first. 
  * Each element is pair of meta-number and boolean meta-value for closeness. */
struct pair_lb_less {
  template<typename lft, typename rgt>
  struct apply : boost::mpl::or_<
                   boost::mpl::less<typename lft::first, typename rgt::first>,
                   boost::mpl::and_<
                     std::is_same<typename lft::first, typename rgt::first>,
                     boost::mpl::bool_<lft::second::value && !rgt::second::value>
                   >
                 > { }; };

/** The larger upper bound comes first. If upper bounds are equal, the closed bound comes first. 
  * Each element is pair of meta-number and boolean meta-value for closeness. */
struct pair_ub_less {
  template<typename lft, typename rgt>
  struct apply : boost::mpl::or_<
                   boost::mpl::less<typename rgt::first, typename lft::first>,
                   boost::mpl::and_<
                     std::is_same<typename lft::first, typename rgt::first>,
                     boost::mpl::bool_<lft::second::value && !rgt::second::value>
                   >
                 > { }; };

/** The smaller lower bound comes first. If lower bounds are equal, the closed bound comes first. 
  * Each element is an interval. */
struct interval_lb_less {
  template<typename lft, typename rgt>
  struct apply : 
    pair_lb_less::apply<
      boost::mpl::pair<
        typename inf_of        <lft>::type,
        typename is_left_closed<lft>::type>,
      boost::mpl::pair<
        typename inf_of        <rgt>::type,
        typename is_left_closed<rgt>::type>
    > { }; };

/** The larger upper bound comes first. If upper bounds are equal, the closed bound comes first. 
  * Each element is an interval. */
struct interval_ub_less {
  template<typename lft, typename rgt>
  struct apply : 
    pair_ub_less::apply<
      boost::mpl::pair<
        typename sup_of         <lft>::type,
        typename is_right_closed<lft>::type>,
      boost::mpl::pair<
        typename sup_of         <rgt>::type,
        typename is_right_closed<rgt>::type>
    > { }; };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename I>
struct close_if_nonempty_integral_impl { 
  template<int dummy>
  struct impl : interval<
                  true,
                  typename boost::mpl::eval_if<
                    is_left_closed<I>,
                    inf_of<I>,
                    inc<typename inf_of<I>::type>
                  >::type,
                  typename boost::mpl::eval_if<
                    is_right_closed<I>,
                    sup_of<I>,
                    dec<typename sup_of<I>::type>
                  >::type,
                  true> { };
  using type = typename boost::mpl::eval_if<
                 boost::mpl::and_<
                   std::is_integral<typename inf_of<I>::type::value_type>,
                   boost::mpl::not_<is_empty_interval<I>>>,
                 impl<1>,
                 I
               >::type;
};

template<typename I>
struct close_if_nonempty_integral : close_if_nonempty_integral_impl<I> { };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** @note nither \c fst not \c snd can be empty. */
template<typename fst_, typename snd_>
struct is_interval_subset_impl { 
  using fst  = typename fst_::type;
  using snd  = typename snd_::type;
  using type = typename boost::mpl::and_<
                 boost::mpl::less_equal<typename inf_of<snd>::type, typename inf_of<fst>::type>,
                 boost::mpl::less_equal<typename sup_of<fst>::type, typename sup_of<snd>::type>,
                 boost::mpl::or_<
                   is_left_open  <fst>,
                   is_left_closed<snd>,
                   boost::mpl::less<typename inf_of<snd>::type, typename inf_of<fst>::type>>,
                 boost::mpl::or_<
                   is_right_open  <fst>,
                   is_right_closed<snd>,
                   boost::mpl::less<typename sup_of<fst>::type, typename sup_of<snd>::type>>
               >::type;
};

template<typename fst, typename snd>
struct is_interval_subset : 
  boost::mpl::eval_if<
    is_empty_interval<fst>,
    boost::mpl::true_,
    boost::mpl::eval_if<
      is_empty_interval<snd>,
      boost::mpl::false_,
      is_interval_subset_impl<
        close_if_nonempty_integral<fst>, 
        close_if_nonempty_integral<snd>>
    >
  >::type { };

template<typename fst, typename snd>
struct is_interval_supset : is_interval_subset<snd, fst> { };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename fst, typename snd>
struct binary_interval_plus {
  using inf = typename boost::mpl::plus<typename inf_of<fst>::type, typename inf_of<snd>::type>::type;
  using sup = typename boost::mpl::plus<typename sup_of<fst>::type, typename sup_of<snd>::type>::type;
  using type = interval<
                 is_left_closed <fst>::value && is_left_closed <snd>::value,
                 inf,
                 sup,
                 is_right_closed<fst>::value && is_right_closed<snd>::value   
               >;
};

template<typename...intervals>
struct interval_plus_impl { };

template<typename interval>
struct interval_plus_impl<interval> : interval { };

template<typename fst, typename snd, typename...rst>
struct interval_plus_impl<fst, snd, rst...> {
  template<int dummy>
  struct impl : interval_plus_impl<
                  typename binary_interval_plus<
                    typename close_if_nonempty_integral<fst>::type, 
                    typename close_if_nonempty_integral<snd>::type
                  >::type,
                  rst...
                > { };
  using type = typename boost::mpl::eval_if<
                 is_empty_interval<fst>,
                 fst,
                 boost::mpl::eval_if<
                   is_empty_interval<snd>,
                   snd,
                   impl<1>
                 >
               >::type;
};

/** Add sequence of intervals. Each element must be an interval.
  * @note At the time of writing this, elements are added using <tt>boost::mpl::plus</tt>. It is required that value-type of adding infimums be the same as 
  *       value-type of adding supremums.
  * @note Not defined for the empty sequence */
template<typename...intervals> 
struct interval_plus : interval_plus_impl<intervals...> { };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename fst, typename snd>
struct binary_interval_times { 
  template<typename n1, typename n2, bool c1, bool c2>
  using closed_bool = boost::mpl::bool_<
                              (c1 && c2) ||
                              (c1 && is_zero<n1>::value) ||
                              (c2 && is_zero<n2>::value)
                            >;
  using list = boost::mpl::list4<
                 boost::mpl::pair<typename boost::mpl::times<typename inf_of<fst>::type, typename inf_of<snd>::type>::type,
                                           closed_bool<typename inf_of<fst>::type, typename inf_of<snd>::type, is_left_closed<fst>::value, is_left_closed<snd>::value>>,
                 boost::mpl::pair<typename boost::mpl::times<typename inf_of<fst>::type, typename sup_of<snd>::type>::type,
                                           closed_bool<typename inf_of<fst>::type, typename sup_of<snd>::type, is_left_closed<fst>::value, is_right_closed<snd>::value>>,
                 boost::mpl::pair<typename boost::mpl::times<typename sup_of<fst>::type, typename inf_of<snd>::type>::type,
                                           closed_bool<typename sup_of<fst>::type, typename inf_of<snd>::type, is_right_closed<fst>::value, is_left_closed<snd>::value>>,
                 boost::mpl::pair<typename boost::mpl::times<typename sup_of<fst>::type, typename sup_of<snd>::type>::type,
                                           closed_bool<typename sup_of<fst>::type, typename sup_of<snd>::type, is_right_closed<fst>::value, is_right_closed<snd>::value>>
               >;
  using inf  = typename boost::mpl::front<typename boost::mpl::sort<list, pair_lb_less>::type>::type;
  using sup  = typename boost::mpl::front<typename boost::mpl::sort<list, pair_ub_less>::type>::type;
  using type = interval<
                 inf::second::value,
                 typename inf::first,
                 typename sup::first,
                 sup::second::value
               >;
};

template<typename...intervals>
struct interval_times_impl { };

template<typename interval>
struct interval_times_impl<interval> : interval { };

template<typename fst, typename snd, typename...rst> 
struct interval_times_impl<fst, snd, rst...> {
  template<int dummy>
  struct impl : interval_times_impl<
                  typename binary_interval_times<
                    typename close_if_nonempty_integral<fst>::type, 
                    typename close_if_nonempty_integral<snd>::type
                  >::type,
                  rst...
                > { };
  using type = typename boost::mpl::eval_if<
                 is_empty_interval<fst>,
                 fst,
                 boost::mpl::eval_if<
                   is_empty_interval<snd>,
                   snd,
                   impl<1>
                 >
               >::type;
};

/** Multiply sequence of intervals. Each element must be an interval.
  * @note At the time of writing this, elements are multiplied using <tt>boost::mpl::times</tt>. It is required that value-type of multipling supremums and 
  *       infimums be the same.
  * @note Not defined for the empty sequence */
template<typename...intervals>
struct interval_times : interval_times_impl<intervals...> { };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename...intervals>
struct interval_intersection_impl { };

template<typename interval>
struct interval_intersection_impl<interval> : interval { };

template<typename fst, typename snd, typename...rst> 
struct interval_intersection_impl<fst, snd, rst...> {

  using list1 = boost::mpl::list2<
                  boost::mpl::pair<typename inf_of<fst>::type, is_left_closed<fst>>,
                  boost::mpl::pair<typename inf_of<snd>::type, is_left_closed<snd>>>;
  using list2 = boost::mpl::list2<
                  boost::mpl::pair<typename sup_of<fst>::type, is_right_closed<fst>>,
                  boost::mpl::pair<typename sup_of<snd>::type, is_right_closed<snd>>>;

  using inf  = typename boost::mpl::deref<typename boost::mpl::next<typename boost::mpl::begin<typename boost::mpl::sort<list1, pair_lb_less>::type>::type>::type>::type;
  using sup  = typename boost::mpl::deref<typename boost::mpl::next<typename boost::mpl::begin<typename boost::mpl::sort<list2, pair_ub_less>::type>::type>::type>::type;

  using type = interval<
                 inf::second::value,
                 typename inf::first,
                 typename sup::first,
                 sup::second::value
               >;
};

/** Intersect sequence of intervals. Each element must be an interval.
  * @note It is required that value-type of endpoints of all intervals be the same.
  * @note Not defined for the empty sequence */
template<typename...intervals> 
struct interval_intersection : interval_intersection_impl<intervals...> { };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

} // namespace cmn::mpl

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace boost::mpl {
  
template<>
struct empty_impl<cmn::mpl::interval_tag> {
  template<typename I>
  struct apply : cmn::mpl::is_empty_interval<I> { }; };

template<>
struct size_impl<cmn::mpl::interval_tag> {
  template<typename I>
  struct apply : cmn::mpl::interval_size<I> { }; };


template<>
struct plus_impl<cmn::mpl::interval_tag, cmn::mpl::interval_tag> {
  template<typename fst, typename snd>
  struct apply : cmn::mpl::interval_plus<fst, snd> { }; };

template<>
struct plus_impl<integral_c_tag, cmn::mpl::interval_tag> {
  template<typename fst, typename snd>
  struct apply : cmn::mpl::interval_plus<cmn::mpl::interval<true,fst,fst,true>, snd> { }; };

template<>
struct plus_impl<cmn::mpl::interval_tag, integral_c_tag> {
  template<typename fst, typename snd>
  struct apply : cmn::mpl::interval_plus<fst, cmn::mpl::interval<true,snd,snd,true>> { }; };


template<>
struct times_impl<cmn::mpl::interval_tag, cmn::mpl::interval_tag> {
  template<typename fst, typename snd>
  struct apply : cmn::mpl::interval_times<fst, snd> { }; };

template<>
struct times_impl<integral_c_tag, cmn::mpl::interval_tag> {
  template<typename fst, typename snd>
  struct apply : cmn::mpl::interval_times<cmn::mpl::interval<true,fst,fst,true>, snd> { }; };

template<>
struct times_impl<cmn::mpl::interval_tag, integral_c_tag> {
  template<typename fst, typename snd>
  struct apply : cmn::mpl::interval_times<fst, cmn::mpl::interval<true,snd,snd,true>> { }; };

} // namespace boost::mpl

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace std {

template<bool lc, typename inf, typename sup, bool rc>
struct is_same<cmn::mpl::interval<lc,inf,sup,rc>, cmn::mpl::interval<lc,inf,sup,rc>> : boost::mpl::true_ { };

template<bool lc1, typename inf1, typename sup1, bool rc1, bool lc2, typename inf2, typename sup2, bool rc2>
struct is_same<cmn::mpl::interval<lc1,inf1,sup1,rc1>, cmn::mpl::interval<lc2,inf2,sup2,rc2>> : 
  boost::mpl::and_<
    cmn::mpl::is_interval_subset<cmn::mpl::interval<lc1,inf1,sup1,rc1>, cmn::mpl::interval<lc2,inf2,sup2,rc2>>,
    cmn::mpl::is_interval_subset<cmn::mpl::interval<lc2,inf2,sup2,rc2>, cmn::mpl::interval<lc1,inf1,sup1,rc1>>
  > { };

}

#endif // CMN__MPL__INTERVAL__HPP
