
#include "cmn/loc_filter.hpp"

#include <boost/predef/os/linux.h>            // BOOST_OS_LINUX
#include <boost/predef/os/macos.h>            // BOOST_OS_MACOS
#include <boost/predef/os/windows.h>          // BOOST_OS_WINDOWS

#include <boost/algorithm/string/predicate.hpp> // starts_with
#include <boost/algorithm/string/trim.hpp>      // trim_copy, trim_left_copy

#include <cstdint>  // fixed-width integers
#include <cstring>  // strlen
#include <fstream>  // ifstream
#include <iostream> // cerr
#include <ostream>  // endl
#include <regex> 
#include <sstream>
#include <string>   // getline
#include <tuple>


//-------------------------------------------------------------------------------------------------------------------------------------------------------------

bool cmn::detail::loc_filter::is_match(std::string const& addr) {
  if(is_empty())
    return true;
  bool res = false;
  for(auto const& pair : stack)
    if(std::get<1>(pair))
      res |= std::regex_match(addr, std::get<0>(pair));
    else 
    if(std::regex_match(addr, std::get<0>(pair)))
      return false;
  return res; }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

void cmn::detail::loc_filter::add_regex_filter(std::string const& flt, bool const positive) {
  stack.emplace_back(std::regex(flt, std::regex::optimize | std::regex::extended | std::regex::nosubs), positive); }

void cmn::detail::loc_filter::add_method_filter(std::string const& flt, bool const positive) {
  add_regex_filter(".*::" + flt, positive); }

void cmn::detail::loc_filter::add_recursive_path_filter(std::string const& flt, bool const positive) {
  add_regex_filter(flt + ".*", positive); }

void cmn::detail::loc_filter::add_nonrecursive_path_filter(std::string const& flt, bool const positive) {
  auto const len = flt.length();
  if(len != 0 && flt[len-1] != '/' && flt[len-1] != '\\') {
    #if BOOST_OS_LINUX || BOOST_OS_MACOS
      add_regex_filter(flt + "/[^/\\\\]*::.*", positive);
    #elif BOOST_OS_WINDOWS
      add_regex_filter(flt + "\\\\[^/\\\\]*::.*", positive);
    #else
      static bool flag = true;
      if(flag) {
        flag = false;
        std::cerr << "Don't recognize operating system. Will add forward slash (/) to the end of input argument: " << flt << "."
                  << " This message won't be displayed anymore." << std::endl; }
      add_regex_filter(flt + "/[^/\\\\]*::.*", positive);
    #endif 
  } else 
    add_regex_filter(flt + "[^/\\\\]*::.*", positive); }

void cmn::detail::loc_filter::add_file_filter(std::string const& flt, bool const positive) {
  auto const pattern = flt.find(".") == std::string::npos ?
                       ".*" + flt + "\\.?[^./\\\\]*::.*"  :
                       ".*" + flt + "::.*"                ;
  add_regex_filter(pattern, positive); }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

void cmn::detail::loc_filter::remove_last_filter() {
  stack.pop_back(); }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

bool  cmn::detail::loc_filter::load_from_stream(std::istream& in) {
  // Trimmed substring starting from 1 after length of \c pre
  constexpr auto substr = [](std::string const& str, char const* const pre) {
    return boost::algorithm::trim_copy(str.substr(std::strlen(pre)+1)); };
  // Remove the optional first '-' or '+' and return \c true iff it does not start with '-'.
  constexpr auto remove_class = [](std::string& str) {
    if(str.empty()) return true;
    if(str.front() == '-') {
      str[0] = ' ';
      boost::algorithm::trim_left(str);
      return false; }
    if(str.front() == '+') {
      str[0] = ' ';
      boost::algorithm::trim_left(str); }
    return true; };
  // Open the file for reading
  // Actual loading starts here
  bool ignored = false;
  std::uint64_t idx = 0;
  for(std::string line; std::getline(in,line);) {
    idx++;
    auto       cpy      = boost::algorithm::trim_left_copy(line);
    bool const positive = remove_class(cpy);
    if( boost::algorithm::starts_with(cpy, "file"     )) add_file_filter             (substr(cpy,"file"     ),positive); else
    if( boost::algorithm::starts_with(cpy, "method"   )) add_method_filter           (substr(cpy,"method"   ),positive); else
    if( boost::algorithm::starts_with(cpy, "r-path"   )) add_recursive_path_filter   (substr(cpy,"r-path"   ),positive); else
    if( boost::algorithm::starts_with(cpy, "nr-path"  )) add_nonrecursive_path_filter(substr(cpy,"nr-path"  ),positive); else
    if( boost::algorithm::starts_with(cpy, "regex"    )) add_regex_filter            (substr(cpy,"regex"    ),positive); else
    if(!boost::algorithm::starts_with(cpy, "#") && cpy.length() > 0) {
      ignored = true;
      std::cerr << "Ignored line " << idx << ", since did not understand its filter type. Its content was: \"" << line << "\"." << std::endl; } }
  return !ignored; }

bool cmn::detail::loc_filter::load_from_file(std::string const& filename) {
  std::ifstream in(filename);
  if(!in) {
    std::cerr << "Could not open the input file '" << filename << "' for reading." << std::endl;
    return false; }
  try { return load_from_stream(in); }
  catch(...) { in.close(); } 
  return false; }

bool cmn::detail::loc_filter::load_from_string(std::string const& str) {
  std::stringstream buff(str);
  return load_from_stream(buff); }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
