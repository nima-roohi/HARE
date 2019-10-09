#ifndef CMN__TYPE_TRAITS__HPP
#define CMN__TYPE_TRAITS__HPP

#include <cstdint>			// fixed-width integers
#include <type_traits>
#include <utility>      // declval

#include <boost/mpl/at.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/mpl/integral_c.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/map.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/pair.hpp>
#include <boost/mpl/string.hpp>
#include <boost/mpl/void.hpp>
#include <boost/multiprecision/cpp_int.hpp>

#include <cstdint>

namespace cmn {

/** Suppose we want to efficiently pass a value to a function such that after the function is returned (either normally or because of an exception) original 
  * value is not changed. For a general type T we do this using 'const T&' argument type. But this method lowers the performance when T is a primitive or a 
  * pointer type (e.g. 'int' or 'long*'). The result of this meta-function is defined by the following rules:
	* -# if \c T is a \a fundamental type then the result will be <tt>const T</tt>, otherwise
	* -# if \c T is not a reference type, pointer, or a fundamental type then the result will be <tt>const T&</tt>, otherwise
	* -# the result will be determined by recursively going down the structure of \c T.
	*	@author Nima Roohi */
template<typename T>	
struct pbv {
  typedef typename boost::mpl::if_<
                     boost::mpl::or_<
                       std::is_fundamental<T>,
                       std::is_enum<T>>,
                     std::add_const_t<T>,
                     std::add_lvalue_reference_t<std::add_const_t<T>>>::type type;
};

template<typename T> struct pbv<T*               > { typedef std::remove_reference_t<typename pbv<T>::type>* const          type; };
template<typename T> struct pbv<T* const         > { typedef std::remove_reference_t<typename pbv<T>::type>* const          type; };
template<typename T> struct pbv<T* volatile      > { typedef std::remove_reference_t<typename pbv<T>::type>* const volatile type; };
template<typename T> struct pbv<T* volatile const> { typedef std::remove_reference_t<typename pbv<T>::type>* const volatile type; };
template<typename T> struct pbv<T&               > { typedef const std::remove_reference_t<typename pbv<T>::type>&          type; };
template<typename T> struct pbv<T&&              > { typedef const std::remove_reference_t<typename pbv<T>::type>&&          type; };

template<typename T>	
using pbv_t = typename pbv<T>::type; 

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** @tparam input   Type which is assumed to be integral (signed or unsigned).
  * @return         A type that is twice as big as the input type.
  * @note A (compile time) error will be generated if this meta function is not defined for the given input. */
template<typename T>
using doubled_t = typename boost::mpl::at<
														 boost::mpl::map<
															 boost::mpl::pair<int8_t                          , int16_t >,
															 boost::mpl::pair<int16_t                         , int32_t >,
															 boost::mpl::pair<int32_t                         , int64_t >,
                               boost::mpl::pair<int64_t                         , boost::multiprecision::int128_t >,
                               boost::mpl::pair<boost::multiprecision::int256_t , boost::multiprecision::int512_t >,
                               boost::mpl::pair<boost::multiprecision::int512_t , boost::multiprecision::int1024_t>,
                               boost::mpl::pair<int64_t                         , boost::multiprecision::int128_t >,
															 boost::mpl::pair<uint8_t                         , uint16_t>,
															 boost::mpl::pair<uint16_t                        , uint32_t>,
															 boost::mpl::pair<uint32_t                        , uint64_t>,
                               boost::mpl::pair<uint64_t                        , boost::multiprecision::uint128_t >,
                               boost::mpl::pair<boost::multiprecision::uint256_t, boost::multiprecision::uint512_t >,
                               boost::mpl::pair<boost::multiprecision::uint512_t, boost::multiprecision::uint1024_t>,
                               boost::mpl::pair<unsigned long, boost::multiprecision::uint128_t> //TODO FIXME
														 >, 
														 T>::type;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename T> 
using noexcept_copy_constructor_t = typename boost::mpl::eval_if<
                                               std::is_fundamental<T>,
                                               boost::mpl::bool_<true>,
                                               boost::mpl::bool_<noexcept(T(std::declval<T>()))>
                                               >::type;

template<typename T> 
using noexcept_move_constructor_t = typename boost::mpl::eval_if<
                                               std::is_fundamental<T>,
                                               boost::mpl::bool_<true>,
                                               boost::mpl::bool_<noexcept(T(std::move(std::declval<T>())))>
                                               >::type;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename T>
struct is_primitive : public boost::mpl::bool_<std::is_fundamental<T>::value> { };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
  
template<typename T>
using is_char_t = boost::mpl::bool_<std::is_same<T,          char>::value ||
                                    std::is_same<T, signed   char>::value ||
                                    std::is_same<T, unsigned char>::value ||
                                    std::is_same<T,       wchar_t>::value ||
                                    std::is_same<T,      char16_t>::value ||
                                    std::is_same<T,      char32_t>::value > ;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<              typename...strs> struct concat { using type = boost::mpl::string<>; };
template<typename str, typename...strs> struct concat<str, strs...> : boost::mpl::insert_range<typename str::type, typename boost::mpl::end<str>::type, typename concat<strs...>::type> { };

struct tnc {
  using   space     = boost::mpl::string<' '>;
  using t_float     = boost::mpl::string<'f', 'l', 'o', 'a', 't'>;
  using t_double    = boost::mpl::string<'d', 'o', 'u', 'b', 'l', 'e'>;
  using t_bool      = boost::mpl::string<'b', 'o', 'o', 'l'>;
  using t_short     = boost::mpl::string<'s', 'h', 'o', 'r', 't'>;
  using t_int       = boost::mpl::string<'i', 'n', 't'>;
  using t_long      = boost::mpl::string<'l', 'o', 'n', 'g'>; 
  using t_signed    = boost::mpl::string<'s', 'i', 'g', 'n', 'e', 'd'>; 
  using t_unsigned  = boost::mpl::string<'u', 'n', 's', 'i', 'g', 'n', 'e', 'd'>; 
  using t_char      = boost::mpl::string<'c', 'h', 'a', 'r'>;
  using t_wchar_t   = boost::mpl::string<'w', 'c', 'h', 'a', 'r', '_', 't'>;
  using t_char16_t  = boost::mpl::string<'c', 'h', 'a', 'r', '1', '6', '_', 't'>;
  using t_char32_t  = boost::mpl::string<'c', 'h', 'a', 'r', '3', '2', '_', 't'>;
};

template<typename kind> struct type_name { };

template<> struct type_name<bool>     : tnc::t_bool     { };
template<> struct type_name<char>     : tnc::t_char     { };
template<> struct type_name<wchar_t>  : tnc::t_wchar_t  { };
template<> struct type_name<char16_t> : tnc::t_char16_t { };
template<> struct type_name<char32_t> : tnc::t_char32_t { };
template<> struct type_name<float>    : tnc::t_float    { };
template<> struct type_name<double>   : tnc::t_double   { };

template<> struct type_name<signed    char> : concat<tnc::t_signed, tnc::space, tnc::t_char > { };
template<> struct type_name<signed   short> : concat<tnc::t_signed, tnc::space, tnc::t_short> { };
template<> struct type_name<signed     int> : concat<tnc::t_signed, tnc::space, tnc::t_int  > { };
template<> struct type_name<signed    long> : concat<tnc::t_signed, tnc::space, tnc::t_long > { };

template<> struct type_name<unsigned  char> : concat<tnc::t_unsigned, tnc::space, tnc::t_char  > { };
template<> struct type_name<unsigned short> : concat<tnc::t_unsigned, tnc::space, tnc::t_short > { };
template<> struct type_name<unsigned   int> : concat<tnc::t_unsigned, tnc::space, tnc::t_int   > { };
template<> struct type_name<unsigned  long> : concat<tnc::t_unsigned, tnc::space, tnc::t_long  > { };
template<> struct type_name<long    double> : concat<tnc::t_long    , tnc::space, tnc::t_double> { };

template<> struct type_name<signed   long long> : concat<tnc::t_signed  , tnc::space, tnc::t_long, tnc::space, tnc::t_long> { };
template<> struct type_name<unsigned long long> : concat<tnc::t_unsigned, tnc::space, tnc::t_long, tnc::space, tnc::t_long> { };
  
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** @brief Removes any \c const, \c volatile, and lvalue or rvalue reference (if they exists) from type \c %T 
  *        (not any of its elements in case it is a composite type). 
  * @note Type is supposed to be included in C++20. */
template<class T> struct remove_cvref : boost::mpl::identity<std::remove_cv_t<std::remove_reference_t<T>>> { };
template<class T> using  remove_cvref_t = typename remove_cvref<T>::type; ///< @see remove_cvref

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

} // namespace cmn

#endif //CMN__TYPE_TRAITS__HPP
