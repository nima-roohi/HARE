/**************************************************************************************************
 *                           HARE: Hybrid Abstraction Refinement Engine                           *
 *                                                                                                *
 * Copyright (C) 2019                                                                             *
 * Authors:                                                                                       *
 *   Nima Roohi <nroohi@ucsd.edu> (University of California San Diego)                            *
 *                                                                                                *
 * This program is free software: you can redistribute it and/or modify it under the terms        *
 * of the GNU General Public License as published by the Free Software Foundation, either         *
 * version 3 of the License, or (at your option) any later version.                               *
 *                                                                                                *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;      *
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.      *
 * See the GNU General Public License for more details.                                           *
 *                                                                                                *
 * You should have received a copy of the GNU General Public License along with this program.     *
 * If not, see <https://www.gnu.org/licenses/>.                                                   *
 **************************************************************************************************/

#ifndef HA__PERFORMANCE_COUNTER__HPP
#define HA__PERFORMANCE_COUNTER__HPP

#include "cmn/type_traits.hpp"

#include "hare/safety_result.hpp"

#include <boost/mpl/at.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/map.hpp>
#include <boost/mpl/pair.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <locale>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace ha::pc {

using counter_t = std::uint_fast64_t;
using timer_t   = std::uint_fast64_t;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

struct DummyTimer { };

struct Timer {
  Timer(timer_t& metric) : start(current_time()), metric(&metric) { }

  Timer(Timer&      ) = delete;
  Timer(Timer&& that) : start(that.start), metric(that.metric) { that.metric = nullptr; }

  Timer& operator=(const Timer&      ) = delete;
  Timer& operator=(      Timer&& that) {
    if(this == &that) return *this;
         start  = that.start ;
         metric = that.metric;
    that.metric = nullptr;
    return *this; }

  ~Timer() { stop(); }

  void stop() {
    if(metric != nullptr) {
      *metric += current_time() - start;
      metric = nullptr; } }

  static timer_t current_time() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::high_resolution_clock::now().time_since_epoch()).count(); }

private:
  timer_t  start ;
  timer_t* metric;
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

struct serializer { 
  virtual void        initialize(      void* const ptr) const = 0;
  virtual std::string serialize (const void* const ptr) const = 0; 
};

struct : serializer {
  void        initialize(      void* const ptr) const { *static_cast<counter_t*>(ptr) = counter_t{}; }
  std::string serialize (const void* const ptr) const { return std::to_string(*static_cast<const counter_t*>(ptr)); } 
} counter_serializer;

struct : serializer {
  void        initialize(      void* const ptr) const { *static_cast<timer_t*>(ptr) = timer_t{}; }
  std::string serialize (const void* const ptr) const { return std::to_string(*static_cast<const timer_t*>(ptr)); } 
} timer_serializer;

struct : serializer {
  void        initialize(      void* const ptr) const { *static_cast<std::string*>(ptr) = std::string{}; }
  std::string serialize (const void* const ptr) const { return *static_cast<const std::string*>(ptr); } 
} string_serializer;

struct : serializer {
  void        initialize(      void* const ptr) const { *static_cast<bool*>(ptr) = bool{}; }
  std::string serialize (const void* const ptr) const { return *static_cast<const bool*>(ptr) ? "true" : "false"; } 
} boolean_serializer;

struct : serializer {
  void        initialize(      void* const ptr) const { *static_cast<poly_safety_result*>(ptr) = poly_safety_result{}; }
  std::string serialize (const void* const ptr) const { std::stringstream buff; buff << *static_cast<const poly_safety_result*>(ptr); return buff.str(); } 
} poly_safety_result_serializer;

struct : serializer {
  void        initialize(      void* const ptr) const { *static_cast<nlfpoly_safety_result*>(ptr) = nlfpoly_safety_result{}; }
  std::string serialize (const void* const ptr) const { std::stringstream buff; buff << *static_cast<const nlfpoly_safety_result*>(ptr); return buff.str(); } 
} nlfpoly_safety_result_serializer;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

using header_t      = std::vector<std::string>;
using record_t      = std::vector<std::string>;
using table_t       = std::vector<record_t>;
using pointers_t    = std::vector<void*>;
using serializers_t = std::vector<serializer*>;

extern serializers_t serializers  ;
extern pointers_t    pointers     ;
extern table_t       table        ;
extern header_t      header       ;
extern std::string   curr_rec_name;

inline void new_record(const std::string& name) {
  const auto col_count = header.size();
  for(typename header_t::size_type i = 0; i < col_count; i++)
    serializers[i]->initialize(pointers[i]);
  curr_rec_name = name; }

inline auto curr_record() {
  const auto col_count = header.size();
  record_t res;
  for(typename header_t::size_type i = 0; i < col_count; i++)
    res.push_back(serializers[i]->serialize(pointers[i]));
  return res; }

inline void add_record() { table.push_back(curr_record()); }

template<typename Out>
Out& print_table(Out& out, const table_t& table) {
  const auto col_count = header.size();
  std::vector<typename std::string::size_type> widths;

  constexpr unsigned header_row_count = 3;

  for(const auto& name : header)
    widths.push_back((name.length()+2)/header_row_count);
  for(const auto& row : table) 
    for(typename header_t::size_type i = 0; i < col_count; i++)
      widths[i] = std::max(widths[i], row[i].length()); 

  const auto third = [](const auto& str, const std::string::size_type width, const std::uint8_t n) {
    return str.substr(std::min(str.length(), n*width), width); };

  const std::string first_col_name = "row";
  const auto first_col_width = std::max(first_col_name.length(), std::to_string(table.size()+1).length());

  const auto print_header = [&](const std::uint8_t n) {
    out << std::right << std::setw(first_col_width) << third(first_col_name, first_col_width, n);
    for(typename header_t::size_type i = 0; i < col_count; i++) 
      out << " | " << std::left << std::setw(widths[i]) << third(header[i], widths[i], n);  
    out << std::endl;
  };

  for(unsigned i = 0; i < header_row_count; i++)
    print_header(i);

  for(typename table_t::size_type i = 0; i < table.size(); i++) {
    out << std::right << std::setw(first_col_width) << i + 1;
    const auto& row = table[i];
    for(typename header_t::size_type i = 0; i < col_count; i++) 
      out << " | " << std::left << std::setw(widths[i]) << row[i];
    out << std::endl; }
  return out; }

template<typename Out>
Out& print_curr_record(Out& out) {
  const auto col_count = header.size();
  std::string::size_type width = 0;
  for(const auto& name : header)
    if(width < name.length()) 
      width = name.length();
  for(typename header_t::size_type i = 0; i < col_count; i++)
    out << std::left << std::setw(width) << header[i] << ": " << serializers[i]->serialize(pointers[i]) << "\n";
  return out; }

template<typename Out>
Out& print_table(Out& out) { return print_table(out, table); }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

struct Null_Record { };
struct RAII_Record {
   RAII_Record(const std::string name) { new_record(name); }
   RAII_Record() : RAII_Record("<no-name>") { }

   RAII_Record(RAII_Record& ) = delete;
   RAII_Record(RAII_Record&&) = delete;
   RAII_Record& operator=(const RAII_Record& ) = delete;
   RAII_Record& operator=(const RAII_Record&&) = delete;
  ~RAII_Record() { add_record(); }
};

using Record = RAII_Record;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** type of header_##var and pointer_##var is irrelevant. I just want to make automatic initialization possible */
#define add_metric(var, name, type, serializer)                               \
  extern type var;                                                            \
  extern typename serializers_t::iterator serializer_##var;                   \
  extern typename pointers_t   ::iterator pointer_##var   ;                   \
  extern typename header_t     ::iterator header_ ##var   ;

#define add_counter_metric(var, name)                                         \
  add_metric(counter_##var, name, counter_t, counter_serializer)              \
  inline void inc_##var()                    { counter_##var++; }             \
  inline void dec_##var()                    { counter_##var--; }             \
  inline void set_##var(const counter_t val) { counter_##var  = val; }        \
  inline void add_##var(const counter_t val) { counter_##var += val; }

#define add_timer_metric(var, name)                                           \
  add_metric(timer_##var, name, timer_t, timer_serializer)                    \
  inline auto start_##var() { return Timer(timer_##var); }

#define add_boolean_metric(var, name)                                         \
  add_metric(bool_##var, name, bool, boolean_serializer)                      \
  inline void set_##var(const bool val) { bool_##var  = val; }

#define add_typed_metric(var, name, type)                                     \
  add_metric(typed_##var, name, type, type##_serializer)                      \
  inline void set_##var(const cmn::pbv_t<type> val) { typed_##var  = val; }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#define DEL_counter_metric(var, name)                             \
  inline void inc_##var()                { /* no op */ }          \
  inline void dec_##var()                { /* no op */ }          \
  inline void set_##var(const counter_t) { /* no op */ }          \
  inline void add_##var(const counter_t) { /* no op */ }

#define DEL_timer_metric(var, name)                               \
  inline auto start_##var() { return DummyTimer(); }

#define DEL_boolean_metric(var, name)                             \
  inline void set_##var(const bool val) { }

#define DEL_typed_metric(var, name, type)                         \
  inline void set_##var(const cmn::pbv_t<type> val) { }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** YES! this is the right place for inclusion, not the begining of this file */
#include "hare/performance_metrics.hpp"

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#undef add_metric
#undef add_counter_metric
#undef add_timer_metric
#undef add_boolean_metric
#undef add_typed_metric

#undef DEL_counter_metric
#undef DEL_timer_metric
#undef DEL_boolean_metric
#undef DEL_typed_metric

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

} // namespace ha::pc

#endif // HA__PERFORMANCE_COUNTER__HPP

