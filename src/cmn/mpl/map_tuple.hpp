#ifndef CMN__MPL__MAP_TUPLE__HPP
#define CMN__MPL__MAP_TUPLE__HPP

#include <boost/mpl/identity.hpp>

#include <tuple>

namespace cmn::mpl {

template<typename tuple, typename map>
struct map_tuple { };

template<typename map, typename...args>
struct map_tuple<std::tuple<args...>, map> : 
  boost::mpl::identity<std::tuple<typename map::template apply<args>::type...>> { };

} // namespace cmn::mpl

#endif // CMN__MPL__MAP_TUPLE__HPP


