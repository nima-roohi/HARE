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

#ifndef HA__UTILS_CEGAR__HPP
#define HA__UTILS_CEGAR__HPP

#include "cmn/dbc.hpp"
#include "cmn/misc/hash.hpp"

#include "hare/poly_ha.hpp"
#include "hare/nlfpoly_ha.hpp"

#include <boost/optional.hpp>

namespace ha::cegar {

using loc_id_t     = std::uint16_t;
using par_loc_id_t = std::vector<loc_id_t>;
using edge_lbl_t   = std::string;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename P> using abs_safety_t        = poly_ha::poly_safety<P>;
template<typename P> using abs_states_seq_t    = std::vector<P>;
template<typename P> using abs_ha_t            = typename abs_safety_t <P>::ha_t        ;
template<typename P> using abs_ha_seq_t        = typename abs_safety_t <P>::ha_seq_t    ;
template<typename P> using abs_ha_index_t      = typename abs_safety_t <P>::ha_index_t  ;
template<typename P> using abs_loc_t           = typename abs_ha_t     <P>::loc_t       ;
template<typename P> using abs_locs_t          = typename abs_ha_t     <P>::locs_t      ;
template<typename P> using abs_edge_t          = typename abs_ha_t     <P>::edge_t      ;
template<typename P> using abs_edges_t         = typename abs_ha_t     <P>::edges_t     ;
template<typename P> using abs_inits_t         = typename abs_safety_t <P>::inits_t     ;
template<typename P> using abs_unsafes_t       = typename abs_safety_t <P>::unsafes_t   ;
template<typename P> using abs_init_index_t    = typename abs_inits_t  <P>::size_type   ;
template<typename P> using abs_unsafe_index_t  = typename abs_unsafes_t<P>::size_type   ;
template<typename P> using abs_edge_index_t    = typename abs_ha_t     <P>::edge_index_t;
template<typename P> using abs_par_edge_idx_t  = typename abs_safety_t <P>::par_edge_t  ;
template<typename P> using abs_par_loc_hash_t  = cmn::misc::sequence_hash<loc_id_t, par_loc_id_t>;
template<typename P> using abs_par_edge_hash_t = cmn::misc::sequence_hash<
                                                   typename abs_par_edge_idx_t<P>::value_type, 
                                                   abs_par_edge_idx_t<P>, 
                                                   cmn::misc::tuple_hash<
                                                     typename abs_par_edge_idx_t<P>::value_type>>;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename P> using cnc_safety_t        = nlfpoly_ha::nlfpoly_safety<P>;
template<typename P> using cnc_ha_t            = typename cnc_safety_t <P>::ha_t        ;
template<typename P> using cnc_ha_seq_t        = typename cnc_safety_t <P>::ha_seq_t    ;
template<typename P> using cnc_ha_index_t      = typename cnc_safety_t <P>::ha_index_t  ;
template<typename P> using cnc_loc_t           = typename cnc_ha_t     <P>::loc_t       ;
template<typename P> using cnc_locs_t          = typename cnc_ha_t     <P>::locs_t      ;
template<typename P> using cnc_edge_t          = typename cnc_ha_t     <P>::edge_t      ;
template<typename P> using cnc_edges_t         = typename cnc_ha_t     <P>::edges_t     ;
template<typename P> using cnc_inits_t         = typename cnc_safety_t <P>::inits_t     ;
template<typename P> using cnc_unsafes_t       = typename cnc_safety_t <P>::unsafes_t   ;
template<typename P> using cnc_init_index_t    = typename cnc_inits_t  <P>::size_type   ;
template<typename P> using cnc_unsafe_index_t  = typename cnc_unsafes_t<P>::size_type   ;
template<typename P> using cnc_loc_index_t     = typename cnc_locs_t   <P>::size_type   ;
template<typename P> using cnc_edge_index_t    = typename cnc_ha_t     <P>::edge_index_t;
template<typename P> using cnc_par_edge_idx_t  = typename cnc_safety_t <P>::par_edge_t  ;

template<typename P> using locs_backtrace_t  = std::vector<std::vector<cnc_loc_index_t <P>>>;
template<typename P> using edges_backtrace_t = std::vector<std::vector<cnc_edge_index_t<P>>>;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename P>
auto concretize_par_loc_id(const par_loc_id_t& abs_par_id, const locs_backtrace_t<P>& locs_backtrace, const cnc_ha_seq_t<P>& cnc_ha_seq) {
  const auto size = abs_par_id.size();
  par_loc_id_t res;
  res.reserve(size);
  asserts_msg(size == locs_backtrace.size(), "size=" << size << ", |locs_backtrace|=" << locs_backtrace.size())
  for(abs_ha_index_t<P> i = 0; i < size; i++) {
    const auto abs_loc_id = abs_par_id[i];
    asserts_msg(abs_loc_id < locs_backtrace[i].size(), "abs_loc_id=" << abs_loc_id << ", |locs_backtrace[i]|=" << locs_backtrace[i].size())
    const auto cnc_loc_id = locs_backtrace[i][abs_loc_id];
    asserts_msg(cnc_loc_id < cnc_ha_seq[i].locs().size(), "cnc_loc_id=" << cnc_loc_id << ", |locs|=" << cnc_ha_seq[i].locs().size())
    res.push_back(cnc_loc_id); }
  return std::move(res); }

template<typename P>
static boost::optional<cnc_par_edge_idx_t<P>> concretize_par_edge_idx(const abs_par_edge_idx_t<P>& par_idx, 
                                                                      const edges_backtrace_t <P>& edges_backtrace, 
                                                                      const cnc_ha_seq_t      <P>& cnc_ha_seq) {
  cnc_par_edge_idx_t<P> res;
  res.reserve(par_idx.size());
  for(const auto& pair : par_idx) {
    const auto ha_index       = std::get<0>(pair);
    const auto abs_edge_index = std::get<1>(pair);
    asserts_msg(abs_edge_index < edges_backtrace[ha_index].size(), "ha_index=" << ha_index << ", abs_edge_index=" << abs_edge_index << 
                                                                   ", |edges_backtrace[ha_index]|=" << edges_backtrace[ha_index].size())
    const auto cnc_edge_index = edges_backtrace[ha_index][abs_edge_index];
    if(cnc_edge_index == std::numeric_limits<cnc_edge_index_t<P>>::max()) return boost::none;
    asserts_msg(cnc_edge_index < cnc_ha_seq[ha_index].edges().size(), "ha_index=" << ha_index << ", cnc_edge_index=" << cnc_edge_index << 
                                                                      ", |edges|=" << cnc_ha_seq[ha_index].edges().size())
    res.push_back(std::make_tuple(ha_index, cnc_edge_index)); }
  return std::move(res); }

}

#endif // HA__UTILS_CEGAR__HPP
