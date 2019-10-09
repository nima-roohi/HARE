#ifndef __CMN__LOGGER_HPP__
#define __CMN__LOGGER_HPP__

#include <boost/predef/os/linux.h>    // BOOST_OS_LINUX
#include <boost/predef/os/macos.h>    // BOOST_OS_MACOS
#include <boost/predef/os/windows.h>  // BOOST_OS_WINDOWS
#include <boost/preprocessor/comparison/equal.hpp>
#include <boost/preprocessor/comparison/less_equal.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/stringize.hpp>

#include <cstddef>      // nullptr_t
#include <cstring>      // strrchr, strcmp
#include <ctime>        // time_t, localtime, tm
#include <iomanip>      // put_time
#include <iostream>     // clog
#include <ostream>      // ostream, endl
#include <sstream>      // stringstream
#include <utility>      // move, forward

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#ifdef CMN_LOG_ENABLE_RUNTIME_FILTER
#include "cmn/loc_filter.hpp"
#else
#include "cmn/loc_filter_dummy.hpp"
#endif

#ifdef CMN_LOG_ENABLE_RUNTIME_FILTER
namespace cmn::logger::detail { extern ::cmn::detail::loc_filter runtime_filter; }
namespace cmn::logger         { inline auto filter = std::move(detail::runtime_filter); }
#else
namespace cmn::logger::detail { extern ::cmn::detail::dummy_loc_filter dummy_runtime_filter; }
namespace cmn::logger         { inline auto filter = std::move(detail::dummy_runtime_filter); }
#endif

#ifdef CMN_LOG_FILTERS_FILE
namespace cmn::logger::detail { auto const dummy = filter.load_from_file(BOOST_PP_STRINGIZE(CMN_LOG_FILTERS_FILE)); }
#endif

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#ifdef CMN_LOG_LEVEL_TRACE
#error "Macro 'CMN_LOG_LEVEL_TRACE' has been already defined"
#endif
#ifdef CMN_LOG_LEVEL_DEBUG
#error "Macro 'CMN_LOG_LEVEL_DEBUG' has been already defined"
#endif
#ifdef CMN_LOG_LEVEL_INFO
#error "Macro 'CMN_LOG_LEVEL_INFO' has been already defined"
#endif
#ifdef CMN_LOG_LEVEL_WARN
#error "Macro 'CMN_LOG_LEVEL_WARN' has been already defined"
#endif
#ifdef CMN_LOG_LEVEL_ERROR
#error "Macro 'CMN_LOG_LEVEL_ERROR' has been already defined"
#endif
#ifdef CMN_LOG_LEVEL_FATAL
#error "Macro 'CMN_LOG_LEVEL_FATAL' has been already defined"
#endif
#ifdef CMN_LOG_LEVEL_OFF
#error "Macro 'CMN_LOG_LEVEL_OFF' has been already defined"
#endif

#define CMN_LOG_LEVEL_TRACE  0
#define CMN_LOG_LEVEL_DEBUG  1
#define CMN_LOG_LEVEL_INFO   2
#define CMN_LOG_LEVEL_WARN   3
#define CMN_LOG_LEVEL_ERROR  4
#define CMN_LOG_LEVEL_FATAL  5
#define CMN_LOG_LEVEL_OFF    6

/** @brief Default log-level (clients can change)
  * @requires It must be set to one of the log-levels */
#ifndef CMN_LOG_LEVEL_DEFAULT
#define CMN_LOG_LEVEL_DEFAULT CMN_LOG_LEVEL_TRACE
#endif

/** @brief Current log-level (clients can change)
  * @requires It must be set to one of the log-levels */
#ifndef CMN_LOG_LEVEL
#define CMN_LOG_LEVEL CMN_LOG_LEVEL_DEFAULT
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/** @brief Default macro for file name (clients can change)
  * @requires Value of this macro is given to an output stream, so whatever it is, it must be acceptable by that. */
#ifndef CMN_LOG_FILENAME
  // CMN_LOG_FILENAME contains the filename in __FILE__ without the path
  #if BOOST_OS_LINUX || BOOST_OS_MACOS
    #define CMN_LOG_FILENAME (::std::strrchr(__FILE__, '/') ? ::std::strrchr(__FILE__, '/') + 1 : __FILE__)
  #elif BOOST_OS_WINDOWS
    #define CMN_LOG_FILENAME (::std::strrchr(__FILE__, '\\') ? ::std::strrchr(__FILE__, '\\') + 1 : __FILE__)
  #else
    #define CMN_LOG_FILENAME __FILE__
  #endif
#endif

/** @brief Default macro for location of the to-be-logged event (clients can change)
  * @requires Value of this macro is given to an output stream, so whatever it is, it must be acceptable by the output stream. */
#ifndef CMN_LOG_PREFIX
#define CMN_LOG_PREFIX << "[" << CMN_LOG_FILENAME << ":" << __FUNCTION__ << ":" <<  __LINE__ << "] "
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/** @brief What output stream should be used for logging (clients can change)
  * @requires Must behave like an output stream */
#ifndef CMN_LOG_OUT
#define CMN_LOG_OUT ::std::clog
#endif

/** @brief What should be used for newline (clients can change)
  * @requires Value of this macro is given to an output stream, so whatever it is, it must be acceptable by the output stream.
  * @note According to <a href="https://en.cppreference.com/w/cpp/io/manip/endl">C++ Reference</a>:
  *       "use of <tt>std::endl</tt> in place of <tt>\\n</tt>, encouraged by some sources, may significantly degrade output performance". */
#ifndef CMN_LOG_NEWLINE
#define CMN_LOG_NEWLINE ::std::endl
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/** @brief String used for the second element in the call-stack (1st element has no indent) (clients can change)
  * @requires Value of this macro is given to an output stream, so whatever it is, it must be acceptable by the output stream. */
#ifndef CMN_LOG_INDENT_STR1
#define CMN_LOG_INDENT_STR1 " ."
#endif

/** @brief String used for the 3rd element in the call-stack and whatever comes after that (clients can change)
  * @requires Value of this macro is given to an output stream, so whatever it is, it must be acceptable by the output stream. */
#ifndef CMN_LOG_INDENT_STR
#define CMN_LOG_INDENT_STR ".."
#endif

/** @brief Whether or not indentation is enabled (must be 0 or 1) (clients can change) */
#ifndef CMN_LOG_INDENT_ENABLED
#define CMN_LOG_INDENT_ENABLED 0
#endif

namespace cmn::logger::detail {
extern thread_local unsigned indent_size;
struct indent {
   indent() { indent_size++; }
  ~indent() { indent_size--; }
  template<class R>
  static R&& put(R&& out) {
    if(indent_size > 1) {
      out << CMN_LOG_INDENT_STR1;
      for(unsigned i = 2; i < indent_size; i++)
        out << CMN_LOG_INDENT_STR; }
    return std::forward<R>(out); } };
} // namespace cmn::logger::detail

/** Helper macros used to create a fresh variable name*/
#define CMN_LOG_VAR_COMBINE1(X,Y) X##Y
#define CMN_LOG_VAR_COMBINE(X,Y) CMN_LOG_VAR_COMBINE1(X,Y)

#ifdef CMN_LOG_INDENT_INC_RAII
#error "Macro 'CMN_LOG_INDENT_INC_RAII' has been already defined"
#endif
/** @brief Define a fresh variable using the current line number.
  * @note Variable name is cmn_log_indent_raii_<line-number>. Therefore, there must be at most one of this macro on every line of the same block. */
#define CMN_LOG_INDENT_INC_RAII ::cmn::logger::detail::indent const CMN_LOG_VAR_COMBINE(cmn_log_indent_raii_,__LINE__) {};

#ifdef CMN_LOG_ADD_INDENT
#error "Macro 'CMN_LOG_ADD_INDENT' has been already defined"
#endif
#define CMN_LOG_ADD_INDENT(rec)   \
  BOOST_PP_IF(CMN_LOG_INDENT_ENABLED,CMN_LOG_INDENT_INC_RAII ::cmn::logger::detail::indent::put(rec),rec)

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
namespace cmn::logger::detail {
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

// Formatting codes
constexpr const char* reset      = "\033[00m";
constexpr const char* bold       = "\033[1m";
constexpr const char* dark       = "\033[2m";
constexpr const char* underline  = "\033[4m";
constexpr const char* blink      = "\033[5m";
constexpr const char* reverse    = "\033[7m";
constexpr const char* concealed  = "\033[8m";

// Foreground colors
constexpr const char* black      = "\033[30m";
constexpr const char* red        = "\033[31m";
constexpr const char* green      = "\033[32m";
constexpr const char* yellow     = "\033[33m";
constexpr const char* blue       = "\033[34m";
constexpr const char* magenta    = "\033[35m";
constexpr const char* cyan       = "\033[36m";
constexpr const char* white      = "\033[37m";

// Background colors
constexpr const char* bg_black   = "\033[40m";
constexpr const char* bg_red     = "\033[41m";
constexpr const char* bg_green   = "\033[42m";
constexpr const char* bg_yellow  = "\033[43m";
constexpr const char* bg_blue    = "\033[44m";
constexpr const char* bg_magenta = "\033[45m";
constexpr const char* bg_cyan    = "\033[46m";
constexpr const char* bg_white   = "\033[47m";

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/** @brief Simple log record.
  * @note  Instances are thread-safe iff instances of \c buff are thread-safe.
  * @note  Destructor is not marked \c virtual for efficiency, so unless \c buff has a virtual destructor, instances must not be casted to \c buff. */
template<class buff>
struct log_record : buff {
  #ifdef CMN_LOG_ENABLE_RUNTIME_FILTER
  log_record(char unsigned const level) : level(level) { }
  log_record(log_record&& rec) : buff(std::move(rec)), level(rec.level) { rec.logged = true; }
  #else 
  log_record() { }
  log_record(log_record&& rec) : buff(std::move(rec)) { rec.logged = true; }
  #endif
  log_record(const log_record&) = delete;
  log_record& operator=(const log_record& ) = delete;
  log_record& operator=(      log_record&&) = delete;
  log_record& check_match(std::string const& file, std::string const& func) {
    #ifdef CMN_LOG_ENABLE_RUNTIME_FILTER
      if(filter.get_level() > level || !filter.is_match(file + "::" + func)) 
        logged = true;
    #endif
    return *this; }
 ~log_record() {
    if(logged) return;
    logged = true;
    *this << reset << CMN_LOG_NEWLINE;
    CMN_LOG_OUT << this->str(); }
private:
  bool logged = false;
  #ifdef CMN_LOG_ENABLE_RUNTIME_FILTER
  char unsigned const level;
  #endif
};

#ifdef CMN_LOG_ADD_CHECK_MATCH
#error "Macro 'CMN_LOG_ADD_CHECK_MATCH' has ben already defined." 
#endif 

#ifdef CMN_LOG_ENABLE_RUNTIME_FILTER
#define CMN_LOG_ADD_CHECK_MATCH .check_match(__FILE__,__FUNCTION__)
#else
#define CMN_LOG_ADD_CHECK_MATCH 
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/** @brief Extremely simple, hence super fast non-configurable logger.
  * @note  Instances are thread-safe. */
template<class buff = std::stringstream>
struct default_logger {
  using record_t = log_record<buff>;
  template<class R>
  static R&& add_time(R&& rec) {
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);
    rec << std::put_time(&tm, "%T");
    return std::forward<R>(rec);  }
  #ifdef CMN_LOG_INIT_RECORD
  #error "Macro 'CMN_LOG_INIT_RECORD' has been already defined."
  #endif
  #ifdef CMN_LOG_ENABLE_RUNTIME_FILTER
  #define CMN_LOG_INIT_RECORD(level) record_t r{level}
  #else
  #define CMN_LOG_INIT_RECORD(level) record_t r{}
  #endif
  record_t trace() { CMN_LOG_INIT_RECORD(CMN_LOG_LEVEL_TRACE); add_time(r <<         magenta         ); return std::move(r); }
  record_t debug() { CMN_LOG_INIT_RECORD(CMN_LOG_LEVEL_DEBUG); add_time(r <<         cyan            ); return std::move(r); }
  record_t info () { CMN_LOG_INIT_RECORD(CMN_LOG_LEVEL_INFO ); add_time(r << bold << green           ); return std::move(r); }
  record_t warn () { CMN_LOG_INIT_RECORD(CMN_LOG_LEVEL_WARN ); add_time(r << bold << yellow          ); return std::move(r); }
  record_t error() { CMN_LOG_INIT_RECORD(CMN_LOG_LEVEL_ERROR); add_time(r << bold << red             ); return std::move(r); }
  record_t fatal() { CMN_LOG_INIT_RECORD(CMN_LOG_LEVEL_FATAL); add_time(r << bold           << bg_red); return std::move(r); }
  #undef CMN_LOG_INIT_RECORD
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/** @brief A logger that logs nothing */
inline std::ostream null_logger(nullptr);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
} // namespace cmn::logger::detail
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** @brief Logger (clients can change)
  * @note The default value provides a thread-safe simple and fast logger.
  * @requires It must provide <tt>trace, debug, info, warn, error, fatal</tt> functions. None of these functions take any parameters, none of them are
  *           \c const and each of them return a log-record. */
#ifndef CMN_LOGGER
#define CMN_LOGGER ::cmn::logger::detail::default_logger<>()
#endif

/** @brief A helper macro that returns the `reset formatting code'. */
#ifndef CMN_LOG_RESET_FMT
#define CMN_LOG_RESET_FMT ::cmn::logger::detail::reset
#endif


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* @note If \c %CMN_LOG_INDENT_ENABLED is 1 then enabled logs produce variables with the current line number encoded in their name. Therefore,
 *       it is recommended to have at most one \c log_xxx at every line. */

#define log_trace BOOST_PP_IF(BOOST_PP_LESS_EQUAL(CMN_LOG_LEVEL, CMN_LOG_LEVEL_TRACE), CMN_LOG_ADD_INDENT(CMN_LOGGER.trace()CMN_LOG_ADD_CHECK_MATCH) << " " CMN_LOG_PREFIX, ::cmn::logger::detail::null_logger)
#define log_debug BOOST_PP_IF(BOOST_PP_LESS_EQUAL(CMN_LOG_LEVEL, CMN_LOG_LEVEL_DEBUG), CMN_LOG_ADD_INDENT(CMN_LOGGER.debug()CMN_LOG_ADD_CHECK_MATCH) << " " CMN_LOG_PREFIX, ::cmn::logger::detail::null_logger)
#define log_info  BOOST_PP_IF(BOOST_PP_LESS_EQUAL(CMN_LOG_LEVEL, CMN_LOG_LEVEL_INFO ), CMN_LOG_ADD_INDENT(CMN_LOGGER.info ()CMN_LOG_ADD_CHECK_MATCH) << " " CMN_LOG_PREFIX, ::cmn::logger::detail::null_logger)
#define log_warn  BOOST_PP_IF(BOOST_PP_LESS_EQUAL(CMN_LOG_LEVEL, CMN_LOG_LEVEL_WARN ), CMN_LOG_ADD_INDENT(CMN_LOGGER.warn ()CMN_LOG_ADD_CHECK_MATCH) << " " CMN_LOG_PREFIX, ::cmn::logger::detail::null_logger)
#define log_error BOOST_PP_IF(BOOST_PP_LESS_EQUAL(CMN_LOG_LEVEL, CMN_LOG_LEVEL_ERROR), CMN_LOG_ADD_INDENT(CMN_LOGGER.error()CMN_LOG_ADD_CHECK_MATCH) << " " CMN_LOG_PREFIX, ::cmn::logger::detail::null_logger)
#define log_fatal BOOST_PP_IF(BOOST_PP_LESS_EQUAL(CMN_LOG_LEVEL, CMN_LOG_LEVEL_FATAL), CMN_LOG_ADD_INDENT(CMN_LOGGER.fatal()CMN_LOG_ADD_CHECK_MATCH) << " " CMN_LOG_PREFIX, ::cmn::logger::detail::null_logger)

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif //__CMN__LOGGER_HPP__
