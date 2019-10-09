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

#ifndef HA__POLY_HA_SAFETY_CHECKER__HPP
#define HA__POLY_HA_SAFETY_CHECKER__HPP

#include "hare/performance_counter.hpp"
#include "hare/poly_ha.hpp"
#include "hare/safety_result.hpp"
#include "hare/utils_poly_ha.hpp"
#include "hare/utils_ppl.hpp"
#include "hare/utils_props.hpp"
#include "hare/utils_z3.hpp"

#include "cmn/dbc.hpp"
#include "cmn/type_traits.hpp"
#include "cmn/misc/hash.hpp"

#include <gmpxx.h>
#include <ppl.hh>
#include <z3.h>
#include <z3++.h>

#include <boost/mpl/if.hpp>
#include <boost/property_tree/ptree.hpp>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <list>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <utility>        // swap

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace ha::poly_ha {

using namespace ppl_utils;
using namespace z3_utils ;

namespace pt = boost::property_tree;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename P>
struct safety_checker {

  using safety_t          = poly_safety<P>;
  using ha_t              = typename safety_t::ha_t      ;
  using ha_seq_t          = typename safety_t::ha_seq_t  ;
  using ha_index_t        = typename safety_t::ha_index_t;
  using inits_t           = typename safety_t::inits_t   ;
  using unsafes_t         = typename safety_t::unsafes_t ;
  using edge_index_t      = typename ha_t::edge_index_t  ;
  using lbl_t             = typename ha_t::edge_t::lbl_t ;
  using par_edge_t        = typename safety_t::par_edge_t;
  using par_edge_varmap_t = std::unordered_map<ppl::Variable, ppl::Variable, ppl_var_hash, ppl_var_equal>;
  using par_edge_info_t   = std::tuple<par_edge_t, P, par_edge_varmap_t>;
  using path_node_index_t = std::uint64_t;
  using init_index_t      = typename inits_t::size_type;
  using path_node_t       = std::tuple<path_node_index_t /* previous edge index */, par_edge_t, init_index_t /* initial index */>;
  using par_loc_hash_t    = cmn::misc::sequence_hash<loc_id, par_loc_id>;
  using par_edge_hash_t   = cmn::misc::sequence_hash<typename par_edge_t::value_type, par_edge_t, cmn::misc::tuple_hash<typename par_edge_t::value_type>>;
  using work_list_t       = std::unordered_map<par_loc_id, std::list<std::tuple<path_node_index_t, P>>, par_loc_hash_t>;
  using visited_solvers_t = std::unordered_map<par_loc_id, z3::solver                                 , par_loc_hash_t>;
  using wanted_solvers_t  = std::unordered_map<par_loc_id, boost::optional<z3::solver>                , par_loc_hash_t>;
  using states_seq_t      = std::vector<P>;
  
  struct counter_example_t {
    using path_t = std::list<par_edge_t>;
    par_loc_id init_par_loc   ;
    par_loc_id unsafe_par_loc ;
    P          init_state     ;
    P          unsafe_state   ;
    path_t     path           ; };
  
  template<typename SS, std::enable_if_t<std::is_same<safety_t, std::remove_const_t<std::remove_reference_t<SS>>>::value>* = nullptr>
  safety_checker(SS&& safety, const bool compute_states_seq = false) : safety_(std::forward<SS>(safety)), 
                                                                       ha_seq (safety_.ha_seq ()),
                                                                       inits  (safety_.inits  ()),
                                                                       unsafes(safety_.unsafes()),
                                                                       props  (safety_.props  ()), 
                                                                       i2v    (safety_.i2v    ()),
                                                                       n2v    (safety_.n2v    ()),
                                                                       ha_size(ha_seq.size    ()), 
                                                                       dim    (safety_.vars().size()),
                                                                       compute_states_seq_(compute_states_seq),
                                                                       inversed_time(should_inverse_time(props, inits, unsafes)),
                                                                       sep_id_resets(should_separate_identity_resets(props)),
                                                                       check_unsafe_after_disc(should_check_unsafe_after_disc_edge(props)),
                                                                       add_to_visiteds_on_check(should_add_to_visiteds_on_check(props)),
                                                                       poly_var_set(create_poly_var_set(dim)),
                                                                       max_iter(max_iter_param(props)),
                                                                       slv(z3::solver(ctx)) {
    log_debug << "starting initialization of poly-ha safety checker";

    pc::set_mc_poly_inversed_time          (inversed_time);
    pc::set_mc_poly_separated_id_resets    (sep_id_resets);
    pc::set_mc_poly_check_unsafe_after_disc(check_unsafe_after_disc);
    pc::set_poly_dim(i2v.size());

    initialize();
    initialization_timer.stop();

    {
      log_debug << "starting poly-ha reachable set computation";
      const auto timer = pc::start_mc_poly_reachablity();
      check_safety(); 
    }

    total_timer.stop();
    pc::set_mc_poly_reached_loc_count(visiteds.size());
    pc::set_mc_poly_safety_result(safety_result_);
    log_debug << "reached to a fixed-point or an unsafe state in poly-ha (safety_result? " << safety_result_ << ")";
  }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------
  
  poly_safety_result safety_result() const noexcept { return safety_result_; }

  const auto& counter_example() const noexcept {
    requires_msg(safety_result_ == poly_safety_result::UNSAFE, "the system is not unsafe!")
    return counter_example_; }

  const auto& states_seq() const noexcept {
    requires_msg(safety_result_ == poly_safety_result::UNSAFE, "the system is not unsafe!")
    requires_msg(compute_states_seq_, "construct-states-seq is false")
    return states_seq_;  }

  template<typename Out, typename V>
  static Out& print_par_loc_id(Out& out, const V& vec) {
    if(vec.size() == 1) out << std::to_string(vec[0]);
    else {
      out << "[";
      bool first = true;
      for(cmn::pbv_t<loc_id> id : vec) {
        if(first) first = false; else out << ",";
        out << std::to_string(id); }
      out << "]"; } 
    return out; }

  template<typename Out>
  Out& print_counter_example(Out& out) const {
    requires_msg(safety_result_ == poly_safety_result::UNSAFE, "the system is safe!")
    auto par_id = counter_example_.init_par_loc;
    print_par_loc_id(out, par_id);
    for(const auto& par_edge : counter_example_.path) {
      out << " --";
      if(ha_size > 1) out << "(";
      bool first = true;
      for(const auto& pair : par_edge) {
        const auto  ha_index   = std::get<0>(pair);
        const auto  edge_index = std::get<1>(pair);
        asserts_msg(ha_index < ha_size, "ha_index=" << ha_index << ", ha_size=" << ha_size)
        const auto& ha    = ha_seq[ha_index];
        const auto& edges = ha.edges();
        requires_msg(edge_index < edges.size(), "ha_index=" << ha_index << ", edge_index=" << edge_index << ", |edges|=" << edges.size())
        const auto& edge = edges[edge_index];
        par_id[ha_index] = edge.dst();  
        if(first) first = false; else out << ", ";
        if(ha_size > 1) out << ha.name() << ".";
        out << edge_index; }
      if(ha_size > 1) out << ")";
      out << "--> "; 
      print_par_loc_id(out, par_id); }
    return out; }

  template<typename Out>
  Out& print_annotated_counter_example(Out& out) const {
    requires_msg(safety_result_ == poly_safety_result::UNSAFE, "the system is not unsafe!")
    requires_msg(compute_states_seq_, "construct-states-seq is false")
    const auto size = states_seq_.size();
    for(typename states_seq_t::size_type i = 0; i < size; i++) {
      out << "Step " << i << " ";
      if( i    == 0) out << "(Initial State):"         ; else
      if((i&1) == 1) out << "(After Countinuous Post):"; else
                     out << "(After Discrete Post):"   ;
      out << std::endl;
      const auto& poly = states_seq_[i];
      ppl_utils::print_polyhedron(out, poly, i2v, "=", "", " && ") << std::endl;
    }
    return out; }

  template<typename Out>
  Out& print_reachable_states(Out& out) const {
    for(const auto& pair : visiteds) {
      const auto& par_id = std::get<0>(pair);
      const auto& solver = std::get<1>(pair);
      print_par_loc_id(out << "Location ", par_id) << ": ";
      const auto& assertions = solver.assertions();
      const auto size = assertions.size();
      for(unsigned i = 0; i < size; i++) {
				const auto expr = assertions[i];
        const auto t    = z3_utils::to_term(expr, n2v); 
        if(i>0) out << " " << symbol::AND << " ";
        out << "(" << t << ")";
      } 
      out << std::endl;
    }
		return out; }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------
  
  static auto create_poly_var_set(const ppl::dimension_type dim) {
    ppl::Variables_Set res;
    for(ppl::dimension_type v = 0; v < dim; v++)
      res.insert(ppl::Variable(v));
    return res; }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------
  
  static auto max_iter_param(const pt::ptree& props) {
    return ::ha::prop_value<val_uint_t>(props, KEY_MaxIter, DEF_MaxIter); }
  
  static auto should_separate_identity_resets(const pt::ptree& props) {
    return ::ha::prop_value<val_bool_t>(props, KEY_SepIDReset, DEF_SepIDReset); }

  static auto should_check_unsafe_after_disc_edge(const pt::ptree& props) {
    return ::ha::prop_value<val_bool_t>(props, KEY_CheckUnsafeAfterDiscEdge, DEF_CheckUnsafeAfterDiscEdge); }

  static auto should_add_to_visiteds_on_check(const pt::ptree& props) {
    return ::ha::prop_value<val_bool_t>(props, KEY_AddToVisitedsOnCheck, DEF_AddToVisitedsOnCheck); }

  static auto should_inverse_time(const pt::ptree& props, const inits_t inits, const unsafes_t unsafes) {
    const auto dir = ::ha::prop_value<val_str_t>(props, KEY_Direction, DEF_Direction);
    requires_msg(dir == VAL_Direction_Forward            || dir == VAL_Direction_Backward || 
                 dir == VAL_Direction_Smaller_or_Forward || dir == VAL_Direction_Smaller_or_Backward,    
                 "invalid direction " << dir << ", possible values are " << 
                 VAL_Direction_Forward            << ", "     << VAL_Direction_Backward << ", " << 
                 VAL_Direction_Smaller_or_Forward << ", and " << VAL_Direction_Smaller_or_Backward)
    return dir == VAL_Direction_Backward || 
          (dir == VAL_Direction_Smaller_or_Backward && inits.size() >= unsafes.size()) ||
          (dir == VAL_Direction_Smaller_or_Forward  && inits.size() >  unsafes.size()) ; }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  void initialize() {
    if(inversed_time)
      inverse_time(safety_);
    normalize_end_points(safety_);
    initialize_wanteds    ();
    initialize_work_list_1(); }
  
  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  const visited_solvers_t& visited_solvers() { return visiteds; }

  auto& visited_solver(const par_loc_id& id) {
    auto iter = visiteds.find(id);
    if(iter != visiteds.end()) return iter->second;
    return std::get<0>(visiteds.emplace(id, z3::solver(ctx)))->second; }
    //return std::get<0>(visiteds.emplace(id, (z3::tactic(ctx, "ctx-solver-simplify") & z3::tactic(ctx, "smt")).mk_solver() ))->second; }
    //return std::get<0>(visiteds.emplace(id, (z3::tactic(ctx, "simplify") & z3::tactic(ctx, "smt")).mk_solver() ))->second; }
    
  auto& wanted_solver (const par_loc_id& id) { 
    auto iter = wanteds.find(id);
    if(iter != wanteds.end()) return iter->second;
    return std::get<0>(wanteds.emplace(id, boost::optional<z3::solver>(z3::solver(ctx))))->second; }
  
  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  /** @return \c true iff \c num becomes 0 */
  template<typename T, typename U>
  static bool inc(const std::vector<T>& max, std::vector<U>& num) {
    const auto size = max.size();
    requires_msg(size == num.size(), "invalid size: |max|=" << max.size() << ", |num|=" << num.size())
    requires_msg(std::all_of(max.cbegin(), max.cend(), [](const auto d){return d != 0;}), "max cannot have a zero digit")
    typename std::vector<T>::size_type i = 0;
    while(i < size && num[i] == max[i] - 1) 
      num[i++] = 0;
    if(i < size)
      num[i]++; 
    return i >= size; }

  /** @return \c true iff \c fst is strictly smaller than \c snd (lexicographically)
    * @note   size of first and second must be equal  */
  template<typename T, typename U>
  static bool is_smaller(const std::vector<T>& fst, const std::vector<U>& snd) {
    const auto size = fst.size();
    requires_msg(size == snd.size(), "invalid size: |fst|=" << fst.size() << ", |snd|=" << snd.size())
    for(typename std::vector<T>::size_type i = 0; i < size; i++)
           if(fst[i] < snd[i]) return true ;
      else if(fst[i] > snd[i]) return false;
      return false;  }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  void initialize_wanteds() {
    pc::set_poly_unsafe_polies_count(unsafes.size());
    std::unordered_map<par_loc_id, z3::expr, par_loc_hash_t> map;
    for(const auto& unsafe : unsafes) {
      const auto& par_loc_id_ = std::get<0>(unsafe);
            auto  expr        = z3_utils::to_bool_expr(
                                  to_term(
                                    std::get<1>(unsafe), 
                                    i2v), 
                                  ctx);
      const auto iter = map.find(par_loc_id_);
      if(iter == map.end()) map.emplace(par_loc_id_, std::move(expr));
      else {
        auto& curr = iter->second;
        curr = curr || expr; } }
    for(auto& pair : map) {
      const auto& par_id = std::get<0>(pair);
            auto& expr   = std::get<1>(pair);
      slv.reset();    slv.add( expr);
      if(z3_utils::is_unsatisfiable(slv)) continue;
      pc::inc_poly_unsafe_loc_count();
      slv.reset();    slv.add(!expr);
      if(z3_utils::is_unsatisfiable(slv)) {
        pc::inc_poly_trivial_unsafe_count();
        wanted_solver(par_id) = boost::none;
      } else                                
        wanted_solver(par_id)->add(std::move(expr)); } }
  
  void initialize_work_list_1() {
    const auto inits_size = inits.size();
    pc::set_poly_init_polies_count(inits_size);
    for(init_index_t i = 0; i < inits_size; i++) {
      const auto& init   = inits[i];
      const auto& par_id = std::get<0>(init);
      const auto& state  = std::get<1>(init);
      path_nodes.emplace_back(0 /* 0 means no more */, par_edge_t{} /* empty par-edge means no more edge */, i);
      work_list_1[par_id].emplace_back(i + 1, state); // indices start from one. The 0-th index is a dummy one!
    }
  }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  void find_and_intersect_with_a_matching_unsafe_stateset(const par_loc_id& unsafe_loc_id, P& state_set) {
    for(const auto& pair : unsafes) {
      const auto& par_id = std::get<0>(pair);
      const auto& unsafe = std::get<1>(pair);
      if(par_id == unsafe_loc_id && !state_set.is_disjoint_from(unsafe)) {
        state_set.intersection_assign(unsafe);
        return; } }
    requires_msg(false, "the input state-set has no intersection with any unsafe state-set")  }

  void fill_counter_example_path(const path_node_index_t index, const par_loc_id& dst) {
    const auto& node       = path_nodes[index];
    const auto  prev_index = std::get<0>(node);
    const auto& par_edge   = std::get<1>(node);
    asserts_msg(prev_index > 0, "error in code: index:" << index)
    const auto  src = source_of(dst, par_edge);
    counter_example_.path.push_front(par_edge);
    if(std::get<0>(path_nodes[prev_index]) == 0) {
      counter_example_.init_par_loc = src;
      counter_example_.init_state   = std::get<1>(inits[prev_index-1]); // 1-based index 
    } else fill_counter_example_path(prev_index, src); }

  void fill_counter_example(const par_loc_id& last_loc_id, const P& last_state, const path_node_index_t index) {
    counter_example_.unsafe_par_loc = last_loc_id;
    counter_example_.unsafe_state   = last_state ;
    find_and_intersect_with_a_matching_unsafe_stateset(counter_example_.unsafe_par_loc, counter_example_.unsafe_state);
    const auto prev_index = std::get<0>(path_nodes[index]);
    if(prev_index == 0) {
      counter_example_.init_par_loc = last_loc_id;
      counter_example_.init_state   = std::get<1>(inits[index-1]); // 1-based index
    } else 
      fill_counter_example_path(index, last_loc_id); 
    if(compute_states_seq_)
      construct_states_seq();
    if(inversed_time) {
      std::swap   (counter_example_.init_par_loc, counter_example_.unsafe_par_loc);
      std::swap   (counter_example_.init_state  , counter_example_.unsafe_state  );
      std::reverse(counter_example_.path.begin(), counter_example_.path.end());  
      std::reverse(states_seq_          .begin(), states_seq_          .end());  }
  }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------
  
  const P& inv_of(const par_loc_id& ids) { 
    requires(ids.size() > 0)
    if(ha_size == 1) return ha_seq[0].loc(ids[0]).inv();
    auto iter = invs.find(ids);
    if(iter != invs.end()) return iter->second;
    P res(dim);
    for(ha_index_t i = 0; i < ha_size; i++) 
      res.intersection_assign(ha_seq[i].loc(ids[i]).inv());
    return std::get<0>(invs.insert(std::make_pair(ids, std::move(res))))->second; }
  
  const P& flow_of(const par_loc_id& ids) { 
    requires(ids.size() > 0)
    if(ha_size == 1) return ha_seq[0].loc(ids[0]).flow();
    auto iter = flows.find(ids);
    if(iter != flows.end()) return iter->second;
    P res(dim);
    for(ha_index_t i = 0; i < ha_size; i++) 
      res.intersection_assign(ha_seq[i].loc(ids[i]).flow());
    return std::get<0>(flows.insert(std::make_pair(ids, std::move(res))))->second; }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------
  
  const auto& edges_of(const par_loc_id& ids) {
    auto iter = edges.find(ids);
    if(iter != edges.end()) return iter->second;
    std::vector<par_edge_t> res;

    const auto syncs = [this, &ids](const auto& label) {
      std::vector<std::vector<edge_index_t>> res(ha_size);
      for(ha_index_t i = 0; i < ha_size; i++)
        for(const auto e : ha_seq[i].sync_edges(ids[i]))
          if(ha_seq[i].edge(e).lbl() == label) 
            res[i].push_back(e);
      return res; };

    const auto ins = [this, &ids](const auto& label, const ha_index_t broad_ha_index, const edge_index_t broad_edge_index) {
      std::vector<std::vector<edge_index_t>> res(ha_size);
      res[broad_ha_index].push_back(broad_edge_index);
      for(ha_index_t i = 0; i < ha_size; i++)
        if(i == broad_ha_index) continue;
        else for(const auto e : ha_seq[i].in_edges(ids[i]))
               if(ha_seq[i].edge(e).lbl() == label) 
                 res[i].push_back(e);
      return res; };

    const auto add_all_combinations = [this, &res](const auto& edges) {
      std::vector<edge_index_t> num(ha_size), max;
      max.reserve(ha_size);
      requires_msg(std::all_of(num.begin(), num.end(), [](const auto d){return d == 0;}), 
                   "error in code: initialization is not as expected (explicitly initialize everything to 0)")
      for(const auto& vec : edges) {
        const auto size = vec.size();
        if(size == 0) return;
        else          max.push_back(size); }
      do {
        par_edge_t par_edge;
        for(ha_index_t i = 0; i < ha_size; i++)
          par_edge.push_back(std::make_tuple(i, edges[i][num[i]]));
        res.push_back(std::move(par_edge));
      } while(!inc(max, num)); };

    const auto add_pairs = [this, &res](const auto& edges, const ha_index_t out_ha_index, const edge_index_t out_edge_index) {
      using tuple_t = typename par_edge_t::value_type;
      const tuple_t tuple(out_ha_index, out_edge_index);
      for(ha_index_t i = 0; i < ha_size; i++)
        if(i == out_ha_index) continue;
        else for(const auto e : edges[i]) {
          par_edge_t par_edge;
          par_edge.push_back(tuple);
          par_edge.push_back(tuple_t(i, e));
          res.push_back(par_edge); } };

    // sync and silent transitions
    for(ha_index_t i = 0; i < ha_size; i++) {
      std::unordered_set<lbl_t> visited_labels;
      for(const auto e : ha_seq[i].sync_edges(ids[i])) {
        const auto& label = ha_seq[i].edge(e).lbl();
        if(label.empty()) res.push_back(par_edge_t({std::make_tuple(i, e)}));
        else if(i == 0 && std::get<1>(visited_labels.insert(label))) // if i > 0 && !label.empty() then we do not need to consider edges with that label!
          add_all_combinations(syncs(label));                        // even when i == 0 we do not need to consider each label more than once
    } }

    if(ha_size > 1) { // input output/broadcast labels can be only enabled when there are more than one hybrid automaton
      // broadcast transitions
      for(ha_index_t i = 0; i < ha_size; i++)
        for(const auto e : ha_seq[i].broad_edges(ids[i])) {
          add_all_combinations(ins(ha_seq[i].edge(e).lbl(), i, e)); } 

      // output transitions
      for(ha_index_t i = 0; i < ha_size; i++)
        for(const auto e : ha_seq[i].out_edges(ids[i])) {
          add_pairs(ins(ha_seq[i].edge(e).lbl(), i, e), i, e); } 
    }

    const auto speedy_relation = [this](P& poly) {
      par_edge_varmap_t res;
      if(!sep_id_resets) return res;
      ppl::Variables_Set no_reset_prime_vars;
      for(ppl::dimension_type i = 0; i < dim; i++) { // order being increasing is important
        auto v1 = ppl::Variable(i);
        auto v2 = ppl::Variable(i+dim);
        if( ({ NNCPoly cpy(poly); cpy.add_constraint(v1 < v2); !cpy.is_empty(); }) ||
            ({ NNCPoly cpy(poly); cpy.add_constraint(v1 > v2); !cpy.is_empty(); })  ) 
          res.emplace(std::move(v1), ppl::Variable(i + dim - no_reset_prime_vars.size()));
        else  no_reset_prime_vars.insert(std::move(v2));} 
      remove_space_dimensions(poly, no_reset_prime_vars);
      poly = P(std::move(poly).minimized_constraints());
      return std::move(res); };

    pc::add_mc_poly_reached_edge_count(res.size());
    //filter unsatisfiable relations out and construct relation
    std::vector<par_edge_info_t> complete_res;
    for(auto& par_edge : res) {
      P poly(dim << 1);
      for(const auto& pair : par_edge)
        poly.intersection_assign(ha_seq[std::get<0>(pair)].edge(std::get<1>(pair)).rel());
      // we don't initially normalize the automata, even if we do that is not enough when there are more than one automata
      const auto dst = destination_of(ids, par_edge);
      auto src_inv = inv_of(ids);
      auto dst_inv = inv_of(dst);
      src_inv.concatenate_assign(std::move(dst_inv));
      poly.intersection_assign(std::move(src_inv)); 
      if(poly.is_empty()) continue;               
      pc::inc_mc_poly_nonempty_edge_count();
      //result_with_polies.emplace_back(std::move(par_edge), P(std::move(poly).minimized_constraints()));
      auto poly_edge_varmap = speedy_relation(poly);
      complete_res.emplace_back(std::move(par_edge),
                                std::move(poly),
                                std::move(poly_edge_varmap));
    }

    return std::get<0>(edges.insert(std::make_pair(ids, std::move(complete_res))))->second; 
  }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------
  
  void add_to_visiteds(const par_loc_id& ids, const P& poly) {
    auto expr = z3_utils::to_bool_expr(to_term(poly.minimized_constraints(), i2v), ctx); 
    visited_solver(ids).add((!expr).simplify());  }

  /** @param ids  loc-id of the parallel location
    * @param poly input polyhedron
    * @param add_to_set true iff the input polyhedron should be added to the set of visited points 
    * @return \c true iff the input set \c poly has already been completely visited in the input location \c ids. */ 
  bool is_visiteds(const par_loc_id& ids, const P& poly, const bool add_to_set) {
    const auto timer = pc::start_mc_poly_fixpoint();
    pc::inc_mc_poly_fixpoint_check();
    auto  expr = z3_utils::to_bool_expr(to_term(poly.minimized_constraints(), i2v), ctx); 
    auto& solver = visited_solver(ids);
    solver.push();
    solver.add(expr);
    const bool res = z3_utils::is_unsatisfiable(solver);
    solver.pop();
    if(res) {
      pc::inc_mc_poly_fixpoint_hit();
      return true; }
    else {
      if(add_to_set) solver.add((!expr).simplify()); 
      return false; } }

  bool is_unsafe(const par_loc_id& ids, const P& poly) {
    const auto timer = pc::start_mc_poly_unsafe_check();
    pc::inc_mc_poly_unsafe_check();
    auto iter = wanteds.find(ids);
    if(iter == wanteds.end()) return false;
    if(!iter->second)         return true ;
    auto& solver = *iter->second;
    const auto expr = z3_utils::to_bool_expr(to_term(poly, i2v), ctx); 
    solver.push();
    solver.add (expr);
    const bool res = z3_utils::is_satisfiable(solver);
    solver.pop();
    return res; }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------
  
  auto source_of(const par_loc_id& dst, const par_edge_t& edges) const {
    par_loc_id res = dst;
    for(const auto& id : edges) {
      const auto  ha_index = std::get<0>(id);
      const auto& edge     = ha_seq[ha_index].edge(std::get<1>(id));
      res[ha_index] = edge.src(); } 
    return res; }
  
  auto destination_of(const par_loc_id& src, const par_edge_t& edges) const {
    par_loc_id res = src;
    for(const auto& id : edges) {
      const auto  ha_index = std::get<0>(id);
      const auto& edge     = ha_seq[ha_index].edge(std::get<1>(id));
      res[ha_index] = edge.dst(); } 
    return res; }

  auto is_transient(const par_loc_id& ids) const {
    for(ha_index_t i = 0; i < ha_size; i++)
      if(ha_seq[i].loc(ids[i]).is_trans())
        return true;
    return false; }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------
  
  boost::optional<P> compute_disc_post_poly(P /* no & */ src, const par_edge_info_t& edge_info) const {
    pc::inc_mc_poly_disc_post();
    const auto& rel    = std::get<1>(edge_info);                        //< reset relation of the input edge
    const auto& varmap = std::get<2>(edge_info);                        //< mapping between a variable that does not have identity reset and its prime version
    src.add_space_dimensions_and_embed(sep_id_resets ? varmap.size() : dim);
    src.intersection_assign(rel);                                       //< compute the intersection
    if(src.is_empty()) return boost::none;                              //< if the edge was not enabled then terminate computation
    pc::inc_mc_poly_successful_disc_post();
    if(sep_id_resets) {
      if(!varmap.empty()) {
        ppl::Constraint_System css;                                     //< swap variables that do not have identity reset with their prime vesion
        css.set_space_dimension(dim);
        for(auto cs : src.constraints()) {
          for(auto& pair : varmap)
            cs.swap_space_dimensions(pair.first, pair.second);
          css.insert(std::move(cs)); }
        src = P(std::move(css));
        src.remove_higher_space_dimensions(dim); }                      //< remove extra dimensions
    } else remove_space_dimensions(src, poly_var_set);
    return std::move(src); }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------
  
  void check_safety() {
    safety_result_ = poly_safety_result::SAFE;
    while(!work_list_1.empty()) {
      pc::inc_mc_poly_iteration();
      for(auto w1_iter = work_list_1.begin(); w1_iter != work_list_1.end();) {
        const auto& src_par_loc = w1_iter->first ;                                     //< the source location
              auto& src_polies  = w1_iter->second;                                     //< sequence of polyhedra that need to be considered
        const auto  trans = is_transient(src_par_loc);                                 //< transient locations have no continuous time transitions
        while(!src_polies.empty()) {                                                   //< while there is more polyhedron to conside:
                auto& pair       = src_polies.front();        
          const auto& node_index = std::get<0>(pair);                                  //< a node index that took us here (used for making counterexample)
                auto& src_poly   = std::get<1>(pair);                                  //< a polyhedron to be considered
          auto visited = is_visiteds(src_par_loc, src_poly, add_to_visiteds_on_check); //< check if the current set is visited
          if(!visited) {
            if(!trans) {                                                               //< transient locations have no time transition
              const auto timer = pc::start_mc_poly_cont_post();
              pc::inc_mc_poly_cont_post();
              src_poly.time_elapse_assign(flow_of(src_par_loc)); }                     //< compute continuous post
            src_poly = P(std::move(src_poly).minimized_constraints());                 //< minimize polyhedron (optional step)
            if(!add_to_visiteds_on_check)
              add_to_visiteds(src_par_loc, src_poly);                                  //< add the current set to the set of visiteds states
            if(is_unsafe(src_par_loc, src_poly)) {                                     //< if we reached to an unsafe state then 
              safety_result_ = poly_safety_result::UNSAFE;                             //< the system is not safe
              fill_counter_example(src_par_loc, src_poly, node_index);                 //< construct a counterexample
              return; }                                                                //< terminate the computation
            const auto timer = pc::start_mc_poly_disc_post();
            const auto& par_edges = edges_of(src_par_loc);                    //< find all the possible outgoing edges of this location (parallel composition)
            for(const auto& par_edge_tuple : par_edges) {                     //< for each parallel edge: 
              auto dst_poly = compute_disc_post_poly(src_poly, par_edge_tuple);
              if(!dst_poly) continue;
              auto& par_edge   = std::get<0>(par_edge_tuple);
              auto dst_par_loc = destination_of(src_par_loc, par_edge);                    //< compute destination using current location and edges
              if(check_unsafe_after_disc && is_unsafe(dst_par_loc, *dst_poly)) {           //< optional check for unsafe state (to terminate sooner)
                safety_result_ = poly_safety_result::UNSAFE;
                fill_counter_example(src_par_loc, *dst_poly, node_index);                  
                return; }                                                     
              auto new_work    = std::make_tuple(path_nodes.size(), std::move(*dst_poly)); //< create a new work
              work_list_2[std::move(dst_par_loc)].push_back(std::move(new_work));          //< add the new work to the working list
              path_nodes.push_back(std::make_tuple(node_index, par_edge, 0));              //< add the edge to the path-nodes (used for making counterexample)
                                                                                           //< last element is arbitrary
            }
          }
          src_polies.pop_front(); // must be last statement of this loop
        } // poly-list while loop
        asserts_msg(src_polies.empty(), "error in code: |src_polies| = " << src_polies.size())
        w1_iter = work_list_1.erase(w1_iter); // must be last statement of this loop
      } // for loop
      asserts_msg(work_list_1.empty(), "error in code: w1.size = " << work_list_1.size())
      std::swap(work_list_1, work_list_2); 
      if(max_iter > 0 && ++cur_iter == max_iter) {
        log_warn << "Maximum number of iterations (" << max_iter << ") is reached. " 
                 << "If a non-linear hybrid automaton is being checked, this only means the current abstract system is bounded-step safe "
                 << "(number of steps in the concrete system for which the system remains safe could be less than " << max_iter << "). "
                 << "If a polyhedral hybrid automaton is being checked, this means the system is bounded-step safe.";
        safety_result_ = poly_safety_result::BOUNDED_SAFE;
        break; }
    } // work-list while loop
  }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  void construct_states_seq() {
    requires_msg(safety_result_ == poly_safety_result::UNSAFE, "the system is safe!")
    // intersect the input polyhedron with the invariants of the input parallel location
    const auto intersect_with_inv = [this](P& poly, const par_loc_id& par_id) { 
      for(ha_index_t i = 0; i < ha_size; i++)
        poly.intersection_assign(ha_seq[i].loc(par_id[i]).inv());  };
    
    const auto intersect_cont_post = [this](P& res, P src, const auto& par_id) {
      if(!is_transient(par_id)) 
        src.time_elapse_assign(flow_of(par_id));
      res.intersection_assign(src); };
    
    const auto intersect_inversed_cont_post = [this](P& res, P src, const auto& par_id) {
      if(!is_transient(par_id)) {
        auto flow = flow_of(par_id);
        flow = ppl_utils::negate_variables(flow);
        src.time_elapse_assign(flow);  }
      res.intersection_assign(src); };

    const auto intersect_disc_post = [this](P& res, P src, const auto& par_edge) {
      src.add_space_dimensions_and_embed(dim);
      for(const auto& pair : par_edge) {
        const auto ha_index   = std::get<0>(pair);
        const auto edge_index = std::get<1>(pair);
        src.intersection_assign(ha_seq[ha_index].edge(edge_index).rel()); }
      remove_space_dimensions(src, poly_var_set);
      res.intersection_assign(src); };

    const auto intersect_inversed_disc_post = [this](P& res, P src, const auto& par_edge) {
      src.add_space_dimensions_and_embed(dim);
      src.map_space_dimensions(ppl_utils::shift_dimensions(dim << 1));
      for(const auto& pair : par_edge) {
        const auto ha_index   = std::get<0>(pair);
        const auto edge_index = std::get<1>(pair);
        src.intersection_assign(ha_seq[ha_index].edge(edge_index).rel()); }
      src.remove_higher_space_dimensions(dim);
      res.intersection_assign(src); };

    const auto& ce = counter_example_;
    P universe;
    states_seq_t& res = states_seq_;
    res.reserve(2 * ce.path.size() + 2);
    universe.add_space_dimensions_and_embed(dim);

    /*
    const auto print = [&]() { 
      int c = 0;
      std::cout << "======================================================\n";
      for(const auto& set : res) {
        P cpy(set.minimized_constraints());
        ppl_utils::print_polyhedron(std::cout, cpy, i2v, "==", "", " && ");
        std::cout << "\n--------------" << ++c << "---------------------------------------\n";
      }
      std::cout << "======================================================\n"; };
    */

    { // initialize the result by the invariant of the visited locations (direction does not matter)
      auto par_id = ce.init_par_loc;
      res.push_back(universe);                            //< start with the true predicate (R^n)
      intersect_with_inv(res.back(), par_id);             //< intersect it with the invariant of the initial location
      res.push_back(res.back());                          //< copy it for the purpose of continuous transition
      for(const auto& par_edge : ce.path) {               //< for any edge along the path:
        res.push_back(universe);                          //< add the true predicaten to the end of \c res
        par_id = destination_of(par_id, par_edge);        //< find the destination of the edge
        intersect_with_inv(res.back(), par_id);           //< same as before
        res.push_back(res.back()); }                      //< same as before
      res.front().intersection_assign(ce.init_state  );   //< intersect the first state-set with the visited initial state
      res.back ().intersection_assign(ce.unsafe_state);   //< intersect the last  state-set with the visited unsafe  state
    }
    //print();
    { // forward direction (in case \c inversed_time is true, the whole safety problem has already been inversed)
      auto par_id = ce.init_par_loc;
      std::uint64_t i = 1;
      intersect_cont_post(res[1], res[0], par_id);
      for(const auto& par_edge : ce.path) {
        par_id = destination_of(par_id, par_edge);
        intersect_disc_post(res[i+1], res[i  ], par_edge);
        intersect_cont_post(res[i+2], res[i+1], par_id); 
        i += 2; }
    }
    //print();
    { // backward direction
      auto par_id = ce.unsafe_par_loc;
      std::uint64_t i = res.size() - 2;
      intersect_inversed_cont_post(res[i], res[i+1], par_id);
      const auto iter_end = ce.path.rend();
      for(auto iter = ce.path.rbegin(); iter != iter_end; iter++) {
        const auto par_edge = *iter;
        par_id = source_of(par_id, par_edge);
        intersect_inversed_disc_post(res[i-1], res[i  ], par_edge);
        intersect_inversed_cont_post(res[i-2], res[i-1], par_id); 
        i -= 2; }
    }
    //print();
    
    for(auto& poly : res) poly = P(poly.minimized_constraints());

    requires_msg(std::none_of(res.begin(), res.end(), [](const auto& poly) { return poly.is_empty(); }), 
                 "the counterexample is not a valid abstract counterexample")
  }  

private:
  decltype(pc::start_mc_poly_total         ()) total_timer          = pc::start_mc_poly_total         (); // the first two initializations in the
  decltype(pc::start_mc_poly_initialization()) initialization_timer = pc::start_mc_poly_initialization(); // top most class (hierarchically)

        safety_t    safety_;
  const ha_seq_t  & ha_seq ;
  const inits_t   & inits  ;
  const unsafes_t & unsafes;
  const pt::ptree & props  ;
  const id2var    & i2v    ;
  const name2var  & n2v    ;
  const ha_index_t  ha_size;
  const ppl::dimension_type dim;

  const bool compute_states_seq_ ;
  const bool inversed_time       ;
  const bool sep_id_resets       ;
  const bool check_unsafe_after_disc;
  const bool add_to_visiteds_on_check;
  const ppl::Variables_Set poly_var_set ;
  const val_uint_t max_iter;

  std::vector<path_node_t> path_nodes{1}; // the first element will be ignored
  work_list_t work_list_1;
  work_list_t work_list_2;

  std::unordered_map<par_loc_id, P                           , par_loc_hash_t > invs ;
  std::unordered_map<par_loc_id, P                           , par_loc_hash_t > flows;
  std::unordered_map<par_loc_id, std::vector<par_edge_info_t>, par_loc_hash_t > edges;

  z3::context       ctx;
  z3::solver        slv;
  visited_solvers_t visiteds;
  wanted_solvers_t  wanteds ;

  poly_safety_result safety_result_  ;
  counter_example_t  counter_example_;
  states_seq_t       states_seq_ ;
  val_uint_t         cur_iter = 0;

#define POLY_MC "mc-poly."
  static constexpr key_t     KEY_Direction = POLY_MC "direction";
  static constexpr val_str_t VAL_Direction_Forward  = "forward" ;
  static constexpr val_str_t VAL_Direction_Backward = "backward";
  static constexpr val_str_t VAL_Direction_Smaller_or_Forward  = "smaller-or-forward" ;
  static constexpr val_str_t VAL_Direction_Smaller_or_Backward = "smaller-or-backward";
  static constexpr val_str_t DEF_Direction = VAL_Direction_Smaller_or_Forward;

  static constexpr key_t      KEY_SepIDReset = POLY_MC "separate-identity-resets";
  static constexpr val_bool_t VAL_SepIDReset_True  = true ;
  static constexpr val_bool_t VAL_SepIDReset_False = false;
  static constexpr val_bool_t DEF_SepIDReset = VAL_SepIDReset_True;

  static constexpr key_t      KEY_CheckUnsafeAfterDiscEdge = POLY_MC "check-unsafe-after-disc-edge";
  static constexpr val_bool_t VAL_CheckUnsafeAfterDiscEdge_True  = true ;
  static constexpr val_bool_t VAL_CheckUnsafeAfterDiscEdge_False = false;
  static constexpr val_bool_t DEF_CheckUnsafeAfterDiscEdge = VAL_CheckUnsafeAfterDiscEdge_False;

  static constexpr key_t      KEY_AddToVisitedsOnCheck = POLY_MC "add-to-visiteds-on-check";
  static constexpr val_bool_t VAL_AddToVisitedsOnCheck_True  = true ;
  static constexpr val_bool_t VAL_AddToVisitedsOnCheck_False = false;
  static constexpr val_bool_t DEF_AddToVisitedsOnCheck = VAL_AddToVisitedsOnCheck_False;

  static constexpr key_t      KEY_MaxIter = POLY_MC "max-iter";
  static constexpr val_uint_t DEF_MaxIter = 0;
#undef  POLY_MC

};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

} // namespace ha::poly_ha

#endif // HA__POLY_HA_SAFETY_CHECKER__HPP
