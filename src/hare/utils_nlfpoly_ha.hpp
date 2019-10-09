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

#ifndef HA__UTILS_NLFPOLY_HA__HPP
#define HA__UTILS_NLFPOLY_HA__HPP

#include "hare/nlfpoly_ha.hpp"
#include "hare/utils_poly_ha.hpp"
#include "hare/utils_ppl.hpp"
#include "hare/utils_term.hpp"

#include <ppl.hh>

#include <algorithm>
#include <limits>
#include <type_traits>
#include <utility>

namespace ha::nlfpoly_ha {

using namespace ppl_utils;
namespace ppl = Parma_Polyhedra_Library;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace internal {

using namespace poly_ha::internal;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

const auto loc_checker_closed      = [](const auto& loc) { return is_closed_poly(loc.inv()); };
const auto loc_checker_open        = [](const auto& loc) { return is_open_poly  (loc.inv()); };

const auto is_closed_ha            = [](const auto& ha) { return check_ha_object(ha, loc_checker_closed, edge_checker_closed); };
const auto is_open_ha              = [](const auto& ha) { return check_ha_object(ha, loc_checker_open  , edge_checker_open  ); };

const auto is_closed_safety = [](const auto& safety) { return check_safety_object(safety, is_closed_ha, poly_map_checker_closed, poly_map_checker_closed); };
const auto is_open_safety   = [](const auto& safety) { return check_safety_object(safety, is_open_ha  , poly_map_checker_open  , poly_map_checker_open  ); };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

} // namespace internal

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename P> bool is_normal(const nlfpoly_ha    <P>& ha    ) { return poly_ha::internal::is_normal_ha    (  ha  ); }
template<typename P> bool is_normal(const nlfpoly_safety<P>& safety) { return poly_ha::internal::is_normal_safety(safety); }

template<typename P> void normalize           (nlfpoly_ha    <P>& ha    ) { poly_ha::internal::normalize_ha        (  ha  ); }
template<typename P> void normalize           (nlfpoly_safety<P>& safety) { poly_ha::internal::normalize_safety    (safety); }
template<typename P> void normalize_end_points(nlfpoly_safety<P>& safety) { poly_ha::internal::normalize_end_points(safety); }

template<typename P> bool is_open  (const nlfpoly_ha    <P>& ha    ) { return internal::is_open_ha      (  ha  ); }
template<typename P> bool is_open  (const nlfpoly_safety<P>& safety) { return internal::is_open_safety  (safety); }

template<typename P> bool is_closed(const nlfpoly_ha    <P>& ha    ) { return internal::is_closed_ha    (  ha  ); }
template<typename P> bool is_closed(const nlfpoly_safety<P>& safety) { return internal::is_closed_safety(safety); }

//template<> bool is_closed<CPoly>(const nlfpoly_ha    <CPoly>& /*ha    */) { return true; }
//template<> bool is_closed<CPoly>(const nlfpoly_safety<CPoly>& /*safety*/) { return true; }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename P>
nlfpoly_ha<CPoly> closed_ha(const nlfpoly_ha<P>& ha) {
  return poly_ha::internal::ha_changer<nlfpoly_ha<CPoly>>::change( 
           ha, 
           [](const auto& q) { return nlfpoly_loc <CPoly>(q.id (), q.name(), closed_poly(q.inv()), q.flow(), q.is_trans()); },
           [](const auto& e) { return nlfpoly_edge<CPoly>(e.src(), e.dst (), e.lbl(), e.knd(), closed_poly(e.rel())); } ); }

template<typename P>
nlfpoly_ha<NNCPoly> open_ha(const nlfpoly_ha<P>& ha) {
  return poly_ha::internal::ha_changer<nlfpoly_ha<NNCPoly>>::change( 
           ha, 
           [](const auto& q) { return nlfpoly_loc <NNCPoly>(q.id (), q.name(), open_poly(q.inv()), q.flow(), q.is_trans()); },
           [](const auto& e) { return nlfpoly_edge<NNCPoly>(e.src(), e.dst (), e.lbl(), e.knd(), open_poly(e.rel())); } ); }

template<typename P>
nlfpoly_safety<CPoly> closed_safety(const nlfpoly_safety<P>& safety) {
  return poly_ha::internal::safety_changer<nlfpoly_safety<CPoly>>::change( 
           safety, 
           [](const auto& ha ) { return closed_ha(ha ); },
           [](const auto& /*new_ha_seq*/, const auto& par_id, const auto& poly) { return std::make_tuple(par_id, closed_poly(poly)); } ,
           [](const auto& /*new_ha_seq*/, const auto& par_id, const auto& poly) { return std::make_tuple(par_id, closed_poly(poly)); } ); }


template<typename P>
nlfpoly_safety<NNCPoly> open_safety(const nlfpoly_safety<P>& safety) {
  return poly_ha::internal::safety_changer<nlfpoly_safety<NNCPoly>>::change( 
           safety, 
           [](const auto& ha ) { return open_ha(ha ); },
           [](const auto& /*new_ha_seq*/, const auto& par_id, const auto& poly) { return std::make_tuple(par_id, open_poly(poly)); } , 
           [](const auto& /*new_ha_seq*/, const auto& par_id, const auto& poly) { return std::make_tuple(par_id, open_poly(poly)); } ); }

//template<> nlfpoly_ha    <CPoly> closed_ha      (const nlfpoly_ha    <CPoly>& ha    ) { return ha    ; }
//template<> nlfpoly_safety<CPoly> closed_safety  (const nlfpoly_safety<CPoly>& safety) { return safety; }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename P>
void inverse_time(nlfpoly_ha<P>& ha) {
  requires(std::numeric_limits<ppl::dimension_type>::max()/2 >= ha.vars().size());
  const auto map = shift_dimensions(ha.vars().size() << 1);
  for(auto& loc  : ha.locs ()) 
    for(auto& pair : loc.flow()) 
      std::get<1>(pair) = negate_variables(std::get<1>(pair));
  for(auto& edge : ha.edges()) {
    edge.rel().map_space_dimensions(map);
    std::swap(edge.src(), edge.dst()); }
  ha.refill_edge_indices(); }

template<typename P>
void inverse_time(nlfpoly_safety<P>& safety) {
  for(auto& ha : safety.ha_seq()) inverse_time(ha);
  std::swap(safety.inits(), safety.unsafes()); }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename P>
bool has_only_linear_flows(const nlfpoly_ha<P>& ha) {
  for(const auto& q : ha.locs())
    for(const auto& xf : q.flow())
      if(!is_linear(std::get<1>(xf))) 
        return false; 
  return true; }

template<typename P>
bool has_only_linear_flows(const nlfpoly_safety<P>& safety) {
  for(const auto& ha : safety.ha_seq())
    if(!has_only_linear_flows(ha))
      return false;
  return true; }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

struct self_loop_map {
  self_loop_map(const ppl::dimension_type d1, const ppl::dimension_type d2) : d1(d1), d2(d2) { }
  bool has_empty_codomain() const { return false; }
  auto max_in_codomain   () const { return d2 + 1; }
  bool maps(const ppl::dimension_type i, ppl::dimension_type& j) const {
    if(i == d2)           { j = d1;    return true; }
    if(d1 <= i && i < d2) { j = i + 1; return true; }
    j = i;
    return true; }
  const ppl::dimension_type d1, d2; };

template<typename P, typename Map, typename Lbl>
void force_discrete_edge_by_adding_self_loops_and_restricting_invariants(nlfpoly_ha<P>& ha, const term_ptr& v, const P& inv, const P& rel, 
                                                                         const P& loop_rel, const Map& map, cmn::pbv_t<Lbl> label) {
  const auto one = val(1);
  ha.vars().insert(v);
  ha.n2v().insert(std::make_pair(v->variable().name(), v));
  ha.i2v().insert(std::make_pair(v->variable().id  (), v));
  auto& edges = ha.edges();
  for(auto& edge : edges) {
    auto& edge_rel = edge.rel();
    edge_rel.add_space_dimensions_and_embed(2);
    edge_rel.map_space_dimensions(map);
    edge_rel.intersection_assign(rel); } 
  for(auto& loc : ha.locs()) {
    loc.flow().insert(std::make_pair(v, one));
    auto& loc_inv = loc.inv();
    loc_inv.add_space_dimensions_and_embed(1);
    loc_inv.intersection_assign(inv); 
    edges.emplace_back(loc.id(), loc.id(), label, label_kind::SYNC, loop_rel);  }
  ha.refill_edge_indices();  }

/** @tparam P        polyhedra type
  * @tparam Lbl      label name type
  * @param  ha       input hybrid automaton
  * @param  duration bound on duration of continuous time transitions
  * @param  lbl      label of the self loops to be added. In order to have a better performance, label should not be hidden in parallel composition
  * @param  var_name name of the new variable to be used for restricting duration of time transitions
  * @param  equality whether or not equality should be used as the guard of new edges. If \c z is the new variable and \c d is the duration, <tt>(z <= d)</tt>
  *                  should be used as the guard of new self-loops or <tt>(z == d)</tt>. 
  * @note   there should be no variable in the input \c hybrid automaton with the name \c var_name
  * @note   input duration must be positive 
  * @note   no edge in the automata of the input \c safety should have the input \c lbl */
template<typename P, typename Lbl>
void force_discrete_edge_by_adding_self_loops_and_restricting_invariants( nlfpoly_ha<P>& ha, 
                                                                          cmn::pbv_t<constant> duration, 
                                                                          cmn::pbv_t<Lbl> lbl,
                                                                          const term_ptr& var,
                                                                          const bool equality = true) {
  auto& vars = ha.vars();
  const auto& var_name = var->variable().name();
  requires_msg(duration > 0, "invalid duration: " << duration)
  requires_msg(ha.n2v().find(var_name) == ha.n2v().end(), "input automaton already has the input variable: " << var)
  requires_msg(std::none_of(ha.edges().begin(), ha.edges().end(), [&lbl](cmn::pbv_t<Lbl> that){ return lbl == that; }), 
               "input label (" << lbl << ") is not fresh")
  const auto dim = vars.size(); 
  const auto z   = ppl::Variable(dim);
  ppl::Constraint_System css_inv;
  ppl::Constraint_System css_rel;
  css_inv.set_space_dimension( dim+1);
  css_rel.set_space_dimension((dim+1) << 1);
  css_inv.insert(duration.get_den()*z <= duration.get_num());
  if(equality) css_rel.insert(duration.get_den()*z == duration.get_num());
  else         css_rel.insert(duration.get_den()*z <= duration.get_num());
  const P rel(css_rel);
  for(const auto& v : vars) {
    const auto var_id = v->variable().id();
    css_rel.insert(ppl::Variable(var_id) == ppl::Variable(var_id + dim)); }
  const P inv     (std::move(css_inv));
  const P loop_rel(std::move(css_rel));
  const self_loop_map map(dim, dim<<1);
  force_discrete_edge_by_adding_self_loops_and_restricting_invariants<P, self_loop_map, Lbl>(ha, var, inv, rel, loop_rel, map, lbl);  }

/** @tparam P        polyhedra type
  * @tparam Lbl      label name type
  * @param  safety   input safety
  * @param  duration bound on duration of continuous time transitions
  * @param  lbl      label of the self loops to be added. In order to have a better performance, label should not be hidden in parallel composition
  * @param  var_name name of the new variable to be used for restricting duration of time transitions
  * @param  equality whether or not equality should be used as the guard of new edges. If \c z is the new variable and \c d is the duration, <tt>(z <= d)</tt>
  *         should be used as the guard of new self-loops or <tt>(z == d)</tt>. 
  * @note   there should be no variable in the input \c safety with the name \c \var_name
  * @note   input duration must be positive 
  * @note   no edge in the automata of the input \c safety should have the input \c lbl */
template<typename P, typename Lbl>
void force_discrete_edge_by_adding_self_loops_and_restricting_invariants( nlfpoly_safety<P>& safety, 
                                                                          cmn::pbv_t<constant> duration, 
                                                                          cmn::pbv_t<Lbl> lbl,
                                                                          const term_ptr& var,
                                                                          const bool equality = true) {
  auto& vars = safety.vars();
  const auto& var_name = var->variable().name();
  requires_msg(duration > 0, "invalid duration: " << duration)
  requires_msg(safety.n2v().find(var_name) == safety.n2v().end(), "input safety already has the input variable: " << var)
  const auto dim = vars.size(); 
  const auto z   = ppl::Variable(dim);
  safety.n2v().insert(std::make_pair(var_name, var));
  safety.i2v().insert(std::make_pair(dim , var));
  ppl::Constraint_System css_inv;
  ppl::Constraint_System css_rel;
  css_inv.set_space_dimension( dim+1);
  css_rel.set_space_dimension((dim+1) << 1);
  css_inv.insert(z >= 0);                                                         //< (0<=z) && (z<=d) will be added to all invariants
  css_inv.insert(duration.get_den()*z <= duration.get_num());                     
  css_rel.insert(ppl::Variable(z.id() + dim + 1) == 0);                           //< z will be reset to zero on all edges
  const P rel(css_rel);
  if(equality) css_rel.insert(duration.get_den()*z == duration.get_num());        //< (z==d) or (z<=d) will be added to the guard of new edges
  else         css_rel.insert(duration.get_den()*z <= duration.get_num());
  for(const auto& v : vars) {
    const auto var_id = v->variable().id();
    css_rel.insert(ppl::Variable(var_id) == ppl::Variable(var_id + dim + 1)); }
  vars.insert(var); // must be after the previous loop
  const P inv     (std::move(css_inv));
  const P loop_rel(std::move(css_rel));
  const self_loop_map map(dim, dim<<1);
  for(auto& tuple : safety.inits()) 
    std::get<1>(tuple).add_space_dimensions_and_project(1);                       //< initially z is zero
  for(auto& tuple : safety.unsafes()) 
    std::get<1>(tuple).add_space_dimensions_and_embed(1);                         //< final value of z is not important
  for(auto& ha : safety.ha_seq()) 
    force_discrete_edge_by_adding_self_loops_and_restricting_invariants<P, self_loop_map, Lbl>(ha, var, inv, rel, loop_rel, map, lbl); }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

} // namespace ha::nlfpoly_ha

#endif // HA__UTILS_NLFPOLY_HA__HPP
