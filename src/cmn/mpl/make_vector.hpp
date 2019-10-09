#ifndef CMN__MPL__MAKE_VECTOR__HPP
#define CMN__MPL__MAKE_VECTOR__HPP

#include "cmn/mpl/list.hpp"

#include <boost/mpl/vector.hpp>
#include <boost/mpl/push_front.hpp>

#include <type_traits>

namespace cmn::mpl {

template<typename vector, typename enabled = void> 
struct make_vector_impl { };

template<typename...elems> 
struct make_vector_impl<list<elems...>, std::enable_if_t<(sizeof...(elems) <= 20)>> {
  template<std::size_t size, typename enabled = void> struct apply { };

  template<std::size_t size> struct apply<size, std::enable_if_t<size == 0>> : boost::mpl::vector0 <>         { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size == 1>> : boost::mpl::vector1 <elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size == 2>> : boost::mpl::vector2 <elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size == 3>> : boost::mpl::vector3 <elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size == 4>> : boost::mpl::vector4 <elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size == 5>> : boost::mpl::vector5 <elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size == 6>> : boost::mpl::vector6 <elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size == 7>> : boost::mpl::vector7 <elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size == 8>> : boost::mpl::vector8 <elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size == 9>> : boost::mpl::vector9 <elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size ==10>> : boost::mpl::vector10<elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size ==11>> : boost::mpl::vector11<elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size ==12>> : boost::mpl::vector12<elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size ==13>> : boost::mpl::vector13<elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size ==14>> : boost::mpl::vector14<elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size ==15>> : boost::mpl::vector15<elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size ==16>> : boost::mpl::vector16<elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size ==17>> : boost::mpl::vector17<elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size ==18>> : boost::mpl::vector18<elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size ==19>> : boost::mpl::vector19<elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size ==20>> : boost::mpl::vector20<elems...> { };
};

template<typename elem, typename...elems> 
struct make_vector_impl<list<elem, elems...>, std::enable_if_t<(sizeof...(elems) >= 20)>> {
  template<std::size_t size> 
  struct apply : boost::mpl::push_front<
                   typename make_vector_impl<list<elems...>>::template apply<sizeof...(elems)>::type, 
                   elem
                 > { };
};

template<typename...elems>
struct make_vector : make_vector_impl<list<elems...>>::template apply<sizeof...(elems)> { };

template<typename...elems>
using make_vector_t = typename make_vector<elems...>::type;

} // namespace cmn::mpl

#endif // CMN__MPL__MAKE_VECTOR__HPP
