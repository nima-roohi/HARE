#ifndef CMN__MPL__ZERO_OF__HPP
#define CMN__MPL__ZERO_OF__HPP

#include <boost/mpl/integral_c.hpp>

namespace cmn::mpl {

template<typename type>
struct zero_of { };

template<> struct zero_of<bool>          : boost::mpl::integral_c<bool         , false> { };
template<> struct zero_of<char>          : boost::mpl::integral_c<char         , 0> { };
template<> struct zero_of<signed   char> : boost::mpl::integral_c<signed   char, 0> { };
template<> struct zero_of<unsigned char> : boost::mpl::integral_c<unsigned char, 0> { };
template<> struct zero_of<signed   short>     : boost::mpl::integral_c<signed   short        , 0> { };
template<> struct zero_of<signed   int>       : boost::mpl::integral_c<signed   int          , 0> { };
template<> struct zero_of<signed   long>      : boost::mpl::integral_c<signed   long         , 0> { };
template<> struct zero_of<signed   long long> : boost::mpl::integral_c<signed   long long    , 0> { };
template<> struct zero_of<unsigned short>     : boost::mpl::integral_c<unsigned short        , 0> { };
template<> struct zero_of<unsigned int>       : boost::mpl::integral_c<unsigned int          , 0> { };
template<> struct zero_of<unsigned long>      : boost::mpl::integral_c<unsigned long         , 0> { };
template<> struct zero_of<unsigned long long> : boost::mpl::integral_c<unsigned long long    , 0> { };

} // namespace cmn::mpl

#endif // CMN__MPL__ZERO_OF__HPP
