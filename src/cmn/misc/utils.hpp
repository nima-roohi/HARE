#ifndef CMN__MISC__UTILS__HPP
#define CMN__MISC__UTILS__HPP

#include "cmn/dbc.hpp"

#include <boost/algorithm/string/trim.hpp>

#include <algorithm>
#include <functional>   // ptr_fun
#include <cctype>       // isspace
#include <cstdint>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace cmn::misc::utils {

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename Out, typename Str>
Out& indent(Out& out, const std::uint64_t size, const Str& str) { 
  for(std::uint64_t i = 0; i < size; i++) out << str; 
  return out; }

template<typename Out>
Out& indent(Out& out, const std::uint64_t size) {  return indent<Out, char>(out, size, ' '); }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** trim from the left */
inline std::string ltrim(const std::string& str) {
  return boost::algorithm::trim_left_copy(str);
}

/** trim from the right */
inline std::string rtrim(const std::string& str) {
  return boost::algorithm::trim_right_copy(str);
}

/** trim from the right */
inline std::string trim(const std::string& str) {
  return boost::algorithm::trim_copy(str);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename Str, typename Code>
void apply_to_tokens(const Str& str, const Str& sep, const Code& code) noexcept(noexcept(code(str))) {
  typename Str::size_type p1 = 0;
  typename Str::size_type p2 = 0;
  const auto sep_len = sep.length();
  while((p2 = str.find(sep, p1)) != Str::npos) {
    code(str.substr(p1, p2 - p1));
    p1 = p2 + sep_len; }
  code(str.substr(p1));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** @brief replace all occurrences of \c what in \c str with \c that */
template<typename Str>
Str replace_all(const Str& str, const Str& what, const Str& that) {
  requires_msg(!what.empty(), "cannot replace occurrences of the empty string")
  typename Str::size_type pos = 0;
  const auto what_size = what.size();
  const auto that_size = that.size();
  auto res = str;
  while((pos = res.find(what, pos)) != Str::npos) { 
    res.replace(pos, what_size, that);
    pos += that_size; } 
  return res; }

/** @brief for every pair of <tt>(what, that)</tt> in the input map, replace all occurrences of \c what with \c that.
  * @note replacement will be atomic and the order of elements does not matter. 
  * @note input sequence of kvs must be sorted strictly decreasingly and there should be no duplicate key in them */
template<typename Str>
Str replace_all(const Str& str, const std::vector<std::tuple<Str, Str>> kvs) {
  requires(std::is_sorted    (kvs.begin(), kvs.end(), [](const auto& fst, const auto& snd) { return std::get<0>(fst) >  std::get<0>(snd); } ) )
  requires(std::adjacent_find(kvs.begin(), kvs.end(), [](const auto& fst, const auto& snd) { return std::get<0>(fst) == std::get<0>(snd); } ) == kvs.end() )
  requires(std::all_of(kvs.begin(), kvs.end(), [](const auto& kv) { return !std::get<0>(kv).empty(); } ) )

  using pos_t = typename Str::size_type;
  std::vector<std::tuple<pos_t /* position inside the string */, pos_t /* key length */, Str /* value */>> positions;
  pos_t pos = 0;
  while(pos != Str::npos) {
    bool found = false;
    auto candid = std::tuple<pos_t, pos_t, Str>(Str::npos, Str::npos, Str());
    for(auto kv : kvs) { 
      const auto p = str.find(std::get<0>(kv), pos);
      if(p != Str::npos && (!found || p < std::get<0>(candid))) {
        found = true;
        candid = std::make_tuple(p, std::get<0>(kv).length(), std::get<1>(kv) ); } }
    if(found) {
      pos = std::get<0>(candid) + std::get<1>(candid); 
      positions.push_back(std::move(candid));
    } else break; }
  std::sort(positions.begin(), positions.end(), [](const auto& fst, const auto& snd){ return std::get<0>(fst) > std::get<0>(snd); }); 
  auto res = str;
  for(const auto& tuple : positions)
    res.replace(std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple));
  return res; 
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

} // namespace cmn::misc::utils

#endif // CMN__MISC__UTILS__HPP
