#ifndef CMN__MPL__IS_TUPLE__HPP
#define CMN__MPL__IS_TUPLE__HPP

#include <boost/mpl/bool.hpp>

#include <tuple>

namespace cmn::mpl {

template<typename T> 
struct is_tuple : boost::mpl::false_ { };

template<typename...Args> 
struct is_tuple<std::tuple<Args...>> : boost::mpl::true_ { };

template<typename T> 
constexpr bool is_tuple_v = is_tuple<T>::value;

} // namespace cmn::mpl

#endif // CMN__MPL__IS_TUPLE__HPP
