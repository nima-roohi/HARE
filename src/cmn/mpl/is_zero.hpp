#ifndef CMN__MPL__IS_ZERO__HPP
#define CMN__MPL__IS_ZERO__HPP

#include <boost/mpl/bool.hpp>
#include <boost/mpl/integral_c_tag.hpp>

namespace cmn::mpl {

template<typename num, typename tag>
struct is_zero_impl { };

template<typename num>
struct is_zero_impl<num, boost::mpl::integral_c_tag> : boost::mpl::bool_<num::value == 0> { }; 

template<typename num>
struct is_zero : is_zero_impl<num, typename num::tag> { };

} // namespace cmn::mpl

#endif // CMN__MPL__IS_ZERO__HPP
