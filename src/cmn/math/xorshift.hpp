#ifndef CMN__MATH__XORSHIFT__HPP
#define CMN__MATH__XORSHIFT__HPP

#include "cmn/type_traits.hpp"

#include <chrono>
#include <cstdlib>								// size_t
#include <cstdint>								// fixed-width integers
#include <limits>                 // numeric_limits
#include <type_traits>						// is_unsigned, is_fundamental
#include <utility>                // move

namespace cmn::math {

enum class xorshift_type: unsigned char {
	NORM_64  ,	/**< Period is 2<sup>64  </sup>-1 */
	STAR_64  ,  /**< Period is 2<sup>64  </sup>-1 */
	PLUS_128 ,  /**< Period is 2<sup>128 </sup>-1 */
	STAR_1024,  /**< Period is 2<sup>1024</sup>-1 */
	STAR_4096   /**< Period is 2<sup>4096</sup>-1 */
};

/** @brief generates a random seed.
	* @note return values are guaranteed to be non-zero 
	* @note \c T is required to be unsigned integral type 
	* @note this method is intended to be used whenever an initial seed is not provided. Its implementation is very likely to read system-time and convert the 
	*				result to \c T. */
template<typename T> 
auto seed() {
	static_assert(std::is_unsigned<T>::value, "T must be unsigned");
  using namespace std::chrono;
	return static_cast<T>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count()); 
}

/** @brief fills the input array with the random numbers generated by the input PRNG.
  * @tparam T type of each element in the input array
  * @tparam N size of the input array
	* @tparam R type of input (pseudo) random number generator 
	* @note \c T is assume to be fundamental */
template<typename T, std::size_t N, typename R>
constexpr void fill(T (&arr)[N], R&& rnd) noexcept {
	static_assert(std::is_fundamental<T>::value, "T must be a fundamental type"); // result in noexcept
	for(std::size_t i = N; i > 0; )
		arr[--i] = rnd();
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** @brief pseudo random number generator based on the <a href="https://en.wikipedia.org/wiki/Xorshift">XOr Shift Pseudo Random Number Generator Algorithm.</a>
  * @see <a href="http://xorshift.di.unimi.it/">xorshift* / xorshift+ generators and the PRNG shootout</a>
  * The following is from C++11 Standand:
  * 6.3.1.3 Signed and unsigned integers
  *  -# When a value with integer type is converted to another integer type other than _Bool, if the value can be represented by the new type, it is unchanged.
  *  -# Otherwise, if the new type is unsigned, the value is converted by repeatedly adding or subtracting one more than the maximum value that can be 
  *     represented in the new type until the value is in the range of the new type.
  *  -# Otherwise, the new type is signed and the value cannot be represented in it; either the result is implementation-defined or an implementation-defined 
  *     signal is raised.
  * @author Nima Roohi */
template<xorshift_type T> class xorshift { };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<> struct xorshift<xorshift_type::NORM_64> {
	using result_type = uint64_t;
  static constexpr result_type min() { return std::numeric_limits<result_type>::min(); }
  static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }
	xorshift() : xorshift(seed<uint64_t>()) { }
	constexpr xorshift(result_type seed) noexcept : x(seed == 0 ? 1 : seed) { }
	constexpr result_type last() const noexcept { return x; }
	constexpr result_type operator()() noexcept {
		x ^= x >> 12; 
		x ^= x << 25;
		x ^= x >> 27;
		return x;
	}
private:
	result_type x;
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<> struct xorshift<xorshift_type::STAR_64> { 
	using result_type = uint64_t;
  static constexpr result_type min() { return std::numeric_limits<result_type>::min(); }
  static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }
	xorshift() : xorshift(seed<uint64_t>()) { }
	constexpr xorshift(result_type seed) noexcept : x(seed == 0 ? 1 : seed) { }
	constexpr result_type last() const noexcept { return x; }
	constexpr result_type operator()() noexcept {
		x ^= x >> 12; 
		x ^= x << 25;
		x ^= x >> 27;
		return x * 2685821657736338717ULL;
	}
private:
	result_type x;
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<> struct xorshift<xorshift_type::PLUS_128> { 
	using result_type = uint64_t;
  static constexpr result_type min() { return std::numeric_limits<result_type>::min(); }
  static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }
	xorshift() : xorshift(seed<uint64_t>()) { }
	constexpr xorshift(result_type seed) noexcept : xorshift(seed, seed) { }
	constexpr xorshift(result_type seed_high, result_type seed_low) noexcept : xl(seed_low == 0 ? 1 : seed_low), xh(seed_high == 0 ? 1 : seed_high) { }
	constexpr result_type last() const noexcept { return xl + xh; }
	constexpr result_type operator()() noexcept {
		      auto cpy_xl = xl; 
		const auto cpy_xh = xh;
		xl = cpy_xh;
		cpy_xl ^= cpy_xl << 23;
		cpy_xl ^= cpy_xl >> 17;
		cpy_xl ^= cpy_xh ^ (cpy_xh >> 26);
		xh = cpy_xl;
		return xl + xh;
	}
private:
	result_type xl, xh;
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<> struct xorshift<xorshift_type::STAR_1024> { 
	constexpr static std::size_t buff_len = 16;
	using result_type = uint64_t;
  static constexpr result_type min() { return std::numeric_limits<result_type>::min(); }
  static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }
	xorshift() : xorshift(seed<uint64_t>()) { }
	constexpr xorshift(result_type seed) noexcept : buff() { 
		fill(buff, xorshift<xorshift_type::NORM_64>(seed == 0 ? 1 : seed));
	}
	constexpr xorshift(const result_type (&seed)[buff_len]) noexcept : buff() { 
		bool has_non_zero = false;
		for(std::size_t i = 0; i != buff_len; i++) {
			has_non_zero |= seed[i];
			buff[i] = seed[i];
		}
		if(!has_non_zero) buff[0] = 1;
	}
	constexpr result_type last() const noexcept { return buff[p] * 1181783497276652981ULL; }
	constexpr result_type operator()() noexcept {
		auto xl = buff[p]; 
		auto xh = buff[p = (p+1) & (buff_len - 1)];
		xh ^= xh << 31;
		xh ^= xh >> 11;
		xl ^= xl >> 30;
		return (buff[p] = xl ^ xh) * 1181783497276652981ULL;
	}
private:
	result_type buff[buff_len];
	unsigned char p = 0;
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<> struct xorshift<xorshift_type::STAR_4096> { 
	constexpr static std::size_t buff_len = 64;
	using result_type = uint64_t;
  static constexpr result_type min() { return std::numeric_limits<result_type>::min(); }
  static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }
	xorshift() : xorshift(seed<uint64_t>()) { }
	constexpr xorshift(result_type seed) noexcept : buff() { 
		fill(buff, xorshift<xorshift_type::NORM_64>(seed == 0 ? 1 : seed));
	}
	constexpr xorshift(const result_type (&seed)[buff_len]) noexcept : buff() { 
		bool hasNonZero = false;
		for(std::size_t i = 0; i != buff_len; i++) {
			hasNonZero |= seed[i];
			buff[i] = seed[i];
		}
		if(!hasNonZero) buff[0] = 1;
	}
	constexpr result_type last() const noexcept { return buff[p] * 8372773778140471301ULL; }
	constexpr result_type operator()() noexcept {
		auto xl = buff[p]; 
		auto xh = buff[p = (p+1) & (buff_len - 1)];
		xh ^= xh << 25;
		xh ^= xh >>  3;
		xl ^= xl >> 49;
		return (buff[p] = xl ^ xh) * 8372773778140471301ULL;
	}
private:
	result_type buff[buff_len];
	unsigned char p = 0;
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** @brief pseudo random number generator for signed numbers and those that are smaller that 64 bits.
  * @tparam T type of random numbers that are generated by the instances of this type
	* @tparam R type of the underlying (pseudo) random number generator */ 
template<typename T, typename R>
struct general_xor_shift {
  using result_type = T;
  using underlying_type = typename R::result_type;
  static_assert(sizeof(underlying_type) >= sizeof(T));
  static_assert( std::is_unsigned<underlying_type>::value);
  static_assert( std::is_integral<underlying_type>::value);
  static_assert( std::is_integral<T>::value);
  static_assert(!std::is_same<T,bool>::value);
  using unsigned_T = std::make_unsigned_t<T>;
  template<typename ...Args>
  constexpr general_xor_shift(pbv_t<Args>...args) : delegate(T(args...))          { }
  constexpr general_xor_shift(const R&  delegate) : delegate(          delegate ) { }
  constexpr general_xor_shift(      R&& delegate) : delegate(std::move(delegate)) { }
	constexpr T last() const noexcept { return convert(delegate.last()); }
	constexpr T operator()() noexcept { return convert(delegate     ()); }

  /** @note if the return type is signed, the smallest possible value will never be returned. */
  constexpr T convert(pbv_t<typename R::result_type> value) noexcept {
    if(std::is_unsigned<T>::value) return static_cast<T>(value);
    const auto ret = static_cast<T>(value & (std::numeric_limits<unsigned_T>::max() >> 1));
    return value & ~(std::numeric_limits<underlying_type>::max() >> 1) ? ret : -ret;
  }
private:
  R delegate;
};

/** @brief pseudo random boolean generator.
	* @tparam R type of the underlying (pseudo) random number generator */
template<typename R>
struct general_xor_shift<bool, R> {
  using result_type = bool;
  template<typename ...Args>
  constexpr general_xor_shift(pbv_t<Args>...args) : delegate(T(args...))          { }
  constexpr general_xor_shift(const R&  delegate) : delegate(          delegate ) { }
  constexpr general_xor_shift(      R&& delegate) : delegate(std::move(delegate)) { }
	constexpr bool last() const noexcept { return delegate.last() & 1; }
	constexpr bool operator()() noexcept { return delegate     () & 1; }
private:
  R delegate;
};


//-------------------------------------------------------------------------------------------------------------------------------------------------------------

} // namespace cmn::math

#endif // CMN__MATH__XORSHIFT__HPP