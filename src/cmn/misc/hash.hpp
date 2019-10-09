#ifndef CMN__MISC__HASH__HPP
#define CMN__MISC__HASH__HPP

#include "cmn/dbc.hpp"
#include "cmn/mpl/map_tuple.hpp"
#include "cmn/mpl/is_tuple.hpp"

#include <boost/mpl/identity.hpp>

#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>
#include <vector>

namespace cmn::misc {

template<typename T, typename Seq, typename Hash = std::hash<T>>
struct sequence_hash {
  using argument_type = Seq;
  using result_type   = std::size_t;
  result_type operator()(const argument_type& that) const {
    const Hash hash{};
    result_type res = 0;
    for(cmn::pbv_t<T> elem : that)
      res ^= hash(elem);
    return res;  }
};

namespace internal {
struct tuple_hash_map {
  template<typename T>
  struct apply : boost::mpl::identity<std::hash<T>> { };
};
} // namespace internal

template<typename T, typename Hash = typename mpl::map_tuple<T, internal::tuple_hash_map>::type, 
                     typename std::enable_if_t<mpl::is_tuple_v<T>  &&  mpl::is_tuple_v<Hash>  && 
                                              std::tuple_size<T>::value == std::tuple_size<Hash>::value>* = nullptr>
struct tuple_hash {
  using argument_type = T;
  using result_type   = std::size_t;

  result_type operator()(const argument_type& that) const { return compute<std::tuple_size<T>::value>(that, Hash()); }

private:
  template<std::size_t index, std::enable_if_t<index == 0>* = nullptr>
  result_type compute(const argument_type& /*that*/, const Hash& /*hash**/) const { return 0; }

  template<std::size_t index, std::enable_if_t<index != 0>* = nullptr>
  result_type compute(const argument_type& that, const Hash& hash) const { 
    return compute<index - 1>(that, hash) ^ std::get<index - 1>(hash)(std::get<index - 1>(that)); }
};

} // namespace cmn::misc

#endif // CMN__MISC__HASH__HPP
