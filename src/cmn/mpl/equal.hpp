#ifndef CMN__MPL__EQUAL__HPP
#define CMN__MPL__EQUAL__HPP

#include <boost/mpl/and.hpp>
#include <boost/mpl/begin.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/deref.hpp>
#include <boost/mpl/end.hpp>
#include <boost/mpl/lambda.hpp>
#include <boost/mpl/next.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/size.hpp>

#include <type_traits>

namespace cmn::mpl {

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename pred, typename fst_iter, typename snd_iter, typename fst_end, typename snd_end>
struct equal_impl : boost::mpl::and_<
                      typename pred::template apply<
                        typename boost::mpl::deref<fst_iter>::type, 
                        typename boost::mpl::deref<snd_iter>::type>::type,
                      equal_impl<
                        pred,
                        typename boost::mpl::next<fst_iter>::type,
                        typename boost::mpl::next<snd_iter>::type, 
                        fst_end,
                        snd_end>> { };

template<typename pred, typename snd_iter, typename fst_end, typename snd_end> struct equal_impl<pred, fst_end , snd_iter, fst_end, snd_end> : boost::mpl::false_ { };
template<typename pred, typename fst_iter, typename fst_end, typename snd_end> struct equal_impl<pred, fst_iter, snd_end , fst_end, snd_end> : boost::mpl::false_ { };
template<typename pred,                    typename fst_end, typename snd_end> struct equal_impl<pred, fst_end , snd_end , fst_end, snd_end> : boost::mpl::true_  { };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename seq1, 
         typename seq2, 
         typename pred = typename boost::mpl::lambda<
                           std::is_same<
                             boost::mpl::placeholders::_1, 
                             boost::mpl::placeholders::_2>>::type>
struct equal : boost::mpl::and_<
                 boost::mpl::bool_<
                   boost::mpl::size<seq1>::value ==
                   boost::mpl::size<seq2>::value>,
                 equal_impl<
                   pred,
                   typename boost::mpl::begin<seq1>::type,
                   typename boost::mpl::begin<seq2>::type,
                   typename boost::mpl::end  <seq1>::type,
                   typename boost::mpl::end  <seq2>::type>
               >::type { };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

} // namespace cmn::mpl

#endif // CMN__MPL__EQUAL__HPP
