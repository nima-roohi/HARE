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

#ifndef HA__UTILS_DREACH__HPP
#define HA__UTILS_DREACH__HPP

#include "hare/nlfpoly_ha.hpp"
#include "hare/poly_ha.hpp"
#include "hare/term.hpp"
#include "hare/performance_counter.hpp"
#include "hare/utils_cegar.hpp"
#include "hare/utils_props.hpp"

#include "cmn/dbc.hpp"
#include "cmn/type_traits.hpp"
#include "cmn/misc/hash.hpp"

#include <gmpxx.h>
#include <ppl.hh>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#include <cstdlib>    // system
#include <fstream> 
#include <tuple>
#include <type_traits>
#include <utility>

namespace hare::dreach_utils {

namespace ppl = Parma_Polyhedra_Library;
namespace pt  = boost::property_tree;

using cnc_par_loc_id = nlfpoly_ha::par_loc_id;
using abs_par_loc_id =    poly_ha::par_loc_id;
static_assert(std::is_same<cnc_par_loc_id, abs_par_loc_id>::value);

template<typename P, typename ConcMC, typename AbstMC>
struct ce_checker {
  using cnc_mc_t = ConcMC;
  using abs_mc_t = AbstMC;

  using cnc_safety_t = typename cnc_mc_t::safety_t;
  using abs_safety_t = typename cnc_mc_t::abs_safety_t;

  using abs_ha_seq_t = typename abs_safety_t::ha_seq_t;
  using cnc_ha_seq_t = typename cnc_safety_t::ha_seq_t;

  using locs_backtrace_t  = typename cnc_mc_t::locs_backtrace_t;
  using edges_backtrace_t = typename cnc_mc_t::edges_backtrace_t;

  using abs_loc_id_t       =    poly_ha::loc_id;
  using cnc_loc_id_t       = nlfpoly_ha::loc_id;
  using cnc_par_loc_id_t   = nlfpoly_ha::par_loc_id;
  using abs_par_loc_id_t   =    poly_ha::par_loc_id;
  using cnc_par_edge_idx_t = typename cnc_safety_t::par_edge_t;
  using abs_par_edge_idx_t = typename abs_safety_t::par_edge_t;
  using cnc_flow_map_t     = nlfpoly_ha::flow_map;

  using abs_counter_example_t = typename abs_mc_t::counter_example_t;
  using abs_states_seq_t      = typename abs_mc_t::states_seq_t;
  using ce_path_t             = typename abs_counter_example_t::path_t;

  using ha_index_t = typename cnc_safety_t::ha_index_t;
  static_assert(std::is_same<ha_index_t, typename abs_safety_t::ha_index_t>::value);

  using abs_par_loc_id_hash_t   = cmn::misc::sequence_hash<abs_loc_id_t, abs_par_loc_id_t>;
  using cnc_par_loc_id_hash_t   = cmn::misc::sequence_hash<cnc_loc_id_t, cnc_par_loc_id_t>;
  using cnc_par_edge_idx_hash_t = cmn::misc::sequence_hash<typename abs_par_edge_idx_t::value_type, 
                                                           abs_par_edge_idx_t, 
                                                           cmn::misc::tuple_hash<typename abs_par_edge_idx_t::value_type>>;

  using mode_t = std::uint64_t;

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------
  
  ce_checker(const cnc_safety_t             & cnc_safety,
             const abs_safety_t             & abs_safety,
             const abs_counter_example_t    & counter_example,
             const abs_states_seq_t         & states_seq,
             const locs_backtrace_t         & locs_backtrace,
             const edges_backtrace_t        & edges_backtrace, 
             const boost::optional<term_ptr>& time_var, 
             const constant                 & single_cont_trans_time_bound) : cnc_safety                   (cnc_safety),
                                                                              cnc_ha_seq                   (cnc_safety.ha_seq()),
                                                                              abs_safety                   (abs_safety),
                                                                              abs_ha_seq                   (abs_safety.ha_seq()),
                                                                              vars                         (cnc_safety.vars()),
                                                                              i2v                          (cnc_safety.i2v()),
                                                                              n2v                          (cnc_safety.n2v()),
                                                                              ce                           (counter_example),
                                                                              states_seq                   (states_seq),
                                                                              locs_backtrace               (locs_backtrace),
                                                                              edges_backtrace              (edges_backtrace),
                                                                              time_var                     (time_var),
                                                                              single_cont_trans_time_bound (single_cont_trans_time_bound),
                                                                              size                         (cnc_ha_seq.size()),
                                                                              dim                          (vars.size()),
                                                                              id_rel                       (create_identity_rel(dim, time_var)) { }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  auto dreach_path(const pt::ptree& props) const { 
    static val_str_t DEF_Path = 
      #if __linux__
        "/opt/dReal-3.16.06.02/bin/dReach";
      #else
        "/opt/dReal-3.16.06.02/bin/dReach";
      #endif // __linux__
     return ::hare::prop_value<val_str_t>(props, KEY_Path, DEF_Path); }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  auto conc_par_loc_id  (const abs_par_loc_id_t  & par_id ) { return cegar::concretize_par_loc_id  <P>(par_id , locs_backtrace , cnc_ha_seq); }
  auto conc_par_edge_idx(const abs_par_edge_idx_t& par_idx) { return cegar::concretize_par_edge_idx<P>(par_idx, edges_backtrace, cnc_ha_seq); }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  static auto cnc_dest_of(const cnc_par_loc_id_t& src, const cnc_par_edge_idx_t& par_idx, const cnc_ha_seq_t& cnc_ha_seq) {
    auto res = src;
    for(const auto& id : par_idx) {
      const auto  ha_index   = std::get<0>(id);
      const auto  edge_index = std::get<1>(id);
      asserts_msg(ha_index < cnc_ha_seq.size(), "ha_index=" << ha_index << ", |cnc_ha_seq|=" << cnc_ha_seq.size())
      asserts_msg(edge_index < cnc_ha_seq[ha_index].edges().size(), "ha_index=" << ha_index << ", edge_index=" << edge_index << 
                                                                    ", |edges|=" << cnc_ha_seq[ha_index].edges().size())
      const auto& edge = cnc_ha_seq[ha_index].edge(edge_index);
      res[ha_index] = edge.dst(); } 
    return res; }

  auto cnc_dest_of(const cnc_par_loc_id_t& src, const cnc_par_edge_idx_t& par_idx) { return cnc_dest_of(src, par_idx, cnc_ha_seq); }

  static auto abs_dest_of(const abs_par_loc_id_t& src, const abs_par_edge_idx_t& par_idx, const abs_ha_seq_t& abs_ha_seq) {
    auto res = src;
    for(const auto& id : par_idx) {
      const auto  ha_index   = std::get<0>(id);
      const auto  edge_index = std::get<1>(id);
      asserts_msg(ha_index < abs_ha_seq.size(), "ha_index=" << ha_index << ", |abs_ha_seq|=" << abs_ha_seq.size())
      asserts_msg(edge_index < abs_ha_seq[ha_index].edges().size(), "ha_index=" << ha_index << ", edge_index=" << edge_index << 
                                                                    ", |edges|=" << abs_ha_seq[ha_index].edges().size())
      const auto& edge = abs_ha_seq[ha_index].edge(edge_index);
      res[ha_index] = edge.dst(); } 
    return res; }

  auto abs_dest_of(const abs_par_loc_id_t& src, const abs_par_edge_idx_t& par_idx) { return abs_dest_of(src, par_idx, abs_ha_seq); }

  bool is_transient(const cnc_par_loc_id_t& par_id) {
    const auto size = cnc_ha_seq.size();
    for(ha_index_t i = 0; i < size; i++) 
      if(cnc_ha_seq[i].loc(par_id[i]).is_trans())
        return true;
    return false; }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  /*
  const P& inv_of(const cnc_par_loc_id_t& par_id) {
    auto iter = invs.find(par_id);
    if(iter != invs.end()) return iter->second;
    P res{};
    res.add_space_dimensions_and_embed(dim);
    for(ha_index_t i = 0; i < size; i++) {
      asserts_msg(par_id[i] < cnc_ha_seq[i].locs().size(), "i: " << i << ", |locs|: " << cnc_ha_seq[i].locs().size() << ", par_id[i]: " << par_id[i])
      res.intersection_assign(cnc_ha_seq[i].loc(par_id[i]).inv()); }
    remove_time_var(res);
    minimize_poly(res);
    return std::get<0>(invs.insert(std::make_pair(par_id, std::move(res))))->second; }
  */

  const P& abs_inv_of(const abs_par_loc_id_t& par_id) {
    auto iter = invs.find(par_id);
    if(iter != invs.end()) return iter->second;
    P res{};
    res.add_space_dimensions_and_embed(dim);
    for(ha_index_t i = 0; i < size; i++) {
      asserts_msg(par_id[i] < abs_ha_seq[i].locs().size(), "i: " << i << ", |locs|: " << abs_ha_seq[i].locs().size() << ", par_id[i]: " << par_id[i])
      res.intersection_assign(abs_ha_seq[i].loc(par_id[i]).inv()); }
    remove_time_var(res);
    minimize_poly(res);
    return std::get<0>(invs.insert(std::make_pair(par_id, std::move(res))))->second; }

  const cnc_flow_map_t& cnc_flow_of(const cnc_par_loc_id_t& par_id) {
    auto iter = flows.find(par_id);
    if(iter != flows.end()) return iter->second;
    cnc_flow_map_t res;
    res.reserve(dim);
    if(is_transient( par_id)) {
      for(const auto& v : vars)
        res.insert(std::make_pair(v, ZERO));
    } else {
      for(ha_index_t i = 0; i < size; i++) 
        for(const auto& vf : cnc_ha_seq[i].loc(par_id[i]).flow()) {
          const auto& v = vf.first ;
          const auto& f = vf.second;
          const auto iter = res.find(v);
          requires_msg(iter==res.end() || iter->second == f, 
                       "flow of variable " << v << " is defined in more than one automata once by \"" << f << "\", and once by \"" << iter->second << "\"")
          if(iter == res.end())
            res.insert(vf); }
      requires_msg(dim == res.size(), "number of variables with defined flow (" << res.size() << ") is different from dimension (" << dim << ")");  }
    if(time_var)
      res.erase(*time_var);
    return std::get<0>(flows.insert(std::make_pair(par_id, std::move(res))))->second; }

  const P& rel_of(const cnc_par_edge_idx_t& par_idx) {
    auto iter = rels.find(par_idx);
    if(iter != rels.end()) return iter->second;
    P res{};
    res.add_space_dimensions_and_embed(dim << 1);
    for(const auto& pair : par_idx) {
      const auto ha_index   = std::get<0>(pair);
      const auto edge_index = std::get<1>(pair);
      res.intersection_assign(cnc_ha_seq[ha_index].edge(edge_index).rel()); }
    remove_time_var(res);
    minimize_poly(res);
    return std::get<0>(rels.insert(std::make_pair(par_idx, std::move(res))))->second; }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  /*
  auto bound_var(const term_ptr& var, std::uint64_t len) {
    requires(len > 0)
    auto par_id = conc_par_loc_id(ce.init_par_loc);
    auto res    = ppl_utils::bound_of_var(var, inv_of(par_id));
    for(const auto& abs_par_idx : ce.path) {
      if(len-- == 0) break;
      const auto cnc_par_idx = conc_par_edge_idx(abs_par_idx);
      if(!cnc_par_idx) continue;
      par_id = cnc_dest_of(par_id, *cnc_par_idx);
      res = res.chull(ppl_utils::bound_of_var(var, inv_of(par_id))); }
    return res; }
  */

  /** I believe in validiating a counter example, I can always use reached sets as invariants. This will always be a subset of actual invariant and of course 
    * it is possible to be a much smaller set. */
  auto bound_var(const term_ptr& var, std::uint64_t len) {
    requires(len > 0)
    auto it = states_seq.cbegin();
    auto res = interval_t::empty();
    do {
      asserts(it != states_seq.cend()) res = res.chull(ppl_utils::bound_of_var(var, * it++));
      asserts(it != states_seq.cend()) res = res.chull(ppl_utils::bound_of_var(var, * it++));
    } while(--len > 0);
    return res; }

  auto bound_vars(const std::uint64_t len) {
    std::unordered_map<term_ptr, interval_t> res;
    res.reserve(vars.size());
    for(const auto& var : vars)
      res.insert(std::make_pair(var, bound_var(var, len)));
    return res; }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------
  
  static auto create_identity_rel(const ppl::dimension_type dim, const boost::optional<term_ptr>& time_var) {
    ppl::Constraint_System id_rel_css{};
    id_rel_css.set_space_dimension(dim << 1);
    const auto id = time_var ? (*time_var)->variable().id() : dim;
    for(ppl::dimension_type i = 0; i < dim; i++) {
      if(i != id)
        id_rel_css.insert(ppl::Variable(i) - ppl::Variable(i+dim) == 0); }
    return P(id_rel_css); }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  void remove_time_var(P& poly) {
    if(!time_var) return;
    const auto poly_dim = poly.space_dimension();
    const auto id = (*time_var)->variable().id();
    poly.unconstrain(ppl::Variable(id));
    if(poly_dim > dim)
      poly.unconstrain(ppl::Variable(id + dim)); }

  void minimize_poly(P& poly) { poly = P(poly.minimized_constraints()); }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  template<typename Out>
  Out& convert_inv(Out& out, const P& inv) {
    if(inv.is_universe()) out << indent << "true;" << std::endl;
    else for(const auto& cs : inv.constraints())
            ppl_utils::print_constraint(out << indent << "(", cs, i2v, "=", "*") << ");" << std::endl;
    return out; }

  template<typename Out>
  Out& convert_flow(Out& out, const cnc_flow_map_t& flow) {
    for(const auto& vf : flow) {
      const auto& v = std::get<0>(vf);
      out << indent << "d/dt[" << v << "] = " << std::get<1>(vf) << ";" << std::endl; }
    return out; }

  template<typename Out>
  Out& convert_poly(Out& out, const P& poly) {
    if(poly.is_universe()) out << "true" << std::endl;
    else {
      out << "(and ";
      for(const auto& cs : poly.constraints()) 
        ppl_utils::print_constraint(out << " (", cs, i2v, "=", "*") << ")";
      out << ")"; } 
    return out; }

  template<typename Out>
  Out& convert_jump(Out& out, const mode_t next_id, const P& rel, const bool has_jump) {
    if(has_jump) {
      out << indent << "true ==> @" << next_id << " ";
      convert_poly(out, rel) << ";" << std::endl;  }
    return out; }

  static constexpr const char* indent = "        ";
  template<typename Out>
  Out& convert_loc(Out& out, const mode_t mode, P inv, const cnc_flow_map_t flow, P rel, const bool has_jump = true) {
    out << "{ mode " << mode << ";" << std::endl;
    convert_inv (out << "  invt:" << std::endl, inv );
    convert_flow(out << "  flow:" << std::endl, flow);
    convert_jump(out << "  jump:" << std::endl, mode+1, rel, has_jump);
    out << "}" << std::endl;  
    return out; }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  cnc_flow_map_t constant_flow() {
    cnc_flow_map_t res;
    for(const auto& iv : i2v) {
      const auto& v = std::get<1>(iv);
      if(v == time_var) continue;
      res.insert(std::make_pair(v, ZERO)); }
    return res; }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  std::string convert_to_dreach_global_bounds(const std::uint64_t len) {
    std::stringstream buff;
    // compute and write variable bounds
    const auto bounds = bound_vars(len);
    for(const auto& bound : bounds) {
      const auto& v = std::get<0>(bound);
      const auto& b = std::get<1>(bound);
      if(time_var && *time_var == v) continue;
      buff << "[" << b.inf() << ", " << b.sup() << "] " << v << ";" << std::endl; }
    buff << "[0, " << single_cont_trans_time_bound << "] time;" << std::endl << std::endl;
    return buff.str();  }

  std::string convert_to_dreach_body() {
    std::stringstream buff;

    // converting location (include edges)
    mode_t mode = 1;

          auto s_iter = states_seq.begin() + 1;
          auto p_iter = ce.path.begin   ();
    const auto p_end  = ce.path.end     ();

    auto abs_par_loc = ce.init_par_loc;
    auto cnc_par_loc = conc_par_loc_id(abs_par_loc);
    while(p_iter != p_end) {
      const auto& inv      = abs_inv_of (abs_par_loc);
      const auto& flow     = cnc_flow_of(cnc_par_loc);
            auto  guard    = * s_iter++;
            auto  next     = * s_iter++;
      const auto& abs_edge = * p_iter++;
      const auto  par_edge = conc_par_edge_idx(abs_edge);
      const auto& rel      = par_edge ? rel_of(*par_edge) : id_rel;
      abs_par_loc = abs_dest_of(abs_par_loc, abs_edge);
      cnc_par_loc = conc_par_loc_id(abs_par_loc);
      guard.add_space_dimensions_and_embed(dim);
      next .add_space_dimensions_and_embed(dim);
      next .map_space_dimensions(ppl_utils::shift_dimensions(dim<<1));
      next .intersection_assign(rel);
      next .intersection_assign(guard);
      remove_time_var(next);
      minimize_poly(next);
      convert_loc(buff, mode++, inv, flow, next);  }
    convert_loc(buff, mode++, abs_inv_of(abs_par_loc), cnc_flow_of(cnc_par_loc), id_rel);

    P unsafe(states_seq.back());
    remove_time_var(unsafe);
    minimize_poly(unsafe);
    convert_loc(buff, mode++, unsafe, constant_flow(), P{}, false);

    buff << "init:" << std::endl;
    P init(states_seq.front());
    remove_time_var(init);
    minimize_poly(init);
    convert_poly(buff << "@1    ", init) << " ;" << std::endl << std::endl;

    buff << "goal:" << std::endl;
    for(mode_t m = 2; m < mode; m++)
      buff << "@" << m << "    true ;" << std::endl; 

    return buff.str();  }

  template<typename Out>
  Out& convert_to_dreach(Out& out, const std::string& body, const std::uint64_t len) {
    out << convert_to_dreach_global_bounds(len);
    out << body;
    return out;
  }

  /*
  template<typename Out>
  Out& convert_to_dreach(Out& out, const std::uint64_t len) {
    // compute and write variable bounds
    const auto bounds = bound_vars(len);
    for(const auto& bound : bounds) {
      const auto& v = std::get<0>(bound);
      const auto& b = std::get<1>(bound);
      if(time_var && *time_var == v) continue;
      out << "[" << b.inf() << ", " << b.sup() << "] " << v << ";" << std::endl; }
    out << "[0, " << single_cont_trans_time_bound << "] time;" << std::endl << std::endl;

    // converting location (include edges)
    mode_t mode = 1;

          auto s_iter = states_seq.begin() + 1;
          auto p_iter = ce.path.begin   ();
    const auto p_end  = ce.path.end     ();

    auto par_loc = conc_par_loc_id(ce.init_par_loc);
    while(p_iter != p_end) {
      const auto& inv      = inv_of (par_loc);
            auto  inv      = * s_iter;
      const auto& flow     = cnc_flow_of(par_loc);
            auto  guard    = * s_iter++;
            auto  next     = * s_iter++;
      const auto& abs_edge = * p_iter++;
                  inv.poly_hull_assign(guard);
      const auto  par_edge = conc_par_edge_idx(abs_edge);
                  par_loc  = par_edge ? cnc_dest_of(par_loc, *par_edge) : par_loc;
      const auto& rel      = par_edge ? rel_of(*par_edge) : id_rel;
      guard.add_space_dimensions_and_embed(dim);
      next .add_space_dimensions_and_embed(dim);
      next .map_space_dimensions(ppl_utils::shift_dimensions(dim<<1));
      next .intersection_assign(rel);
      next .intersection_assign(guard);
      remove_time_var(next);
      minimize_poly(next);
      convert_loc(out, mode++, inv, flow, next);  }
    auto inv = inv_of(par_loc);;
    convert_loc(out, mode++, inv, cnc_flow_of(par_loc), id_rel);

    P unsafe(states_seq.back());
    remove_time_var(unsafe);
    minimize_poly(unsafe);
    convert_loc(out, mode++, unsafe, constant_flow(), P{}, false);

    out << "init:" << std::endl;
    P init(states_seq.front());
    remove_time_var(init);
    minimize_poly(init);
    convert_poly(out << "@1    ", init) << " ;" << std::endl << std::endl;

    out << "goal:" << std::endl;
    for(mode_t m = 2; m < mode; m++)
      out << "@" << m << "    true ;" << std::endl; 

    return out;
  } 
  */

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  /** @brief create and return an empty directory. */
  static auto create_ws_folder() {
#if __linux__
    return std::string("./ws/");
#else
    static std::string res = "";
    if(res.empty()) {
      std::string   prefix = "./ws/n";
      std::uint64_t index  = 1;
      boost::filesystem::path path;
      while(boost::filesystem::exists(path = boost::filesystem::path(res = prefix + std::to_string(index++)))) {
        log_trace << "already exists folder " << res;
      }
      const auto succeed = boost::filesystem::create_directories(path);
      asserts_msg(succeed, "could not create the working space folder " << res)
      log_debug << "working space is created at " << res;  
      res += "/"; }
    return res; 
#endif // __linux__
  }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  static bool is_sat(const std::string& file) {
    std::ifstream in(file);
    requires_msg(in.is_open(), "cannot open file " << file << " for reading. It was supposed to be created by dReach!")
    std::string line;
    while(std::getline(in,line))
      if(line.find("SAT") != std::string::npos)
        return true;
    in.close();
    return false; }

  /** @return the longest index for which the input counterexample is valid. 
    * @note return value 0 means no edge could be taken.
    *       return value 1 means only the first edge could be taken.
    *       return value n for n at most length of the counter example means only the first n edges could be taken.
    *       returning the length of counterexample means all edges could have been taken, but the unsafe state till is not reachable.
    *       returning 1 + the length of counterexample means the counterexample is valid. */
  mode_t longest_valid_prefix() {
    const auto folder      = create_ws_folder();
    const auto model_file  = folder + "dreach.drh";
    const auto output_file = folder + "dreach.out";

    const auto body = convert_to_dreach_body();

    for(auto len = ce.path.size() + 1; len > 0; len--) {
      std::ofstream out(model_file);
      convert_to_dreach(out, body, len);
      out.close();
      std::stringstream buff;
      buff << dreach_path(cnc_safety.props());
      buff << " -l " << len << " -u " << len << " " << model_file << " > " << output_file;
      const auto cmd = buff.str();
      log_debug << cmd;
      const auto ret = ({ const auto timer = pc::start_mc_dreach_total();
                          std::system(cmd.c_str()); });
      log_debug << "dReach return code: " << ret;
      requires_msg(ret == 0, "dreach return code (" << ret << ") is not zero")
      if(is_sat(output_file)) return len; }
    return 0;
  }

private:
  const cnc_safety_t & cnc_safety;
  const cnc_ha_seq_t & cnc_ha_seq;
  const abs_safety_t & abs_safety;
  const abs_ha_seq_t & abs_ha_seq;
  const var_set      & vars;
  const id2var       & i2v ;
  const name2var     & n2v ;

  const abs_counter_example_t & ce;
  const abs_states_seq_t      & states_seq;
  const locs_backtrace_t      & locs_backtrace;
  const edges_backtrace_t     & edges_backtrace;

  const boost::optional<term_ptr>  time_var; // by value 
  const constant                  &single_cont_trans_time_bound;

  const ha_index_t          size;
  const ppl::dimension_type dim ;

  const P id_rel;

  mutable std::unordered_map<cnc_par_loc_id_t  , cnc_flow_map_t, cnc_par_loc_id_hash_t  > flows;
  mutable std::unordered_map<abs_par_loc_id_t  , P             , abs_par_loc_id_hash_t  > invs ;
  mutable std::unordered_map<cnc_par_edge_idx_t, P             , cnc_par_edge_idx_hash_t> rels ;


#define DREACH "dreach."
  static constexpr key_t      KEY_Path = DREACH "path";
#undef  DREACH
};

} // hare::dreal_utils

#endif // HA__UTILS_DRREACH__HPP

