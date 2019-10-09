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

#ifndef HA__NLFPOLY_HA_SAFETY_CHECKER__HPP
#define HA__NLFPOLY_HA_SAFETY_CHECKER__HPP

#include "hare/nlfpoly_ha.hpp"
#include "hare/poly_ha.hpp"
#include "hare/poly_ha_safety_checker.hpp"
#include "hare/safety_result.hpp"
#include "hare/utils_cegar.hpp"
#include "hare/utils_dreach.hpp"
#include "hare/utils_nlfpoly_ha.hpp"
#include "hare/utils_ppl.hpp"
#include "hare/utils_props.hpp"
#include "hare/utils_term.hpp"

#include "cmn/dbc.hpp"

#include <boost/numeric/interval.hpp>
#include <boost/property_tree/ptree.hpp>

#include <algorithm>
#include <cmath>                // nextafter
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace hare::nlfpoly_ha {

namespace pt  = boost::property_tree   ;
namespace ppl = Parma_Polyhedra_Library;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename P>
struct safety_checker {

  using abs_safety_checker_t = poly_ha::safety_checker<P>;

  using safety_t          = nlfpoly_safety<P>;
  using ha_t              = typename safety_t::ha_t      ;
  using loc_t             = typename ha_t::loc_t         ;
  using edge_t            = typename ha_t::edge_t        ;
  using locs_t            = typename ha_t::locs_t        ;
  using ha_seq_t          = typename safety_t::ha_seq_t  ;
  using ha_index_t        = typename safety_t::ha_index_t;
  using inits_t           = typename safety_t::inits_t   ;
  using unsafes_t         = typename safety_t::unsafes_t ;
  using edge_index_t      = typename ha_t::edge_index_t  ;
  using lbl_t             = typename ha_t::edge_t::lbl_t ;
  using par_edge_t        = typename safety_t::par_edge_t;
  using loc_index_t       = typename locs_t::size_type   ;
  using flow_map_t        = flow_map;

  using locs_backtrace_t  = std::vector<std::vector<loc_index_t >>;
  using edges_backtrace_t = std::vector<std::vector<edge_index_t>>;

  using abs_safety_t      = typename abs_safety_checker_t::safety_t    ;
  using abs_ha_t          = typename abs_safety_checker_t::ha_t        ;
  using abs_loc_t         = typename abs_ha_t::loc_t                   ;
  using abs_edge_t        = typename abs_ha_t::edge_t                  ;
  using abs_locs_t        = typename abs_ha_t::locs_t                  ;
  using abs_ha_seq_t      = typename abs_safety_checker_t::ha_seq_t    ;
  using abs_ha_index_t    = typename abs_safety_checker_t::ha_index_t  ;
  using abs_inits_t       = typename abs_safety_checker_t::inits_t     ;
  using abs_unsafes_t     = typename abs_safety_checker_t::unsafes_t   ;
  using abs_edge_index_t  = typename abs_safety_checker_t::edge_index_t;
  using abs_lbl_t         = typename abs_safety_checker_t::lbl_t       ;
  using abs_par_edge_t    = typename abs_safety_checker_t::par_edge_t  ;
  using abs_loc_index_t   = typename abs_locs_t::size_type             ;

  using dreach_utils_t = dreach_utils::ce_checker<P, safety_checker, abs_safety_checker_t>;

  static_assert(std::is_same<abs_ha_index_t  , ha_index_t  >::value);
  static_assert(std::is_same<abs_inits_t     , inits_t     >::value);
  static_assert(std::is_same<abs_unsafes_t   , unsafes_t   >::value);
  static_assert(std::is_same<abs_edge_index_t, edge_index_t>::value);
  static_assert(std::is_same<abs_lbl_t       , lbl_t       >::value);
  static_assert(std::is_same<abs_par_edge_t  , par_edge_t  >::value);
  static_assert(std::is_same<abs_loc_index_t , loc_index_t >::value);

  template<typename SS, std::enable_if_t<std::is_same<safety_t, std::remove_const_t<std::remove_reference_t<SS>>>::value>* = nullptr>
  safety_checker(SS&& safety) : bounded_cont_trans   (param_bounded_cont_trans   (safety.props())),
                                bound_ctran_by_eq    (param_bound_ctran_by_eq    (safety.props())),
                                connect_splitted_locs(param_connect_splitted_locs(safety.props())),
                                use_empty_labels     (param_use_empty_labels     (safety.props())),
                                ctran_duration       (param_ctran_duration       (safety.props())),
                                linear_flow_abst     (param_linear_flow_abstraction_method(safety.props())),
                                init_refine_count    (param_init_refine_count    (safety.props())),
                                bounding_var         (create_bounding_var        (safety)),
                                connector_rel        (create_connector_rel       (safety.vars().size() + (bounded_cont_trans ? 1 : 0))),
                                max_iter             (max_iter_param             (safety.props())),
                                safety_(bound_cont_transitions(std::forward<SS>(safety), 
                                                               bounded_cont_trans, 
                                                               use_empty_labels, 
                                                               bound_ctran_by_eq, 
                                                               ctran_duration, 
                                                               bounding_var)),
                                ha_seq (safety_.ha_seq ()),
                                inits  (safety_.inits  ()),
                                unsafes(safety_.unsafes()),
                                props  (safety_.props  ()), 
                                i2v    (safety_.i2v    ()),
                                n2v    (safety_.n2v    ()),
                                ha_size(ha_seq.size    ()), 
                                dim    (safety_.vars().size()),
                                poly_var_set(abs_safety_checker_t::create_poly_var_set(dim)),
                                abs_safety (initial_abstraction(safety_, linear_flow_abst)),
                                abs_ha_seq (abs_safety.ha_seq ()),
                                abs_inits  (abs_safety.inits  ()),
                                abs_unsafes(abs_safety.unsafes()) {

    log_debug << "starting initialization of nlfpoly-ha safety checker";
    initialize();
    initialization_timer.stop();
    {
      log_debug << "starting nlfpoly-ha cegar loop";
      const auto timer = pc::start_mc_nlfpoly_reachability();
      check_safety();
    }
    total_timer.stop();
    pc::set_mc_nlfpoly_safety_result(safety_result_);
    log_debug << "cegar loop in nlfpoly safety checker terminated (safety_result? " << safety_result_ << ")";
  }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------
  
  static auto max_iter_param(const pt::ptree& props) {
    return ::hare::prop_value<val_uint_t>(props, KEY_MaxIter, DEF_MaxIter); }
  
  static auto param_bounded_cont_trans(const pt::ptree& props) { 
    return ::hare::prop_value<val_bool_t>(props, KEY_BoundContTrans, DEF_BoundContTrans); }
  
  static auto param_bound_ctran_by_eq(const pt::ptree& props) { 
    return ::hare::prop_value<val_bool_t>(props, KEY_BoundCTranByEQ, DEF_BoundCTranByEQ); }

  static auto param_connect_splitted_locs(const pt::ptree& props) { 
    return ::hare::prop_value<val_bool_t>(props, KEY_ConnectSplittedLocs, DEF_ConnectSplittedLocs); }

  static auto param_use_empty_labels(const pt::ptree& props) { 
    return ::hare::prop_value<val_bool_t>(props, KEY_UseEmptyLabels, DEF_UseEmptyLabels); }

  // the trivial implimentation does not work on Linux!! Damn it GMP on Linux!
  static auto param_ctran_duration(const pt::ptree& props) { 
    #if __linux__    
    static std::string def_str = ({ 
                                    std::stringstream buff;
                                    buff << DEF_CTranDuration;
                                    buff.str(); 
                                  });
    const auto str_val = ::hare::prop_value<std::string>(props, KEY_CTranDuration, def_str); 
    return val_num_t(str_val); 
    #else    
    return ::hare::prop_value<val_num_t>(props, KEY_CTranDuration, DEF_CTranDuration); 
    #endif    
  }

  static val_uint_t param_init_refine_count(const pt::ptree& props) { 
    const auto res = ::hare::prop_value<val_uint_t>(props, KEY_InitRefineCount, DEF_InitRefineCount); 
    requires_msg(res <= 16, "maximum value for parameter " << KEY_InitRefineCount << "is 16 (input value is " << res << ")");
    return res; }

  static val_str_t param_linear_flow_abstraction_method(const pt::ptree& props) {
    const auto res = ::hare::prop_value<val_str_t>(props, KEY_LinearFlowAbstraction, DEF_LinearFlowAbstraction);
    requires_msg(res == VAL_LinearFlowAbstraction_Poly || res == VAL_LinearFlowAbstraction_Rect,
                 "invalid linear-flow-abstraction: " << res << ", possible values are " 
                 << VAL_LinearFlowAbstraction_Poly << " and " << VAL_LinearFlowAbstraction_Rect)
    if(res == VAL_LinearFlowAbstraction_Poly) return VAL_LinearFlowAbstraction_Poly;
    return VAL_LinearFlowAbstraction_Rect; }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------
  
  static auto create_bounding_var(const safety_t& safety) {
    const auto& n2v = safety.n2v();
    const std::string prefix = "z";
          std::string name   = prefix;
    std::uint64_t iter = 0;
    while(true) {
      if(n2v.find(name) == n2v.end()) return var(safety.n2v().size(), name);
      name = prefix + std::to_string(++iter);
      requires_msg(iter != 0, "too many variables: " << n2v.size()) } }

  static auto self_loop_label(const safety_t& safety) {
    const abs_lbl_t prefix = "self-loop";
          abs_lbl_t res    = prefix;
    std::uint64_t iter = 0;
    while(true) {
      bool unique = true;
      for(const auto& ha : safety.ha_seq())
        for(const auto& edge : ha.edges())
          if(res == edge.lbl()) {
            unique = false;
            goto OUT; }
      OUT:
      if(unique) return res;
      res = prefix + std::string("-") + std::to_string(++iter);
      requires_msg(iter != 0, "too many labels") } }

  template<typename SS>
  static auto bound_cont_transitions(SS&& safety, 
                                     const bool bound_cont_trans, 
                                     const bool use_empty_labels, 
                                     const bool bound_ctran_by_eq,
                                     cmn::pbv_t<val_num_t> ctran_duration, 
                                     cmn::pbv_t<term_ptr > bounding_var) {
    safety_t cpy(std::forward<SS>(safety));
    if(bound_cont_trans)
      force_discrete_edge_by_adding_self_loops_and_restricting_invariants<P, abs_lbl_t>(
        cpy, 
        ctran_duration, 
        use_empty_labels ? "" : self_loop_label(cpy), 
        bounding_var,
        bound_ctran_by_eq);
    else log_warn << "Continuous transitions are not explicitly bounded. "
                     "If a counterexample is found, it is possible that model checker refute it incorrectly. "
                     "If no counterexample is found, this warning can be ignored.";
    return std::move(cpy); }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  /* @note since the invariants will change, the result must not be cached */
  P abst_inv_of(const par_loc_id& ids) { 
    requires(ids.size() > 0)
    if(ha_size == 1) return abs_ha_seq[0].loc(ids[0]).inv();
    P res(dim);
    for(ha_index_t i = 0; i < ha_size; i++) 
      res.intersection_assign(abs_ha_seq[i].loc(ids[i]).inv());
    return std::move(res); }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  template<typename PP>
  bool split_loc(PP&& par_inv, const abs_ha_index_t ha_index, const abs_loc_index_t loc_index) {
    constexpr edge_index_t MAX_EDGE_INDEX = std::numeric_limits<edge_index_t>::max();
    auto& ha    = abs_ha_seq[ha_index];
    asserts_msg(ha_index < abs_ha_seq.size(), "ha_index=" << ha_index << ", |abs_ha_seq|=" << abs_ha_seq.size())
    asserts_msg(ha_index <     ha_seq.size(), "ha_index=" << ha_index << ", |    ha_seq|=" <<     ha_seq.size())
    auto& locs  = ha.locs();
    asserts_msg(loc_index < locs.size(), "loc_index=" << loc_index << ", |locs|=" << locs.size())
    auto& edges = ha.edges();
    /** add/remove unsafe/initial states. Remove a state-set if it has no intersection with the invariant of the updated location. Add a state-set if it has
      * intersection with the invariant of the new location. */
    const auto update_extreme = [ha_index](const auto old_id, const auto new_id, const auto& inv1, const auto& inv2, auto& seq) {
      std::remove_const_t<std::remove_reference_t<decltype(seq)>> new_tuples;
      auto iter = seq.begin();
      while(iter != seq.end()) {
        const auto& par_id = std::get<0>(*iter);
        const auto& poly   = std::get<1>(*iter);
        if(par_id[ha_index] != old_id) { 
          iter++;
          continue; }
        if(!poly.is_disjoint_from(inv2)) {
          par_loc_id new_par_id = par_id;
          new_par_id[ha_index]  = new_id;
          new_tuples.emplace_back(std::move(new_par_id), poly); } 
        if( poly.is_disjoint_from(inv1)) iter = seq.erase(iter);
        else                             iter++;
      }
      for(auto& tuple : new_tuples) 
        seq.push_back(std::move(tuple)); 
    };
    // checks whether an edge with the input source, destination, and relation, can ever be enabled
    const auto enabled_edge = [&locs](const auto src, const auto dst, const auto& rel) {
      auto inv_src_dst = locs[src].inv();
      inv_src_dst.concatenate_assign(locs[dst].inv());
      return !rel.is_disjoint_from(std::move(inv_src_dst)); };
    // if the new edge with the given soruce, destination, and relation, can ever be enabled, add it to the sequence of new edges.
    std::vector<std::tuple<edge_index_t, abs_edge_t>> new_edges;
    const auto add_edge = [&enabled_edge, &new_edges](const auto& old_edge, const auto edge_index, const auto src, const auto dst) {
      if(!enabled_edge(src, dst, old_edge.rel())) return;
      auto new_edge = abs_edge_t(src, dst, old_edge.lbl(), old_edge.knd(), old_edge.rel());
      new_edges.emplace_back(std::make_tuple(edge_index, std::move(new_edge)));  };
    // create the new location and update name, invariant, and flow of the old one. Also, update the location backtrace accordingly
    asserts_msg(loc_index < locs_backtrace[ha_index].size(), "loc_index=" << loc_index << "|locs_backtrace[ha_index]|=" << locs_backtrace[ha_index].size())
    const auto cnc_loc_index = locs_backtrace[ha_index][loc_index];
    asserts_msg(cnc_loc_index < ha_seq[ha_index].locs().size(), "cnc_loc_index=" << cnc_loc_index << ", |ha_seq[ha_index].locs|=" << ha_seq[ha_index].locs().size())
    const auto& flow = ha_seq[ha_index].loc(cnc_loc_index).flow();
    auto& loc1  = locs[loc_index];
    requires_msg(!loc1.is_trans(), "something is wrong! transient locations should not need splitting (they don't have any continuous dynamics)")
    P& inv1(loc1.inv());
    P  inv2(std::forward<PP>(par_inv));
    const bool splitted = split_inv(ha_index, loc_index, inv1, inv2, flow);
    if(!splitted)
      return false;
    auto name2 = loc1.name() + "_2"; 
    loc1.name() += "_1";
    loc1.flow() = abstract_flow(inv1, flow, linear_flow_abst);
    auto flow2  = abstract_flow(inv2, flow, linear_flow_abst);
    const auto id1 = loc1.id();
    const auto id2 = locs.size();
    abs_loc_t loc2(id2, std::move(name2), inv2, std::move(flow2), loc1.is_trans());
    locs_backtrace[ha_index].push_back(locs_backtrace[ha_index][id1]);
    // update initial and unsafe states
    update_extreme(id1, id2, inv1, inv2, abs_inits  ); 
    update_extreme(id1, id2, inv1, inv2, abs_unsafes); 
    // \c inv1 is a reference to a sub-element in \c locs. After the following step no such reference is valid.
    locs.push_back(std::move(loc2));
    // construct new edges and remove the old ones that are never enabled
    auto  edge_iter           = edges.begin();
    auto& edge_backtrace      = edges_backtrace[ha_index];
    auto  edge_backtrace_iter = edge_backtrace.begin();
    while(edge_iter != edges.end()) {
      const auto& edge = *edge_iter;
      const auto  src  = edge.src();
      const auto  dst  = edge.dst();
      const auto& rel  = edge.rel();
      if(src == id1)              add_edge(edge, *edge_backtrace_iter, id2, dst);
      if(dst == id1)              add_edge(edge, *edge_backtrace_iter, src, id2);
      if(src ==id1 && dst == id1) add_edge(edge, *edge_backtrace_iter, id2, id2);
      // the enabled_edge is rather an expensive check. So, optionally, we check it only when source or destination locations have been changed
      if((src == id1 || dst == id1) && !enabled_edge(src, dst, rel)) {
        edge_iter           = edges         .erase(edge_iter);
        edge_backtrace_iter = edge_backtrace.erase(edge_backtrace_iter);
      } else {
        edge_iter++;
        edge_backtrace_iter++; } }
    for(auto tuple : new_edges) {
      edge_backtrace.push_back(std::move(std::get<0>(tuple)));
      edges         .push_back(std::move(std::get<1>(tuple))); }
    /* 1.  if bound_ctran_by_eq is true, it means the extra edges will be guarded by <tt>z==d</tt> where \c z is the bounding variable and \c d is the 
     *     time-bound. These edges cannot be used to connect splitted locations, since the connector edges must be enabled at all possible times.
     * 1.  if use_empty_labels is false, it means the extra edges that bound duration of continuous transitions will be synchoronized across different 
     *     automata. So they cannot appear independently, which makes them not usable for connector edges.
     * 1.  if bounded_cont_trans is false, it means no extra edges has been added to the locations. So we need connector edges.
     * If none of the above conditions are true, i.e. duration of continuous transitions are bounded using self-loops, labels of the self-loops are empty, and
     * guard of these slef-loops are <tt>(z<=d)</tt> then we don't need to explicitly connect these edges. */
    if(bound_ctran_by_eq || !use_empty_labels || !bounded_cont_trans) { 
      if(connect_splitted_locs) {
        edge_backtrace.push_back(MAX_EDGE_INDEX);
        edge_backtrace.push_back(MAX_EDGE_INDEX);
        edges.emplace_back(id1, id2, "" /*always empty*/, label_kind::SYNC, connector_rel);
        edges.emplace_back(id2, id1, "" /*always empty*/, label_kind::SYNC, connector_rel); 
        requires_msg(edges.size() < MAX_EDGE_INDEX, "too many edges! MAX_EDGE_INDEX=" << MAX_EDGE_INDEX) }
      else log_warn << "location " << loc_index << " in automaton at index " << ha_index << " is splitted, "
                    << "but no effort has been made to directly connect the new locations ("
                    << "bounded_cont_trans=" << bounded_cont_trans << ", "
                    << "use_empty_labels="   << use_empty_labels   << ", " 
                    << "bound_ctran_by_eq="  << bound_ctran_by_eq  << ")"; }
    return true;
  } 

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  bool split_inv(const ha_index_t ha_index, const abs_loc_index_t loc_index, P& inv1, P& inv2, const flow_map_t& flow) const { 
    // only used in \c requires_msg
    const auto to_string = [](const auto& poly, const auto& i2v, const auto& flow) { 
      std::stringstream buff; 
      buff << "INV: ";
      P cpy(poly.minimized_constraints());
      print_polyhedron(buff, cpy, i2v, "==", "", " && "); 
      buff << std::endl << "FLOW: ";
      for(const auto& vf : flow)
        buff << std::get<0>(vf) << "' == " << std::get<1>(vf) << std::endl;
      return buff.str(); };
    requires(dim > 0)
    // split the captured invariant using the two input constraints (called at most once)
    const auto split = [this, &inv1, &inv2, ha_index, loc_index](const auto& term_range, const auto& mid, ppl::Constraint&& cs1, ppl::Constraint&& cs2) {
      log_debug << "[ha_index,loc_index]=[" << ha_index << ", " << loc_index << "] "
                << "splitting invariant on " << std::get<0>(term_range) << "==" << (mid < 0 ? "-" : "") << (mid < 0 ? -mid : mid) 
                << " when range is " << std::get<1>(term_range);
      ppl::Constraint_System css1; css1.set_space_dimension(dim); css1.insert(std::move(cs1)); inv1.intersection_assign(P(std::move(css1)));
      ppl::Constraint_System css2; css2.set_space_dimension(dim); css2.insert(std::move(cs2)); inv2.intersection_assign(P(std::move(css2))); 
      inv1 = P(inv1.minimized_constraints());
      inv2 = P(inv2.minimized_constraints());
    };
    std::unordered_set<term_ptr> flow_rhs_vars;
    for(const auto& vf : flow)
      for(const auto& v : ::cmn::math::term::variables_of(std::get<1>(vf)))
        flow_rhs_vars.insert(v);
    std::vector<std::tuple<term_ptr, interval_t>> candidates;
    candidates.reserve(flow_rhs_vars.size());
    for(const auto& v : flow_rhs_vars) 
      candidates.emplace_back(v, ppl_utils::bound_of_var(v, inv2));
    const auto split_iter = std::max_element(candidates.begin(), candidates.end(), [](const auto& fst, const auto& snd) {
      const auto fst_len = std::get<1>(fst).sup() - std::get<1>(fst).inf();
      const auto snd_len = std::get<1>(snd).sup() - std::get<1>(snd).inf();
      return fst_len.is_finite() && fst_len < snd_len; } );
    if(split_iter == candidates.end()) {
      log_trace << "[ha_index,loc_index]=[" << ha_index << ", " << loc_index << "] "
                << "no variable found to be split. No non-time-bounding variable is appeared in the RHS of a flow. " << to_string(inv2, i2v, flow);
      return false; }
    const ppl::Variable var(std::get<0>(*split_iter)->variable().id());
    const auto&         rng(std::get<1>(*split_iter));
    constant mid = rng.is_left_unbounded() ? rng.is_right_unbounded() ?  constant(0) 
                                                                      :  rng.sup().value() < 0 ? rng.sup().value() * 3 : constant(-3) // any positive number
                                           : rng.is_right_unbounded() ?  rng.inf().value() > 0 ? rng.inf().value() * 3 : constant( 3) // other than 2 works!
                                           : (rng.inf() + rng.sup()).value() / 2;
    const mpz_class mid_num(mid.get_num());
    const mpz_class mid_den(mid.get_den());
    const auto split_expr_lhs = mid_den * var;
    split(*split_iter, mid, split_expr_lhs >= mid_num, split_expr_lhs <= mid_num);  
    return true; }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  static auto abstract_flow(const P& inv, const flow_map_t& flow_map, cmn::pbv_t<val_str_t> linear_flow_abst) {
    using D = float;
    const auto dim = inv.space_dimension();
    P res(inv);
    ppl::Constraint_System intervals;
    intervals.set_space_dimension(dim);
    const auto add_interval = [](auto& intervals, const auto& var, const auto& interval) {
      if(interval.is_left_bounded()) {
        const auto& inf = interval.inf().value();
        if(interval.is_left_closed()) intervals.insert(var * inf.get_den() >= inf.get_num());
        else                          intervals.insert(var * inf.get_den() >  inf.get_num()); }
      if(interval.is_right_bounded()) {
        const auto& sup = interval.sup().value();
        if(interval.is_right_closed()) intervals.insert(var * sup.get_den() <= sup.get_num());
        else                           intervals.insert(var * sup.get_den() <  sup.get_num()); } };
    res.add_space_dimensions_and_embed(dim);
    boost::optional<std::unordered_map<term_ptr, boost_interval_t<D>>> bounds = boost::none;
    for(const auto& vf : flow_map) {
      const auto& v = std::get<0>(vf);
      const auto& f = std::get<1>(vf);
      bool linear;
      if((linear_flow_abst == VAL_LinearFlowAbstraction_Poly && (linear=is_linear(f))) || is_ground_term(f)) {
        auto lcm  = lcm_of_denominators(f);
        log_trace << "polyhedronizing flow d[" << v << "]/dt=" << f << ", LCM=" << lcm;
        auto expr = ppl_utils::to_linear_expr(f, lcm);
        res.affine_image(ppl::Variable(v->variable().id()+dim), std::move(expr), std::move(lcm)); }
      else {
        log_trace << "rectangularizing flow d[" << v << "]/dt=" << f;
        if(!bounds) {
          std::unordered_map<term_ptr, boost_interval_t<D>> map;
          map.reserve(flow_map.size());
          for(const auto& pair : flow_map) {
            const auto& vv = std::get<0>(pair);
            const auto& ff = std::get<1>(pair);
            if(map.find(vv) == map.end())
              map.emplace(vv, to_boost_interval<D>(ppl_utils::bound_of_var(vv, inv))); 
            for(const auto& vvv : variables_of(ff))
              if(map.find(vvv) == map.end())
                map.emplace(vvv, to_boost_interval<D>(ppl_utils::bound_of_var(vvv, inv))); }
          bounds = std::move(map); }
        const auto bound = to_cmn_math_interval<D>(bound_of_expr(*bounds, f));
        log_trace << "rectangularizing flow d[" << v << "]/dt=" << f << " to " << bound;
        add_interval(intervals, ppl::Variable(v->variable().id()), bound); 
      } 
    }
    res.map_space_dimensions(ppl_utils::shift_dimensions(dim << 1));
    res.remove_higher_space_dimensions(dim);
    if(!intervals.empty())
      res.intersection_assign(P(std::move(intervals)));
    return std::move(res);  }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  static auto initial_abstraction(const safety_t& safety, cmn::pbv_t<val_str_t> linear_flow_abst) {
    const auto timer = pc::start_mc_nlfpoly_abstraction();
    using abs_locs_t = typename abs_ha_t::locs_t;
    abs_ha_seq_t abs_ha_seq;
    abs_ha_seq.reserve(safety.ha_seq().size());
    for(const auto& ha : safety.ha_seq()) {
      abs_locs_t abs_locs;
      for(const auto& loc : ha.locs())
        abs_locs.emplace_back(loc.id(), loc.name(), loc.inv(), abstract_flow(loc.inv(), loc.flow(), linear_flow_abst), loc.is_trans());
      abs_ha_seq.emplace_back(ha.name(), ha.vars(), std::move(abs_locs), ha.edges());  }
    return abs_safety_t(std::move(abs_ha_seq), safety.inits(), safety.unsafes(), safety.props()); }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------
  
  /* @brief create the relation of the connector edges. A connector edge has no correspondent edge in the concrete automaton. It connects splitted locations 
   *        (in both directions) so the abstracted continuous dynamics can change while in the concrete automata no discrete transition happened (so the 
   *        location is not changed).
   * @return the idendity relation. It means it maps every variable, including the bounding variable, to itselt. */ 
  static auto create_connector_rel(const ppl::dimension_type dim) {
    ppl::Constraint_System css;
    css.set_space_dimension(dim << 1);
    for(ppl::dimension_type i = 0; i < dim; i++)
      css.insert( ppl::Variable(i) == ppl::Variable(i+dim)  );
    return P(std::move(css)); }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  void initialize_backtraces() {
    for(ha_index_t i = 0; i < ha_size; i++) {
      typename locs_backtrace_t ::value_type loc_backtrace ;
      typename edges_backtrace_t::value_type edge_backtrace;
      const auto loc_size  = ha_seq[i].locs ().size();
      const auto edge_size = ha_seq[i].edges().size();
      for(loc_index_t  j = 0; j < loc_size ; j++) loc_backtrace .push_back(j);
      for(edge_index_t j = 0; j < edge_size; j++) edge_backtrace.push_back(j);
      locs_backtrace .push_back(std::move(loc_backtrace ));
      edges_backtrace.push_back(std::move(edge_backtrace));  }  }

  void initialize() {
    initialize_backtraces();
  }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  nlfpoly_safety_result safety_result() const { return safety_result_; }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  /** construct an string representation of the abstract counterexample (for the pourpose of logging) */
  auto abst_ce_path_to_string(par_loc_id par_id, const typename abs_safety_checker_t::counter_example_t::path_t& path) {
    std::stringstream buff;
    const auto destination_of = [this](const par_loc_id& src, const par_edge_t& edges) {
      par_loc_id res = src;
      for(const auto& id : edges) {
        const auto ha_index   = std::get<0>(id);
        const auto edge_index = std::get<1>(id);
        const auto& edges = abs_ha_seq[ha_index].edges();
        requires_msg(edge_index < edges.size(), "ha_index=" << ha_index << ", edge_index=" << edge_index << ", |edges|=" << edges.size())
        const auto& edge = edges[edge_index];
        res[ha_index] = edge.dst(); } 
      return res; };
    abs_safety_checker_t::print_par_loc_id(buff, par_id);
    for(const auto& edge : path) {
      par_id = destination_of(par_id, edge);
      buff << " --> "; 
      abs_safety_checker_t::print_par_loc_id(buff, par_id); }
    return buff.str();  }

  /** construct an string representation of the concrete counterexample (for the pourpose of logging) */
  auto conc_ce_path_to_string(par_loc_id par_id, const typename abs_safety_checker_t::counter_example_t::path_t& path) {
    std::stringstream buff;
    par_id = cegar::concretize_par_loc_id<P>(par_id, locs_backtrace, ha_seq);
    abs_safety_checker_t::print_par_loc_id(buff, par_id);
    for(const auto& edge : path) {
      const auto cnc_par_edge = cegar::concretize_par_edge_idx<P>(edge, edges_backtrace, ha_seq); 
      if(cnc_par_edge)
        par_id = dreach_utils_t::cnc_dest_of(par_id, *cnc_par_edge, ha_seq);
      buff << " --> "; 
      abs_safety_checker_t::print_par_loc_id(buff, par_id); }
    return buff.str();  }

  template<typename Out>
  Out& print_concrete_counter_example(Out& out, cmn::pbv_t<par_loc_id> init_par_id, const typename abs_safety_checker_t::counter_example_t::path_t& path) const { 
    auto par_id = cegar::concretize_par_loc_id<P>(init_par_id, locs_backtrace, ha_seq);
    abs_safety_checker_t::print_par_loc_id(out, par_id);
    for(const auto& par_edge : path) {
      const auto cnc_par_edge = cegar::concretize_par_edge_idx<P>(par_edge, edges_backtrace, ha_seq); 
      out << " --"; 
      if(cnc_par_edge) {
        bool first = true; 
				for(const auto& pair : *cnc_par_edge) {
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
					out << edge_index; } }
      out << "--> "; 
      abs_safety_checker_t::print_par_loc_id(out, par_id); }
		return out; }

  template<typename Out>
  Out& print_concrete_counter_example(Out& out) const { 
    requires_msg(safety_result_ == nlfpoly_safety_result::UNSAFE || safety_result_ == nlfpoly_safety_result::UNKNOWN, "safety_result=" << safety_result_)
    return print_concrete_counter_example(out, abs_mc->counter_example().init_par_loc, abs_mc->counter_example().path); }

  template<typename Out>
  Out& print_abstract_counter_example(Out& out) const { 
    requires_msg(safety_result_ == nlfpoly_safety_result::UNSAFE || safety_result_ == nlfpoly_safety_result::UNKNOWN, "safety_result=" << safety_result_)
		return abs_mc->print_counter_example(out); }

  template<typename Out>
  Out& print_annotated_counter_example(Out& out) const { 
    requires_msg(safety_result_ == nlfpoly_safety_result::UNSAFE || safety_result_ == nlfpoly_safety_result::UNKNOWN, "safety_result=" << safety_result_)
    return abs_mc->print_annotated_counter_example(out); }

  template<typename Out>
  Out& print_reachable_states(Out& out) const { 
    for(const auto& pair : abs_mc->visited_solvers()) {
      const auto& solver     = std::get<1>(pair);
      const auto& abs_par_id = std::get<0>(pair);
      const auto  cnc_par_id = cegar::concretize_par_loc_id<P>(abs_par_id, locs_backtrace, ha_seq);
      abs_safety_checker_t::print_par_loc_id(out << "Location Abstract/Concrete ", abs_par_id);
      abs_safety_checker_t::print_par_loc_id(out << "/"                           ,cnc_par_id) << ": ";
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

  void initial_split() {
    if(init_refine_count > 0) {
      requires_msg(ha_size == 1, "initially splitting locations when there are more than one automata is no supported. Set " << KEY_InitRefineCount << "to 0")
      auto& ha = abs_ha_seq[0];
      par_loc_id par_id(1);
      for(val_uint_t r = 0; r < init_refine_count; r++)
        for(loc_index_t q = ha.locs().size(); q > 0;) {
          par_id[0] = --q;
          split_loc(abst_inv_of(par_id), 0, par_id[0]); }
      ha.refill_edge_indices();  }  }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  void check_safety() {
    initial_split();

    // convert the input id to string (only for the pourpose of debugging)
    const auto to_str = [](const par_loc_id& par_id) {
      if(par_id.size() == 1) return std::to_string(par_id[0]);
      std::stringstream buff;
      buff << "[";
      bool first = true;
      for(const auto id : par_id) {
        if(first) first = false; else buff << ",";
        buff << std::to_string(id); }
      buff << "]";
      return buff.str();  };

    const boost::optional<term_ptr> time_var = bounding_var;
    while(true) {
      pc::inc_mc_nlfpoly_iteration();
      abs_mc = std::make_unique<abs_safety_checker_t>(abs_safety, true);
      const auto abs_safety_res = abs_mc->safety_result();
      if(abs_safety_res == poly_safety_result::SAFE        ) { safety_result_ = nlfpoly_safety_result::SAFE;                  break; } 
      if(abs_safety_res == poly_safety_result::BOUNDED_SAFE) { safety_result_ = nlfpoly_safety_result::ABSTRACT_BOUNDED_SAFE; break; } 
      const auto& ce = abs_mc->counter_example();
      const auto  ce_len = ce.path.size();
      log_debug << "counter-example length (number of edges): " << ce_len; 
      log_debug << "abst-CE: " << abst_ce_path_to_string(ce.init_par_loc, ce.path);
      log_debug << "conc-CE: " << conc_ce_path_to_string(ce.init_par_loc, ce.path);
      const auto index =
      ({
        const auto timer = pc::start_mc_nlfpoly_ce_check();
        dreach_utils_t dreach(safety_, 
                              abs_safety,
                              abs_mc->counter_example(), 
                              abs_mc->states_seq(), 
                              locs_backtrace, 
                              edges_backtrace, 
                              bounded_cont_trans ? time_var : boost::none, 
                              ctran_duration);
        dreach.longest_valid_prefix();
      });
      log_debug << "max valid index: " << index;
      if(index == ce_len + 1) {
        safety_result_ = nlfpoly_safety_result::UNSAFE;
        // TODO backtrace edges and construct a counter example
        break; }
      if(max_iter > 0 && ++cur_iter == max_iter) {
        log_warn << "Maximum number of iterations (" << max_iter << ") is reached. " 
                 << "CEGAR loop did not terminate after this number of iterations.";
        safety_result_ = nlfpoly_safety_result::UNKNOWN;
        break; }
      { // refinement
        const auto timer = pc::start_mc_nlfpoly_refinement();
        auto par_id = ce.init_par_loc;
        std::remove_const_t<std::remove_reference_t<decltype(index)>> i = 0;
        for(const auto edge : ce.path) {
          if(i++ >= index) break;
          par_id = abs_mc->destination_of(par_id, edge); }
        log_debug << "splitting location: " << to_str(par_id);
        bool splitted_some_loc = false;
        auto par_inv = abst_inv_of(par_id);
        for(ha_index_t i = 0; i < ha_size; i++) {
          const bool splitted = i == ha_size - 1 ? split_loc(std::move            (par_inv), i, par_id[i])  :
                                                   split_loc(static_cast<const P&>(par_inv), i, par_id[i])  ;
          splitted_some_loc |= splitted;
          if(splitted)
            abs_ha_seq[i].refill_edge_indices();  }
        requires_msg(splitted_some_loc, "no location in any of the automata are splitted")
      }
    }
  }

private:
  decltype(pc::start_mc_nlfpoly_total         ()) total_timer          = pc::start_mc_nlfpoly_total         (); // the first two initializations in the
  decltype(pc::start_mc_nlfpoly_initialization()) initialization_timer = pc::start_mc_nlfpoly_initialization(); // top most class (hierarchically)

  const val_bool_t bounded_cont_trans   ;
  const val_bool_t bound_ctran_by_eq    ;
  const val_bool_t connect_splitted_locs;
  const val_bool_t use_empty_labels     ; 
  const val_num_t  ctran_duration       ;
  const val_str_t  linear_flow_abst     ;
  const val_uint_t init_refine_count    ;
  const term_ptr   bounding_var         ;
  const P          connector_rel        ;
  const val_uint_t max_iter;

        safety_t    safety_; 
  const ha_seq_t  & ha_seq ;
  const inits_t   & inits  ;
  const unsafes_t & unsafes;
  const pt::ptree & props  ;
  const id2var    & i2v    ;
  const name2var  & n2v    ;
  const ha_index_t  ha_size;
  const ppl::dimension_type dim;
  const ppl::Variables_Set poly_var_set ;

  abs_safety_t    abs_safety ;
  abs_ha_seq_t  & abs_ha_seq ;
  abs_inits_t   & abs_inits  ;
  abs_unsafes_t & abs_unsafes;

  std::unique_ptr<abs_safety_checker_t> abs_mc;

  locs_backtrace_t  locs_backtrace ;
  edges_backtrace_t edges_backtrace;

  nlfpoly_safety_result safety_result_  ;
  val_uint_t            cur_iter = 0;

#define NLFPOLY_MC "mc-nlfpoly."
  /** @brief Whether or not continuous transitions in the abstract system should be bounded. 
    * @brief Setting value of this parameter to \c false generates a warning, because it becomes user's responsibility to make sure that either there is a
    *        bound in the abstract automata any way or conrete model checker can handle infinite time transitions. */
  static constexpr key_t      KEY_BoundContTrans = NLFPOLY_MC "bound-cont-trans";
  static constexpr val_bool_t VAL_BoundContTrans_True  = true ;
  static constexpr val_bool_t VAL_BoundContTrans_False = false;
  static constexpr val_bool_t DEF_BoundContTrans = VAL_BoundContTrans_True;

  /** @brief non-empty label syncs. In Parallel composition this reduces number of discrete transitions. */
  static constexpr key_t      KEY_UseEmptyLabels = NLFPOLY_MC "use-empty-labels-for-bounding-time";
  static constexpr val_bool_t VAL_UseEmptyLabels_True  = true ;
  static constexpr val_bool_t VAL_UseEmptyLabels_False = false;
  static constexpr val_bool_t DEF_UseEmptyLabels = VAL_UseEmptyLabels_True;

  /** @brief Whether or not refinement should add edges between the splitted locations. 
    * @note  At the time of writing this comment, this property will be ignored if <tt>bound-cont-trans<\tt> abd <tt>use-empty-labels-for-bounding-time</tt> 
    *        are both \c true. Duration of continuous transitions are bounded in abstract system by initially adding self loops to the concrete system. When 
    *        hybrid automata have those special-purpose self-loops with empty labels (so they won't sync with other edges) splitted locations will 
    *        automatically be added. 
    * @note  Setting values of both <tt>bound-cont-trans</tt> and <tt>connect-splitting-locs</tt> to \c false generates a warning. Because it becomes the 
    *        user's responsibility to make sure that after splitting locations of the automata under consideration, the splitted locations will automatically 
    *        be connected. */
  static constexpr key_t      KEY_ConnectSplittedLocs = NLFPOLY_MC "connect-splitted-locs";
  static constexpr val_bool_t VAL_ConnectSplittedLocs_True  = true ;
  static constexpr val_bool_t VAL_ConnectSplittedLocs_False = false;
  static constexpr val_bool_t DEF_ConnectSplittedLocs = VAL_ConnectSplittedLocs_True;

  /** @brief whether or not the new edges that bound time use equality in their guards (as opposed to non-strict equality) */
  static constexpr key_t      KEY_BoundCTranByEQ = NLFPOLY_MC "bound-cont-trans-by-eq";
  static constexpr val_bool_t VAL_BoundCTranByEQ_True  = true ;
  static constexpr val_bool_t VAL_BoundCTranByEQ_False = false;
  static constexpr val_bool_t DEF_BoundCTranByEQ = VAL_BoundCTranByEQ_True;

  /** @brief The bound on duration on each continuous transition. 
    * @note  Even if <tt>bound-cont-trans</tt> is \c false, this parameter is used in verifying counterexamples. If <tt>bound-cont-trans</tt> is \c true, 
    *        <tt>cont-tran-duration</tt> is also used to force the duration on abstract model checkings.
    * @note  The value for this property must be non-negative. Zero means no bound.
    * @note  At the time of writing this comment, only positive values are supported. */
  static constexpr key_t     KEY_CTranDuration = NLFPOLY_MC "cont-tran-duration";
  static const     val_num_t DEF_CTranDuration;

  static constexpr key_t     KEY_LinearFlowAbstraction      = NLFPOLY_MC "linear-flow-abstraction";
  static constexpr val_str_t VAL_LinearFlowAbstraction_Poly = "polyhedronize" ;
  static constexpr val_str_t VAL_LinearFlowAbstraction_Rect = "rectangularize";
  static constexpr val_str_t DEF_LinearFlowAbstraction      = VAL_LinearFlowAbstraction_Poly;

  static constexpr key_t      KEY_InitRefineCount = NLFPOLY_MC "initial-refinement-count";
  static constexpr val_uint_t DEF_InitRefineCount = 0;

  static constexpr key_t      KEY_MaxIter = NLFPOLY_MC "max-iter";
  static constexpr val_uint_t DEF_MaxIter = 0;
#undef  NLFPOLY_MC
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename P> const ::hare::val_num_t safety_checker<P>::DEF_CTranDuration = ::hare::val_num_t(10/*any positive*/); 

} // namespace hare::nlfpoly_ha

#endif // HA__NLFPOLY_HA_SAFETY_CHECKER__HPP
