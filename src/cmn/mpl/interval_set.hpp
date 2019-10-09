#ifndef CMN__MPL__INTERVAL_SET__HPP
#define CMN__MPL__INTERVAL_SET__HPP

#include "cmn/mpl/equal.hpp"
#include "cmn/mpl/interval.hpp"
#include "cmn/mpl/is_well_formed.hpp"
#include "cmn/mpl/make_list.hpp"

#include <boost/mpl/begin_end.hpp>
#include <boost/mpl/deref.hpp>
#include <boost/mpl/empty.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/front_inserter.hpp>
#include <boost/mpl/joint_view.hpp>
#include <boost/mpl/list.hpp>
#include <boost/mpl/next.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/transform_view.hpp>
#include <boost/mpl/vector.hpp>

#include <limits>

namespace cmn::mpl {

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

struct interval_set_tag;

template<typename seq_>
struct interval_set {
  using type = interval_set;
  using tag  = interval_set_tag;
  using seq  = seq_;
  static_assert(is_well_formed<type>::value, "invalid sequence of intervals");
};

template<typename...elems>
using interval_set_t = interval_set<make_list_t<elems...>>;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename interval_set> struct seq_of                          { };
template<typename seq>          struct seq_of<interval_set<seq>> : seq { };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename interval_set>
struct is_empty_interval_set : boost::mpl::empty<typename seq_of<interval_set>::type> { }; 

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename interval_set>
struct interval_set_size_impl { 
  using seq = typename boost::mpl::transform<
                typename seq_of<interval_set>::type,
                boost::mpl::size<boost::mpl::placeholders::_1>,
                boost::mpl::front_inserter<boost::mpl::list0<>>
              >::type;
  template<int dummy>
  struct impl : boost::mpl::fold<
                  typename boost::mpl::pop_front<seq>::type,
                  typename boost::mpl::    front<seq>::type,
                  boost::mpl::plus<
                    boost::mpl::placeholders::_1,
                    boost::mpl::placeholders::_2>
                > {  };
  using type = typename boost::mpl::eval_if<
                 boost::mpl::empty<seq>,
                 boost::mpl::integral_c<std::uint8_t, 0>,
                 impl<1>
               >::type;
};

template<typename interval_set>
struct interval_set_size : interval_set_size_impl<interval_set>::type { };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename fst, typename iter, typename end>
struct is_well_formed_interval_set {
  using snd  = typename boost::mpl::deref<iter>::type;
  using next = typename boost::mpl::next <iter>::type;
  using I    = interval<
                 is_right_open  <fst>::value,
                 typename sup_of<fst>::type ,
                 typename inf_of<snd>::type ,
                 is_left_open   <snd>::value
               >;
  using type = typename boost::mpl::and_<
                 boost::mpl::not_<boost::mpl::empty<fst>>,
                 boost::mpl::not_<boost::mpl::empty<I  >>,
                 is_well_formed_interval_set<snd, next, end>
               >::type;
};

template<typename head, typename iter>
struct is_well_formed_interval_set<head, iter, iter> : boost::mpl::not_<boost::mpl::empty<head>> { };

template<typename seq> 
struct is_well_formed_impl<interval_set<seq>, interval_set_tag> {
  template<int dummy>
  struct impl : is_well_formed_interval_set<
                  typename boost::mpl::front<seq>::type,
                  typename boost::mpl::next<typename boost::mpl::begin<seq>::type>::type,
                  typename boost::mpl::end<seq>::type> { };
  using type = typename boost::mpl::or_<
                 boost::mpl::empty<seq>,
                 impl<1>
               >::type;
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename res_, typename I, typename iter, typename end>
struct binary_interval_set_union_impl { 
  using res   = typename res_::type;
  using deref = typename boost::mpl::deref<iter>::type;
  using next  = typename boost::mpl::next <iter>::type;
  using I2 = interval<
               is_right_open<I>::value,
               typename sup_of<I>    ::type,
               typename inf_of<deref>::type,
               is_left_open<deref>::value>;
  using list = boost::mpl::list2<
                 boost::mpl::pair<typename sup_of<I    >::type, is_right_closed<I    >>,
                 boost::mpl::pair<typename sup_of<deref>::type, is_right_closed<deref>>>;
  using sup  = typename boost::mpl::front<typename boost::mpl::sort<list, pair_ub_less>::type>::type;
  using type = typename boost::mpl::eval_if<
                 boost::mpl::empty<I2>,
                 binary_interval_set_union_impl<
                   res,
                   interval<
                     is_left_closed<I>::value,
                     typename inf_of<I>::type,
                     typename sup::first,
                     sup::second::value>,
                   next,
                   end>,
                 binary_interval_set_union_impl<
                   boost::mpl::push_back<res, I>,
                   deref,
                   next,
                   end>
               >::type;
};

template<typename res, typename I, typename iter>
struct binary_interval_set_union_impl<res, I, iter, iter> : interval_set<typename boost::mpl::push_back<typename res::type, I>::type> { };

/** @note sequence of at least \c fst or \c snd must be non-empty. */
template<typename fst, typename snd>
struct binary_interval_set_union {
  using seq = typename boost::mpl::sort<
                typename boost::mpl::joint_view<
                  typename seq_of<fst>::type,
                  typename seq_of<snd>::type
                >::type,
                interval_lb_less,
                boost::mpl::back_inserter<boost::mpl::vector0<>>
              >::type;
  using type = typename binary_interval_set_union_impl<
                 boost::mpl::vector0<>,
                 typename boost::mpl::front<seq>::type,
                 typename boost::mpl::next<typename boost::mpl::begin<seq>::type>::type,
                 typename boost::mpl::end<seq>::type
               >::type;
};

template<typename...interval_sets>
struct interval_set_union_impl : interval_set_t<> { static_assert(sizeof...(interval_sets) == 0, "error in code"); };

template<typename interval_set>
struct interval_set_union_impl<interval_set> : interval_set { };

template<typename fst, typename snd, typename...rst>
struct interval_set_union_impl<fst, snd, rst...> {
  template<int dummy>
  struct impl : interval_set_union_impl<typename binary_interval_set_union<fst, snd>::type, rst...> { };

  using type = typename boost::mpl::eval_if<
                 is_empty_interval_set<fst>,
                 interval_set_union_impl<snd, rst...>,
                 boost::mpl::eval_if<
                   is_empty_interval_set<snd>,
                   interval_set_union_impl<fst, rst...>,
                   impl<1>
                 >
               >::type;
};

/** Union sequence of interval_sets. Each element must be an interval_set. */
template<typename...interval_sets> 
struct interval_set_union : interval_set_union_impl<interval_sets...> { };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename res, typename seq, typename I>
struct interval_seq_I_intersection {
  template<typename ans, typename I2>
  struct step : boost::mpl::eval_if<
                  boost::mpl::empty<typename interval_intersection<I, I2>::type>,
                  ans,
                  boost::mpl::push_back<
                    ans,
                    typename interval_intersection<I, I2>::type
                  >
                > { }; 
  using type = typename boost::mpl::fold<
                 seq,
                 res,
                 step<boost::mpl::placeholders::_1, boost::mpl::placeholders::_2>
                >::type;
};

/** @note sequence of at least \c fst or \c snd must be non-empty. */
template<typename fst, typename snd>
struct binary_interval_set_intersection {
  using seq = typename boost::mpl::fold<
                typename seq_of<fst>::type,
                boost::mpl::vector0<>,
                interval_seq_I_intersection<
                  boost::mpl::placeholders::_1,
                  typename seq_of<snd>::type,
                  boost::mpl::placeholders::_2>
              >::type;
  using sorted = typename boost::mpl::sort<seq, interval_lb_less>::type;
  template<int dummy>
  struct impl : binary_interval_set_union_impl<
                  boost::mpl::vector0<>,
                  typename boost::mpl::front<sorted>::type,
                  typename boost::mpl::next<typename boost::mpl::begin<sorted>::type>::type,
                  typename boost::mpl::end<sorted>::type
                > { };
  using type = typename boost::mpl::eval_if<
                 boost::mpl::empty<sorted>,
                 interval_set_t<>,
                 impl<1>
               >::type;
};

template<typename...interval_sets>
struct interval_set_intersection_impl { };

template<typename interval_set>
struct interval_set_intersection_impl<interval_set> : interval_set { };

template<typename fst, typename snd, typename...rst>
struct interval_set_intersection_impl<fst, snd, rst...> {
  template<int dummy>
  struct impl : interval_set_intersection_impl<typename binary_interval_set_intersection<fst, snd>::type, rst...> { };

  using type = typename boost::mpl::eval_if<
                 boost::mpl::or_<
                   is_empty_interval_set<fst>,
                   is_empty_interval_set<snd>>,
                 interval_set_t<>,
                 impl<1>
               >::type;
};

/** Union sequence of interval_sets. Each element must be an interval_set. 
  * @note undefined for the empty sequence */
template<typename...interval_sets> 
struct interval_set_intersection : interval_set_intersection_impl<interval_sets...> { };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename res, typename seq, typename I>
struct interval_seq_I_plus :
  boost::mpl::transform<
    seq,
    boost::mpl::plus<
      boost::mpl::placeholders::_1,
      I>,
    boost::mpl::front_inserter<res>
  > { };

/** @note sequence of at least \c fst or \c snd must be non-empty. */
template<typename fst, typename snd>
struct interval_set_binary_plus {
  using seq = typename boost::mpl::fold<
                typename seq_of<fst>::type,
                boost::mpl::list0<>,
                interval_seq_I_plus<
                  boost::mpl::placeholders::_1,
                  typename seq_of<snd>::type,
                  boost::mpl::placeholders::_2>
              >::type;
  using sorted = typename boost::mpl::sort<seq, interval_lb_less>::type;
  using type = typename binary_interval_set_union_impl<
                 boost::mpl::vector0<>,
                 typename boost::mpl::front<sorted>::type,
                 typename boost::mpl::next<typename boost::mpl::begin<sorted>::type>::type,
                 typename boost::mpl::end<sorted>::type
               >::type;
};

template<typename...interval_sets>
struct interval_set_plus_impl : interval_set_t<> { static_assert(sizeof...(interval_sets) == 0, "error in code"); };

template<typename interval_set>
struct interval_set_plus_impl<interval_set> : interval_set { };

template<typename fst, typename snd, typename...rst>
struct interval_set_plus_impl<fst, snd, rst...> {
  template<int dummy>
  struct impl : interval_set_plus_impl<
                  typename interval_set_binary_plus<fst, snd>::type,
                  rst...
                > { };
  using type = typename boost::mpl::eval_if<
                 boost::mpl::or_<
                   is_empty_interval_set<fst>,
                   is_empty_interval_set<snd>>,
                 interval_set_t<>,
                 impl<1>
               >::type;
};

/** Add sequence of interval_sets. Each element must be an interval_set. */
template<typename...interval_sets> 
struct interval_set_plus : interval_set_plus_impl<interval_sets...> { };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename res, typename seq, typename I>
struct interval_seq_I_times :
  boost::mpl::transform<
    seq,
    boost::mpl::times<
      boost::mpl::placeholders::_1,
      I>,
    boost::mpl::front_inserter<res>
  > { };

/** @note sequence of at least \c fst or \c snd must be non-empty. */
template<typename fst, typename snd>
struct interval_set_binary_times {
  using seq = typename boost::mpl::fold<
                typename seq_of<fst>::type,
                boost::mpl::list0<>,
                interval_seq_I_times<
                  boost::mpl::placeholders::_1,
                  typename seq_of<snd>::type,
                  boost::mpl::placeholders::_2>
              >::type;
  using sorted = typename boost::mpl::sort<seq, interval_lb_less>::type;
  using type = typename binary_interval_set_union_impl<
                 boost::mpl::vector0<>,
                 typename boost::mpl::front<sorted>::type,
                 typename boost::mpl::next<typename boost::mpl::begin<sorted>::type>::type,
                 typename boost::mpl::end<sorted>::type
               >::type;
};

template<typename...interval_sets>
struct interval_set_times_impl : interval_set_t<> { static_assert(sizeof...(interval_sets) == 0, "error in code"); };

template<typename interval_set>
struct interval_set_times_impl<interval_set> : interval_set { };

template<typename fst, typename snd, typename...rst>
struct interval_set_times_impl<fst, snd, rst...> {
  template<int dummy>
  struct impl : interval_set_times_impl<
                  typename interval_set_binary_times<fst, snd>::type,
                  rst...
                > { };
  using type = typename boost::mpl::eval_if<
                 boost::mpl::or_<
                   is_empty_interval_set<fst>,
                   is_empty_interval_set<snd>>,
                 interval_set_t<>,
                 impl<1>
               >::type;
};

/** Add sequence of interval_sets. Each element must be an interval_set. */
template<typename...interval_sets> 
struct interval_set_times : interval_set_times_impl<interval_sets...> { };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

} // namespace cmn::mpl

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace boost::mpl {
  
template<>
struct empty_impl<cmn::mpl::interval_set_tag> {
  template<typename I>
  struct apply : cmn::mpl::is_empty_interval_set<I> { }; };

template<>
struct size_impl<cmn::mpl::interval_set_tag> {
  template<typename I>
  struct apply : cmn::mpl::interval_set_size<I> { }; };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<>
struct plus_impl<cmn::mpl::interval_set_tag, cmn::mpl::interval_set_tag> {
  template<typename fst, typename snd>
  struct apply : cmn::mpl::interval_set_plus<fst, snd> { }; };

template<>
struct plus_impl<cmn::mpl::interval_tag, cmn::mpl::interval_set_tag> {
  template<typename fst, typename snd>
  struct apply : 
    eval_if<
      empty<fst>,
      cmn::mpl::interval_set_t<>,
      plus<cmn::mpl::interval_set_t<fst>, snd>
    >::type { }; };

template<>
struct plus_impl<cmn::mpl::interval_set_tag, cmn::mpl::interval_tag> {
  template<typename fst, typename snd>
  struct apply : 
    eval_if<
      empty<snd>,
      cmn::mpl::interval_set_t<>,
      plus<fst, cmn::mpl::interval_set_t<snd>>
    >::type { }; };

template<>
struct plus_impl<integral_c_tag, cmn::mpl::interval_set_tag> {
  template<typename fst, typename snd>
  struct apply : plus<cmn::mpl::interval<true,fst,fst,true>, snd> { }; };

template<>
struct plus_impl<cmn::mpl::interval_set_tag, integral_c_tag> {
  template<typename fst, typename snd>
  struct apply : plus<fst, cmn::mpl::interval<true,snd,snd,true>> { }; };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<>
struct times_impl<cmn::mpl::interval_set_tag, cmn::mpl::interval_set_tag> {
  template<typename fst, typename snd>
  struct apply : cmn::mpl::interval_set_times<fst, snd> { }; };

template<>
struct times_impl<cmn::mpl::interval_tag, cmn::mpl::interval_set_tag> {
  template<typename fst, typename snd>
  struct apply : 
    eval_if<
      empty<fst>,
      cmn::mpl::interval_set_t<>,
      times<cmn::mpl::interval_set_t<fst>, snd>
    >::type { }; };

template<>
struct times_impl<cmn::mpl::interval_set_tag, cmn::mpl::interval_tag> {
  template<typename fst, typename snd>
  struct apply : 
    eval_if<
      empty<snd>,
      cmn::mpl::interval_set_t<>,
      times<fst, cmn::mpl::interval_set_t<snd>>
    >::type { }; };

template<>
struct times_impl<integral_c_tag, cmn::mpl::interval_set_tag> {
  template<typename fst, typename snd>
  struct apply : times<cmn::mpl::interval<true,fst,fst,true>, snd> { }; };

template<>
struct times_impl<cmn::mpl::interval_set_tag, integral_c_tag> {
  template<typename fst, typename snd>
  struct apply : times<fst, cmn::mpl::interval<true,snd,snd,true>> { }; };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

} // namespace boost::mpl

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace std {

template<typename seq>
struct is_same<cmn::mpl::interval_set<seq>, cmn::mpl::interval_set<seq>> : boost::mpl::true_ { };

template<typename seq1, typename seq2>
struct is_same<cmn::mpl::interval_set<seq1>, cmn::mpl::interval_set<seq2>> : 
  cmn::mpl::equal<seq1, seq2> { };

}

#endif // CMN__MPL__INTERVAL_SET__HPP
