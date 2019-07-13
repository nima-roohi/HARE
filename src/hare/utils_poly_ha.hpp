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

#ifndef HA__UTILS_POLY_HA__HPP
#define HA__UTILS_POLY_HA__HPP

#include "ha/poly_ha.hpp"
#include "ha/utils_ppl.hpp"

#include <ppl.hh>

#include <algorithm>
#include <limits>
#include <type_traits>
#include <utility>

namespace ha::poly_ha {

using namespace ppl_utils;
namespace ppl = Parma_Polyhedra_Library;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace internal {

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename HA, typename LocChecker, typename EdgeChecker>
bool check_ha_object(const HA& ha, const LocChecker& loc_checker, const EdgeChecker& edge_checker) {
  const auto& locs  = ha.locs ();
  const auto& edges = ha.edges();
  return std::all_of(locs .begin(), locs .end(), [&loc_checker        ](const auto& loc ) { return loc_checker (loc       ); }) &&
         std::all_of(edges.begin(), edges.end(), [&edge_checker, &locs](const auto& edge) { return edge_checker(locs, edge); }) ; }

template<typename Safety, typename HAChecker, typename InitsChecker, typename UnsafeChecker>
bool check_safety_object(const Safety& safety, const HAChecker& ha_checker, const InitsChecker& init_checker, const UnsafeChecker& unsafe_checker) {
  const auto& ha_seq  = safety.ha_seq ();
  const auto& inits   = safety.inits  ();
  const auto& unsafes = safety.unsafes();
  return std::all_of(ha_seq.begin(), ha_seq.end(), ha_checker)                                         &&
         std::all_of(inits  .begin(), inits  .end(), [&ha_seq, &init_checker  ] (const auto& pair) { 
             return init_checker  (ha_seq, std::get<0>(pair), std::get<1>(pair)); })                   &&
         std::all_of(unsafes.begin(), unsafes.end(), [&ha_seq, &unsafe_checker] (const auto& pair) { 
             return unsafe_checker(ha_seq, std::get<0>(pair), std::get<1>(pair)); }); }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename HA, typename LocChanger, typename EdgeChanger>
void change_ha_object_inplace(HA& ha, const LocChanger& loc_changer, const EdgeChanger& edge_changer) {
  auto& locs  = ha.locs ();
  auto& edges = ha.edges();
  for(auto& loc  : locs ) loc_changer (loc);
  for(auto& edge : edges) edge_changer(locs, edge); }

template<typename Safety, typename HAChanger, typename InitsChanger, typename UnsafeChanger>
void change_safety_object_inplace(Safety& safety, const HAChanger& ha_changer, const InitsChanger& init_changer, const UnsafeChanger& unsafe_changer) {
  auto& ha_seq  = safety.ha_seq ();
  for(auto& ha : ha_seq) ha_changer(ha);
  for(auto& pair : safety.inits  ()) init_changer  (ha_seq, std::get<0>(pair), std::get<1>(pair));
  for(auto& pair : safety.unsafes()) unsafe_changer(ha_seq, std::get<0>(pair), std::get<1>(pair)); }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

const auto loc_checker_true = [](const auto& /*loc*/) { return true; };

const auto edge_checker_rels_subsets_of_invs = [](const auto& locs, const auto& edge) {
  auto inv = locs[edge.src()].inv();
  inv.concatenate_assign(locs[edge.dst()].inv()); 
  return inv.contains(edge.rel()); };

const auto poly_map_checker_subset_of_inv = [](const auto& ha_seq, const auto& par_id, const auto& poly) {
  const auto size = par_id.size();
  for(std::remove_const_t<std::remove_reference_t<decltype(size)>> i = 0; i < size; i++)
      if(!ha_seq[i].loc(par_id[i]).inv().contains(poly))
        return false;
    return true; }; 

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

const auto is_normal_ha     = [](const auto& ha    ) { return check_ha_object    ( ha, loc_checker_true, edge_checker_rels_subsets_of_invs); };
const auto is_normal_safety = [](const auto& safety) { return check_safety_object( safety, 
                                                                                   is_normal_ha, 
                                                                                   poly_map_checker_subset_of_inv, 
                                                                                   poly_map_checker_subset_of_inv ); };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

const auto loc_changer_inline_id = [](const auto& /*loc*/) { /* no op */ };

const auto edge_changer_rels_subsets_of_inv = [](const auto& locs, auto& edge) {
  auto inv = locs[edge.src()].inv();
  inv.concatenate_assign(locs[edge.dst()].inv()); 
  edge.rel().intersection_assign(inv); };

const auto poly_map_changer_subsets_of_inv = [](const auto& ha_seq, const auto& par_id, auto& poly) {
  const auto size = par_id.size();
  for(std::remove_const_t<std::remove_reference_t<decltype(size)>> i = 0; i < size; i++)
    poly.intersection_assign(ha_seq[i].loc(par_id[i]).inv()); };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

const auto remove_empty_statesets = [](auto& tuples) {
  tuples.erase(std::remove_if(tuples.begin(), tuples.end(), [](const auto& tuple) { return std::get<1>(tuple).is_empty(); } ), tuples.end()); };

const auto normalize_ha = [](auto& ha) { 
  change_ha_object_inplace(ha, loc_changer_inline_id, edge_changer_rels_subsets_of_inv); 
  ensures(is_normal_ha(ha)) };

const auto normalize_safety = [](auto& safety) { 
  change_safety_object_inplace(safety, normalize_ha, poly_map_changer_subsets_of_inv, poly_map_changer_subsets_of_inv);
  remove_empty_statesets(safety.inits  ());
  remove_empty_statesets(safety.unsafes());
  ensures(is_normal_safety(safety)) };

const auto normalize_end_points = [](auto& safety) { 
  change_safety_object_inplace(safety, [](const auto&){}, poly_map_changer_subsets_of_inv, poly_map_changer_subsets_of_inv);
  remove_empty_statesets(safety.inits  ());
  remove_empty_statesets(safety.unsafes()); };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

const auto loc_checker_closed      = [](const auto& loc) { return is_closed_poly(loc.inv()) && is_closed_poly(loc.flow()); };
const auto loc_checker_open        = [](const auto& loc) { return is_open_poly  (loc.inv()) && is_open_poly  (loc.flow()); };

const auto edge_checker_closed     = [](const auto& /*locs*/, const auto& edge) { return is_closed_poly(edge.rel()); };
const auto edge_checker_open       = [](const auto& /*locs*/, const auto& edge) { return is_open_poly  (edge.rel()); };

const auto poly_map_checker_closed = [](const auto& /*ha_seq*/, const auto& /*par_id*/, const auto& poly) { return is_closed_poly(poly); };
const auto poly_map_checker_open   = [](const auto& /*ha_seq*/, const auto& /*par_id*/, const auto& poly) { return is_open_poly  (poly); };

const auto is_closed_ha            = [](const auto& ha) { return check_ha_object(ha, loc_checker_closed, edge_checker_closed); };
const auto is_open_ha              = [](const auto& ha) { return check_ha_object(ha, loc_checker_open  , edge_checker_open  ); };

const auto is_closed_safety = [](const auto& safety) { return check_safety_object(safety, is_closed_ha, poly_map_checker_closed, poly_map_checker_closed); };
const auto is_open_safety   = [](const auto& safety) { return check_safety_object(safety, is_open_ha  , poly_map_checker_open  , poly_map_checker_open  ); };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename HAout>
struct ha_changer {
  template<typename HAin, typename QChanger, typename EChanger>
  static auto change(const HAin& ha, const QChanger& loc_changer, const EChanger& edge_changer) {
    typename HAout::locs_t  locs;
    typename HAout::edges_t edges;
    locs .reserve(ha.locs ().size());
    edges.reserve(ha.edges().size());
    for(const auto& loc  : ha.locs ()) locs .push_back(loc_changer (loc ));
    for(const auto& edge : ha.edges()) edges.push_back(edge_changer(edge));
    return HAout(ha.name(), ha.vars(), std::move(locs), std::move (edges)); } };

template<typename SOut>
struct safety_changer {
  template<typename SIn, typename HAChanger, typename InitChanger, typename UnsafeChanger>
  static auto change(const SIn& safety, const HAChanger& ha_changer, const InitChanger& init_changer, const UnsafeChanger& unsafe_changer) {
    typename SOut::ha_seq_t  ha_seq;
    typename SOut::inits_t   inits;
    typename SOut::unsafes_t unsafes;
    ha_seq .reserve(safety.ha_seq ().size());
    inits  .reserve(safety.inits  ().size());
    unsafes.reserve(safety.unsafes().size());
    for(const auto& ha   : safety.ha_seq ()) ha_seq .push_back(ha_changer(ha));
    for(const auto& pair : safety.inits  ()) inits  .push_back(init_changer  (ha_seq, std::get<0>(pair), std::get<1>(pair)));
    for(const auto& pair : safety.unsafes()) unsafes.push_back(unsafe_changer(ha_seq, std::get<0>(pair), std::get<1>(pair)));
    return SOut(std::move(ha_seq), std::move(inits), std::move(unsafes)); } };

} // namespace internal

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename P> bool is_normal(const poly_ha    <P>& ha    ) { return internal::is_normal_ha    (  ha  ); }
template<typename P> bool is_normal(const poly_safety<P>& safety) { return internal::is_normal_safety(safety); }

template<typename P> void normalize           (poly_ha    <P>& ha    ) { internal::normalize_ha        (  ha  ); }
template<typename P> void normalize           (poly_safety<P>& safety) { internal::normalize_safety    (safety); }
template<typename P> void normalize_end_points(poly_safety<P>& safety) { internal::normalize_end_points(safety); }

template<typename P> bool is_open  (const poly_ha    <P>& ha    ) { return internal::is_open_ha      (  ha  ); }
template<typename P> bool is_open  (const poly_safety<P>& safety) { return internal::is_open_safety  (safety); }

template<typename P> bool is_closed(const poly_ha    <P>& ha    ) { return internal::is_closed_ha    (  ha  ); }
template<typename P> bool is_closed(const poly_safety<P>& safety) { return internal::is_closed_safety(safety); }

//template<> bool is_closed<CPoly>(const poly_ha    <CPoly>& /*ha    */) { return true; }
//template<> bool is_closed<CPoly>(const poly_safety<CPoly>& /*safety*/) { return true; }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename P>
poly_ha<CPoly> closed_ha(const poly_ha<P>& ha) {
  return internal::ha_changer<poly_ha<CPoly>>::change( 
           ha, 
           [](const auto& q) { return poly_loc <CPoly>(q.id (), q.name(), closed_poly(q.inv()), closed_poly(q.flow()), q.is_trans()); },
           [](const auto& e) { return poly_edge<CPoly>(e.src(), e.dst (), e.lbl(), e.knd(), closed_poly(e.rel())); } ); }

template<typename P>
poly_ha<NNCPoly> open_ha(const poly_ha<P>& ha) {
  return internal::ha_changer<poly_ha<NNCPoly>>::change( 
           ha, 
           [](const auto& q) { return poly_loc <NNCPoly>(q.id (), q.name(), open_poly(q.inv()), open_poly(q.flow()), q.is_trans()); },
           [](const auto& e) { return poly_edge<NNCPoly>(e.src(), e.dst (), e.lbl(), e.knd(), open_poly(e.rel())); } ); }

template<typename P>
poly_safety<CPoly> closed_safety(const poly_safety<P>& safety) {
  return internal::safety_changer<poly_safety<CPoly>>::change( 
           safety, 
           [](const auto& ha ) { return closed_ha(ha ); },
           [](const auto& /*new_ha_seq*/, const auto& par_id, const auto& poly) { return std::make_tuple(par_id, closed_poly(poly)); } ,
           [](const auto& /*new_ha_seq*/, const auto& par_id, const auto& poly) { return std::make_tuple(par_id, closed_poly(poly)); } ); }


template<typename P>
poly_safety<NNCPoly> open_safety(const poly_safety<P>& safety) {
  return internal::safety_changer<poly_safety<NNCPoly>>::change( 
           safety, 
           [](const auto& ha ) { return open_ha(ha ); },
           [](const auto& /*new_ha_seq*/, const auto& par_id, const auto& poly) { return std::make_tuple(par_id, open_poly(poly)); } ,
           [](const auto& /*new_ha_seq*/, const auto& par_id, const auto& poly) { return std::make_tuple(par_id, open_poly(poly)); } ); }

//template<> poly_ha    <CPoly> closed_ha      (const poly_ha    <CPoly>& ha    ) { return ha    ; }
//template<> poly_safety<CPoly> closed_safety  (const poly_safety<CPoly>& safety) { return safety; }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename P>
void inverse_time(poly_ha<P>& ha, const ppl::dimension_type dim) {
  requires(std::numeric_limits<ppl::dimension_type>::max()/2 >= dim);
  const auto map = shift_dimensions(ha.vars().size() << 1);
  for(auto& loc  : ha.locs ()) 
    loc.flow() = negate_variables(loc.flow());
  for(auto& edge : ha.edges()) {
    edge.rel().map_space_dimensions(map);
    std::swap(edge.src(), edge.dst()); }
  ha.refill_edge_indices(); }

template<typename P>
void inverse_time(poly_safety<P>& safety) {
  for(auto& ha : safety.ha_seq()) inverse_time(ha, safety.vars().size());
  std::swap(safety.inits(), safety.unsafes()); }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

} // namespace ha::poly_ha

#endif // HA__UTILS_POLY_HA__HPP
