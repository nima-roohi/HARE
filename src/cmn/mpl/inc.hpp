#ifndef CMN__MPL__INC__HPP
#define CMN__MPL__INC__HPP

#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/integral_c.hpp>
#include <boost/mpl/integral_c_tag.hpp>
#include <boost/mpl/plus.hpp>

#include <cstdint>
#include <type_traits>

namespace cmn::mpl {

template<typename num, typename tag>
struct inc_impl { };

template<typename num>
struct inc_impl<num, boost::mpl::integral_c_tag> : 
  boost::mpl::eval_if<
    std::is_signed<typename num::value_type>,
    boost::mpl::plus<num, boost::mpl::integral_c<std:: int8_t, 1>>,
    boost::mpl::plus<num, boost::mpl::integral_c<std::uint8_t, 1>>
  >::type { };

template<typename num>
struct inc : inc_impl<num, typename num::tag> { };

} // namespace cmn::mpl

#endif // CMN__MPL__INC__HPP
