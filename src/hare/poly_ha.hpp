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

#ifndef HA__POLY_HA__HPP
#define HA__POLY_HA__HPP

#include "hare/hybrid_automaton.hpp"
#include "hare/utils_ppl.hpp"

#include "cmn/dbc.hpp"

#include <cstdint>
#include <ostream>      // endl
#include <string>
#include <tuple>
#include <vector>

namespace hare::poly_ha {

using namespace ppl_utils;

using loc_id     = std::uint16_t;
using par_loc_id = std::vector<loc_id>;
using edge_lbl   = std::string;

template<typename P>
using poly_map = std::vector<std::tuple<par_loc_id, P>>;

struct universal_poly { template<typename P> bool operator()(const P& poly)          const { return poly.is_universe(); } };
struct empty_poly_map { template<typename P> bool operator()(const poly_map<P>& map) const { return map.empty       (); } };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

struct poly_printer {
  template<typename Out, typename P>
  Out& operator()(Out& out, const P& poly, std::uint64_t indent, const id2var& i2v, const name2var& /*n2v*/) const {
    std::stringstream sep;
    cmn::misc::utils::indent(sep << " && \"  \\\n", indent + 1) << "\"";
    print_polyhedron<Out, P>(out << "\"", poly, i2v, "==", "", sep.str().c_str()) << "\""; 
    return out; } };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

struct relation_printer {
  template<typename Out, typename P>
  Out& operator()(Out& out, const P& rel, const std::uint64_t indent, const id2var& i2v, const name2var& /*n2v*/) const {
    std::stringstream sep;
    cmn::misc::utils::indent(sep << " && \"  \\\n", indent + 4) << "\"";
    cmn::misc::utils::indent(out << std::endl, indent);
    print_polyhedron<Out, P>(out << "rel \"", rel, i2v, "==", "", sep.str().c_str()) << "\"";
    return out; } };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename Out>
inline std::uint64_t print_par_loc_id(Out& out, cmn::pbv_t<par_loc_id> par_id) {
  bool first = true;
  out << "\"[";
  std::uint64_t res = 4;
  for(cmn::pbv_t<loc_id> id : par_id) {
    if(first) first = false;
    else { 
      out   << ","; 
      res += 1; } 
    out << id;
    res += std::to_string(id).length(); }
  out << "]\"";
  return res; }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

struct poly_map_printer {
  template<typename Out, typename P>
  Out& operator()(Out& out, const poly_map<P>& map, std::uint64_t indent, const id2var& i2v, const name2var& /*n2v*/) const {
    namespace utils = cmn::misc::utils;
    for(const auto& m : map) {
      const auto& poly = std::get<1>(m);
      const auto size = print_par_loc_id(cmn::misc::utils::indent(out, indent), std::get<0>(m));
      std::stringstream sep;
      cmn::misc::utils::indent(sep << " && \"  \\\n", indent + size + 1) << "\"";
      out << " ";
      if(poly.is_universe()) out << "true" << std::endl;
      else {
        print_polyhedron<Out, P>(out << "\"", poly, i2v, "==", "", sep.str().c_str()) << "\"" << std::endl; } }
    return out;  }
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename P> using poly_loc    = loc <loc_id, P, P, poly_printer, poly_printer, universal_poly, universal_poly>;
template<typename P> using poly_edge   = edge<loc_id, edge_lbl, P, relation_printer, universal_poly>;
template<typename P> using poly_ha     = hybrid_automaton<poly_loc<P>, poly_edge<P>>;
template<typename P> using poly_safety = safety<poly_ha<P>, poly_map<P>, poly_map<P>, poly_map_printer, poly_map_printer, empty_poly_map, empty_poly_map>;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

} // namespace hare::poly_ha

#endif // HA__POLY_HA__HPP
