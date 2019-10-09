#ifndef CMN__MISC__SIMPLE_SHARED_PTR__HPP
#define CMN__MISC__SIMPLE_SHARED_PTR__HPP

#include "cmn/dbc.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <type_traits>

namespace cmn::misc {

/** @tparam T type of object instances will refer to
  * @tparam C type of counter
  * @note instances that shares the same pointer are not thread-safe (so the result code have a chance to run faster). */
template<typename T,
         typename C = std::uint64_t> 
struct simple_shared_ptr {
  using type = simple_shared_ptr;
  using value_type = T;
  static_assert( std::is_integral<C>::value);
  static_assert( std::is_unsigned<C>::value);
  static_assert(!std::is_same<C, bool>::value);

  template<typename UU, typename TT, typename CC> 
  friend simple_shared_ptr<UU,CC> static_pointer_cast(const simple_shared_ptr<TT,CC>& that) noexcept;

private:
  simple_shared_ptr(T* const ptr, C* const counter) : ptr(ptr), counter(counter) { }

public:

  constexpr simple_shared_ptr() : ptr(nullptr), counter(nullptr) { }
  
  /** It is guaranteed that the ptr will be deleted if the constructor encounters an exception. */
  simple_shared_ptr(T* const ptr) : ptr(ptr), counter(nullptr) {
    requires(ptr != nullptr)
    safe_alloc_counter();
    inc_counter(); }

  simple_shared_ptr(const simple_shared_ptr<T,C>&  that) noexcept : ptr(that.ptr), counter(that.counter) { 
    requires(that.ptr != nullptr ^ that.counter == nullptr)
    if(that.ptr != nullptr) 
      inc_counter(); } 
  
  simple_shared_ptr(simple_shared_ptr<T,C>&& that) noexcept : ptr(that.ptr), counter(that.counter) { 
    requires(that.ptr != nullptr ^ that.counter == nullptr)
    that.ptr     = nullptr; 
    that.counter = nullptr; }

  template<typename U>
  simple_shared_ptr(const simple_shared_ptr<U,C>&  that) noexcept : ptr(that.ptr), counter(that.counter) { 
    static_assert(std::is_base_of<T,U>::value);
    requires(that.ptr != nullptr ^ that.counter == nullptr)
    if(that.ptr != nullptr) 
      inc_counter(); } 
  
  template<typename U>
  simple_shared_ptr(simple_shared_ptr<U,C>&& that) noexcept : ptr(that.ptr), counter(that.counter) { 
    static_assert(std::is_base_of<T,U>::value);
    requires(that.ptr != nullptr ^ that.counter == nullptr)
    that.ptr     = nullptr; 
    that.counter = nullptr; }

  ~simple_shared_ptr() { clear(); }
  
  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  type& operator=(const type& that) noexcept { 
    if(this != &that) {
      requires(that.ptr != nullptr ^ that.counter == nullptr)
      clear();
      ptr     = that.ptr    ;
      counter = that.counter;
      if(that.ptr != nullptr) 
        inc_counter(); } 
    return *this; }

  type& operator=(type&& that) noexcept {
    if(this != &that) {
      clear();
      ptr     = that.ptr    ;
      counter = that.counter;
      that.ptr     = nullptr;
      that.counter = nullptr;  }
    return *this; }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  explicit operator bool() const noexcept { return ptr != nullptr; }
  bool unique() const noexcept { return counter != nullptr && *counter == 1; }

  T& operator* () const noexcept { requires(ptr != nullptr) return *ptr; }
  T* operator->() const noexcept { return  ptr; }

  bool operator==(const type& that) const { return (ptr == nullptr ^ that.ptr != nullptr) && (ptr == nullptr || *ptr == *that.ptr); }
  bool operator!=(const type& that) const { return (ptr == nullptr ^ that.ptr == nullptr) || (ptr != nullptr && *ptr != *that.ptr); }

  T* get()           const noexcept { return ptr;     }
  C use_count()      const noexcept { requires(counter!=nullptr) return *counter; }

  void reset() noexcept { clear2(); }

  /** \c ptr should not be already managed by any other instance 
    * It is guaranteed that the ptr will be deleted if the constructor encounters an exception. */
  void reset(T* const ptr) { 
    if(ptr == this->ptr) return;
    clear2();
    if(ptr == nullptr) return;
    this->ptr = ptr;
    safe_alloc_counter();
    inc_counter(); }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

private:

  void clear() noexcept {
    asserts_msg(counter != nullptr ^   ptr == nullptr, "counter == nullptr is " << (counter == nullptr) << ", ptr == nullptr is " << (ptr == nullptr))
    asserts    (counter == nullptr || *counter > 0)
    if(counter) {
      if(*counter <= 1) {
        asserts(*counter != 0);
        delete counter;
        delete ptr    ;
        counter = nullptr;
        ptr     = nullptr;
      } else (*counter)--; } }
  
  void clear2() noexcept {
    clear();
    counter = nullptr;
    ptr     = nullptr;  }

  void safe_alloc_counter() {
    struct safe {
      safe(T* ptr, C*& counter) : ptr(ptr), counter(counter) { }
      ~safe() { if(!counter) delete ptr; }
      T*     ptr;
      C*& counter;
    } instance(ptr, counter);
    counter = new C{0};
    inc_counter(); }

  void inc_counter() {
    asserts(counter != nullptr)
    requires(*counter < std::numeric_limits<C>::max()) 
    (*counter)++;  }

private:
  T*    ptr;
  C* counter;
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename T, typename C = std::uint64_t> 
auto make_shared(T* ptr) { return simple_shared_ptr<T,C>(ptr); }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename U, typename T, typename C> 
simple_shared_ptr<U,C> static_pointer_cast(const simple_shared_ptr<T,C>& that) noexcept {
  auto ptr = static_cast<U*>(that.ptr);
  auto res = simple_shared_ptr<U,C>(ptr, that.counter);
  res.inc_counter();
  return res; };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

} // namespace cmn::misc

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace std {

template<typename T, typename C>
struct hash<cmn::misc::simple_shared_ptr<T,C>> {
  using argument_type = cmn::misc::simple_shared_ptr<T,C>;
  using result_type   = std::size_t;
  result_type operator()(const argument_type& that) const { 
    return that ? hash<T>()(*that) : 1; } 
};

} // namespace std

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif // CMN__MISC__SIMPLE_SHARED_PTR__HPP
