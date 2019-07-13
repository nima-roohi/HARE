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

#ifndef HA__NLFPOLY_HA__HPP
#define HA__NLFPOLY_HA__HPP

#include "ha/hybrid_automaton.hpp"
#include "ha/poly_ha.hpp"
#include "ha/term.hpp"

#include <cstdint>
#include <ostream>          // endl
#include <unordered_map>

namespace ha::nlfpoly_ha {

using namespace ppl_utils;

using loc_id     = poly_ha::loc_id    ;
using par_loc_id = poly_ha::par_loc_id;
using edge_lbl   = poly_ha::edge_lbl  ;

template<typename P>
using poly_map = poly_ha::poly_map<P> ; 

using poly_map_printer = poly_ha::poly_map_printer; 
using empty_poly_map   = poly_ha::empty_poly_map  ; 
using poly_printer     = poly_ha::poly_printer    ; 
using universal_poly   = poly_ha::universal_poly  ; 

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

using flow_map = std::unordered_map<term_ptr /*variable*/, term_ptr /*its flow*/>;

struct trivial_flow_map { bool operator()(const flow_map& map) const { return map.empty(); } };

struct flow_printer {
  template<typename Out>
  Out& operator()(Out& out, const flow_map& flow, std::uint64_t indent, const id2var& /*i2v*/, const name2var& /*n2v*/) const {
    out << "{";
    for(const auto& pair : flow) {
      cmn::misc::utils::indent(out << std::endl, indent);
      out << pair.first << " \"" << pair.second << "\""; }
    out << "  }";
    return out; } };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename P> using nlfpoly_loc    = loc<loc_id, P, flow_map, poly_printer, flow_printer, universal_poly, trivial_flow_map>;
template<typename P> using nlfpoly_edge   = poly_ha::poly_edge<P>;
template<typename P> using nlfpoly_ha     = hybrid_automaton<nlfpoly_loc<P>, nlfpoly_edge<P>>;
template<typename P> using nlfpoly_safety = safety<nlfpoly_ha<P>, poly_map<P>, poly_map<P>, poly_map_printer, poly_map_printer, empty_poly_map, empty_poly_map>;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------


} // namespace ha::nlfpoly_ha

#endif // HA__NLFPOLY_HA__HPP
