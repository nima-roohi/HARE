#ifndef CMN__MPL__MAKE_LIST__HPP
#define CMN__MPL__MAKE_LIST__HPP

#include "cmn/mpl/list.hpp"

#include <boost/mpl/list.hpp>
#include <boost/mpl/push_front.hpp>

#include <type_traits>

namespace cmn::mpl {

template<typename list, typename enabled = void> 
struct make_list_impl { };

template<typename...elems> 
struct make_list_impl<list<elems...>, std::enable_if_t<(sizeof...(elems) <= 20)>> {
  template<std::size_t size, typename enabled = void> struct apply { };

  template<std::size_t size> struct apply<size, std::enable_if_t<size == 0>> : boost::mpl::list0 <>         { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size == 1>> : boost::mpl::list1 <elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size == 2>> : boost::mpl::list2 <elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size == 3>> : boost::mpl::list3 <elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size == 4>> : boost::mpl::list4 <elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size == 5>> : boost::mpl::list5 <elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size == 6>> : boost::mpl::list6 <elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size == 7>> : boost::mpl::list7 <elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size == 8>> : boost::mpl::list8 <elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size == 9>> : boost::mpl::list9 <elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size ==10>> : boost::mpl::list10<elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size ==11>> : boost::mpl::list11<elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size ==12>> : boost::mpl::list12<elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size ==13>> : boost::mpl::list13<elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size ==14>> : boost::mpl::list14<elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size ==15>> : boost::mpl::list15<elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size ==16>> : boost::mpl::list16<elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size ==17>> : boost::mpl::list17<elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size ==18>> : boost::mpl::list18<elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size ==19>> : boost::mpl::list19<elems...> { };
  template<std::size_t size> struct apply<size, std::enable_if_t<size ==20>> : boost::mpl::list20<elems...> { };
};

template<typename elem, typename...elems> 
struct make_list_impl<list<elem, elems...>, std::enable_if_t<(sizeof...(elems) >= 20)>> {
  template<std::size_t size> 
  struct apply : boost::mpl::push_front<
                   typename make_list_impl<list<elems...>>::template apply<sizeof...(elems)>::type, 
                   elem
                 > { };
};

template<typename...elems>
struct make_list : make_list_impl<list<elems...>>::template apply<sizeof...(elems)> { };

template<typename...elems>
using make_list_t = typename make_list<elems...>::type;

} // namespace cmn::mpl

#endif // CMN__MPL__MAKE_LIST__HPP
