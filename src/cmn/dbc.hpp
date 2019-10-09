#ifndef __CMN__DBC_HPP__
#define __CMN__DBC_HPP__

#include "cmn/logger.hpp"
#include "cmn/type_traits.hpp"

// #define _GNU_SOURCE
#define BOOST_STACKTRACE_LINK
#include <boost/stacktrace.hpp>

#include <cstdlib>    // abort
#include <functional> // function
#include <ostream>    // endl
#include <stdexcept>  // logic_error
#include <string>

#ifndef CMN__ADD_STACK_TRACE
#define CMN__ADD_STACK_TRACE CMN_LOG_RESET_FMT << std::endl << boost::stacktrace::stacktrace()
#endif

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace cmn {

struct dbc_error : public std::logic_error {
  explicit dbc_error()                        : std::logic_error("")  { }
  explicit dbc_error(const char*         msg) : std::logic_error(msg) { }
  explicit dbc_error(const std::string&  msg) : std::logic_error(msg) { }
  explicit dbc_error(      std::string&& msg) : std::logic_error(std::forward<std::string>(msg)) { }
};

struct asserts_error : public dbc_error {
  explicit asserts_error()                        : dbc_error("")  { }
  explicit asserts_error(const char*         msg) : dbc_error(msg) { }
  explicit asserts_error(const std::string&  msg) : dbc_error(msg) { }
  explicit asserts_error(      std::string&& msg) : dbc_error(std::forward<std::string>(msg)) { }
};

struct requires_error : public dbc_error {
  explicit requires_error()                        : dbc_error("")  { }
  explicit requires_error(const char*         msg) : dbc_error(msg) { }
  explicit requires_error(const std::string&  msg) : dbc_error(msg) { }
  explicit requires_error(      std::string&& msg) : dbc_error(std::forward<std::string>(msg)) { }
};

struct ensures_error : public dbc_error {
  explicit ensures_error()                        : dbc_error("")  { }
  explicit ensures_error(const char*         msg) : dbc_error(msg) { }
  explicit ensures_error(const std::string&  msg) : dbc_error(msg) { }
  explicit ensures_error(      std::string&& msg) : dbc_error(std::forward<std::string>(msg)) { }
};
  
} // namespace cmn

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//#define CMN_DISABLE_DBC       // uncomment this line to completely disable DBC checks
#ifndef CMN_DISABLE_DBC

namespace cmn {
  template<typename R> inline const char*         err_msg(pbv_t<R>    , const char*         msg) { return msg; }
  template<typename R> inline const std::string&  err_msg(pbv_t<R>    , const std::string&  msg) { return msg; }
  template<typename R> inline       std::string&& err_msg(pbv_t<R>    ,       std::string&& msg) { return std::forward<std::string>(msg); }
  template<typename R> inline       std::string   err_msg(pbv_t<R> res, const std::function<std::string(pbv_t<R>)>& msg) { return msg(res); }
} // namespace cmn

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#define throw_trace(exp) { log_trace << #exp << CMN__ADD_STACK_TRACE; throw exp; }
#define throw_debug(exp) { log_debug << #exp << CMN__ADD_STACK_TRACE; throw exp; }
#define throw_info(exp)  { log_info  << #exp << CMN__ADD_STACK_TRACE; throw exp; }
#define throw_warn(exp)  { log_ward  << #exp << CMN__ADD_STACK_TRACE; throw exp; }
#define throw_error(exp) { log_error << #exp << CMN__ADD_STACK_TRACE; throw exp; }
#define throw_fatal(exp) { log_fatal << #exp << CMN__ADD_STACK_TRACE; throw exp; }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#define asserts(expr)                                                                                                 \
  if(!(expr)) {                                                                                                       \
    log_fatal << "Assertion Failed: " #expr << CMN__ADD_STACK_TRACE;                                                  \
    ::std::abort(); }
#define asserts_msg(expr, msg)                                                                                        \
  if(!(expr)) {                                                                                                       \
    log_fatal << "Assertion Failed: " #expr " :: " << msg << CMN__ADD_STACK_TRACE;                                    \
    ::std::abort(); } 
#define asserts_pred(expr, pred)                                                                                      \
  if(const auto eval = expr; !pred(eval)) {                                                                           \
    log_fatal << "Assertion Failed: " #expr " :: " #pred " :: eval=" << eval << CMN__ADD_STACK_TRACE;                 \
    ::std::abort(); }
#define asserts_res(res, pred) ({                                                                                     \
  auto result = res;                                                                                                  \
  if(!(pred(result))) {                                                                                               \
    log_fatal << "Assertion Failed: " #pred " :: result=" << result << CMN__ADD_STACK_TRACE;                          \
    ::std::abort(); }                                                                                                 \
  ::std::move(result);                                                                                                \
});  
#define asserts_res_msg(res, pred, msg) ({                                                                            \
  auto result = res;                                                                                                  \
  if(!(pred(result))) {                                                                                               \
    log_fatal << "Assertion Failed: " #pred " :: result=" << result << " :: "                                         \
                << ::cmn::err_msg<decltype(result)>(result, msg) << CMN__ADD_STACK_TRACE;                             \
    ::std::abort(); }                                                                                                 \
  ::std::move(result);                                                                                                \
}); 

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
#define requires(expr)                                                                                                \
  if(!(expr)) {                                                                                                       \
    log_fatal << "Requirement Failed: " #expr << CMN__ADD_STACK_TRACE;                                                \
    ::std::abort(); }
#define requires_msg(expr, msg)                                                                                       \
  if(!(expr)) {                                                                                                       \
    log_fatal << "Requirement Failed: " #expr << " :: " << msg << CMN__ADD_STACK_TRACE;                               \
    ::std::abort(); }
#define requires_pred(expr, pred)                                                                                     \
  if(const auto eval = expr; !pred(eval)) {                                                                           \
    log_fatal << "Requirement Failed: " #expr " :: " #pred " :: eval=" << eval << CMN__ADD_STACK_TRACE;               \
    ::std::abort(); }
#define requires_res(res, pred) ({                                                                                    \
  auto result = res;                                                                                                  \
  if(!(pred(result))) {                                                                                               \
    log_fatal << "Requirement Failed: " #pred " :: result=" << result << CMN__ADD_STACK_TRACE;                        \
    ::std::abort(); }                                                                                                 \
  ::std::move(result);                                                                                                \
}); 
#define requires_res_msg(res, pred, msg) ({                                                                           \
  auto result = res;                                                                                                  \
  if(!(pred(result))) {                                                                                               \
    log_fatal << "Requirement Failed: " #pred " :: result=" << result << " :: "                                       \
                << ::cmn::err_msg<decltype(result)>(result, msg) << CMN__ADD_STACK_TRACE;                             \
    ::std::abort(); }                                                                                                 \
  ::std::move(result);                                                                                                \
});  

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
#define ensures(expr)                                                                                                 \
  if(!(expr)) {                                                                                                       \
    log_fatal << "Ensuring Failed: " #expr << CMN__ADD_STACK_TRACE;                                                   \
    ::std::abort(); }
#define ensures_msg(expr, msg)                                                                                        \
  if(!(expr)) {                                                                                                       \
    log_fatal << "Ensuring Failed: " #expr << " :: " << msg << CMN__ADD_STACK_TRACE;                                  \
    ::std::abort(); }
#define ensures_pred(expr, pred)                                                                                      \
  if(const auto eval = expr; !pred(eval)) {                                                                           \
    log_fatal << "Ensuring Failed: " #expr " :: " #pred " :: eval=" << eval << CMN__ADD_STACK_TRACE;                  \
    ::std::abort(); }
#define ensures_res(res, pred) ({                                                                                     \
  auto result = res;                                                                                                  \
  if(!(pred(result))) {                                                                                               \
    log_fatal << "Ensuring Failed: " #pred " :: result=" << result << CMN__ADD_STACK_TRACE;                           \
    ::std::abort(); }                                                                                                 \
  ::std::move(result);                                                                                                \
});  
#define ensures_res_msg(res, pred, msg) ({                                                                            \
  auto result = res;                                                                                                  \
  if(!(pred(result))) {                                                                                               \
    log_fatal << "Ensuring Failed: " #pred " :: result=" << result << " :: "                                          \
                << ::cmn::err_msg<decltype(result)>(result, msg) << CMN__ADD_STACK_TRACE;                             \
    ::std::abort(); }                                                                                                 \
  ::std::move(result);                                                                                                \
});  

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
#define t_asserts(expr)                                                                                               \
  if(!(expr))                                                                                                         \
    throw ::cmn::asserts_error("Assertion Failed: " #expr);                                         
#define t_asserts_msg(expr, msg)                                                                                      \
  if(!(expr)) {                                                                                                       \
    auto out = ::std::stringstream();                                                                                 \
    out << "Assertion Failed: " #expr << " :: " << msg;                                                               \
    throw ::cmn::asserts_error(out.str()); }
#define t_asserts_pred(expr, pred)                                                                                    \
  if(const auto eval = expr; !pred(eval)) {                                                                           \
    auto out = ::std::stringstream();                                                                                 \
    out << "Assertion Failed: " #expr " :: " #pred " :: eval=" << eval;                                               \
    throw ::cmn::asserts_error(out.str()); }
#define t_asserts_res(res, pred) ({                                                                                   \
  auto result = res;                                                                                                  \
  if(!(pred(result))) {                                                                                               \
    auto out = ::std::stringstream();                                                                                 \
    out << "Assertion Failed: " #pred " :: result=" << result;                                                        \
    throw ::cmn::asserts_error(out.str()); }                                                                          \
  ::std::move(result);                                                                                                \
});  
#define t_asserts_res_msg(res, pred, msg) ({                                                                          \
  auto result = res;                                                                                                  \
  if(!(pred(result))) {                                                                                               \
    auto out = ::std::stringstream();                                                                                 \
    out << "Assertion Failed: " #pred " :: result=" << result << " :: "                                               \
        << ::cmn::err_msg<decltype(result)>(result, msg);                                                             \
    throw ::cmn::asserts_error(out.str()); }                                                                          \
  ::std::move(result);                                                                                                \
});  

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
#define t_requires(expr)                                                                                              \
  if(!(expr))                                                                                                         \
    throw ::cmn::requires_error("Requirement Failed: " #expr);  
#define t_requires_msg(expr, msg)                                                                                     \
  if(!(expr)) {                                                                                                       \
    auto out = ::std::stringstream();                                                                                 \
    out << "Requirement Failed: " #expr << " :: " << msg;                                                             \
    throw ::cmn::requires_error(out.str());  }
#define t_requires_pred(expr, pred)                                                                                   \
  if(const auto eval = expr; !pred(eval)) {                                                                           \
    auto out = ::std::stringstream();                                                                                 \
    out << "Requirement Failed: " #expr " :: " #pred " :: eval=" << eval;                                             \
    throw ::cmn::requires_error(out.str()); }
#define t_requires_res(res, pred) ({                                                                                  \
  auto result = res;                                                                                                  \
  if(!(pred(result))) {                                                                                               \
    auto out = ::std::stringstream();                                                                                 \
    out << "Requirement Failed: " #pred " :: result=" << result;                                                      \
    throw ::cmn::requires_error(out.str());   }                                                                       \
  ::std::move(result);                                                                                                \
});  
#define t_requires_res_msg(res, pred, msg) ({                                                                         \
  auto result = res;                                                                                                  \
  if(!(pred(result))) {                                                                                               \
    auto out = ::std::stringstream();                                                                                 \
    out << "Requirement Failed: " #pred " :: result=" << result                                                       \
        << ::cmn::err_msg<decltype(result)>(result, msg);                                                             \
    throw ::cmn::requires_error(out.str());   }                                                                       \
  ::std::move(result);                                                                                                \
}); 

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
#define t_ensures(expr)                                                                                               \
  if(!(expr))                                                                                                         \
    throw ::cmn::ensures_error("Ensuring Failed: " #expr);                            
#define t_ensures_msg(expr, msg)                                                                                      \
  if(!(expr)) {                                                                                                       \
    auto out = ::std::stringstream();                                                                                 \
    out << "Ensuring Failed: " #expr << " :: " << msg;                                                                \
    throw ::cmn::ensures_error(out.str()); }  
#define t_ensures_pred(expr, pred)                                                                                    \
  if(const auto eval = expr; !pred(eval)) {                                                                           \
    auto out = ::std::stringstream();                                                                                 \
    out << "Ensuring Failed: " #expr " :: " #pred " :: eval=" << eval;                                                \
    throw ::cmn::ensures_error(out.str()); }
#define t_ensures_res(res, pred) ({                                                                                   \
  auto result = res;                                                                                                  \
  if(!(pred(result))) {                                                                                               \
    auto out = ::std::stringstream();                                                                                 \
    out << "Ensuring Failed: " #pred " :: result=" << result;                                                         \
    throw ::cmn::ensures_error(out.str()); }                                                                          \
  ::std::move(result);                                                                                                \
});  
#define t_ensures_res_msg(res, pred, msg) ({                                                                          \
  auto result = res;                                                                                                  \
  if(!(pred(result))) {                                                                                               \
    auto out = ::std::stringstream();                                                                                 \
    out << "Ensuring Failed: " #pred " :: result=" << result                                                          \
       << ::cmn::err_msg<decltype(result)>(result, msg);                                                              \
    throw ::cmn::ensures_error(out.str()); }                                                                          \
  ::std::move(result);                                                                                                \
});  

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#else  // CMN_DISABLE_DBC
  
  #define asserts(expr)
  #define asserts_msg(expr, msg)
  #define asserts_pred(expr, pred)
  #define asserts_res(res, pred)              res;
  #define asserts_res_msg(res, pred, msg)     res;
    
  #define requires(expr)
  #define requires_msg(expr, msg)
  #define requires_pred(expr, pred)
  #define requires_res(res, pred)             res;
  #define requires_res_msg(res, pred, msg)    res;
    
  #define ensures(expr)
  #define ensures_msg(expr, msg)
  #define ensures_pred(expr, pred)
  #define ensures_res(res, pred)              res;
  #define ensures_res_msg(res, pred, msg)     res;
  
  #define t_asserts(expr)
  #define t_asserts_msg(expr, msg)
  #define t_asserts_pred(expr, pred)
  #define t_asserts_res(res, pred)            res;
  #define t_asserts_res_msg(res, pred, msg)   res;
    
  #define t_requires(expr)
  #define t_requires_msg(expr, msg)
  #define t_requires_pred(expr, pred)
  #define t_requires_res(res, pred)           res;
  #define t_requires_res_msg(res, pred, msg)  res;
    
  #define t_ensures(expr)
  #define t_ensures_msg(expr, msg)
  #define t_ensures_pred(expr, pred)
  #define t_ensures_res(res, pred)            res;
  #define t_ensures_res_msg(res, pred, msg)   res;
    
#endif // CMN_DISABLE_DBC

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif // __CMN__DBC_HPP__
