#ifndef CMN__MPL__IS_WELL_FORMED__HPP
#define CMN__MPL__IS_WELL_FORMED__HPP

namespace cmn::mpl {

template<typename type, typename tag>
struct is_well_formed_impl { };

template<typename type>
struct is_well_formed : is_well_formed_impl<type, typename type::tag>::type { };

} // namespace cmn::mpl

#endif // CMN__MPL__IS_WELL_FORMED__HPP
