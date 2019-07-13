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

#ifndef HA__POLY_HA_PARSER__HPP
#define HA__POLY_HA_PARSER__HPP

#include "ha/poly_ha.hpp"
#include "ha/term.hpp"
#include "ha/term_parser.hpp"
#include "ha/performance_counter.hpp"

#include "cmn/dbc.hpp"
#include "cmn/misc/utils.hpp"

#include <boost/iterator/filter_iterator.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>

#include <algorithm>
#include <cctype>       // isalnum, isalpha
#include <fstream>      // filebuf
#include <istream>      // istream
#include <ios>          // ios
#include <sstream>      // istringstream
#include <stdexcept>
#include <string>
#include <tuple>        // get
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ha::poly_ha::parser {
  
namespace pt = boost::property_tree;

struct parse_error : std::runtime_error { 
  explicit parse_error(const std::string& what_arg) : std::runtime_error(what_arg) { }
  explicit parse_error(const char*        what_arg) : std::runtime_error(what_arg) { }
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

const auto ha_pred = [](const auto& iter) { return iter.first == "hybrid-automaton"; };
using ha_pred_t    = std::remove_const_t<std::remove_reference_t<decltype(ha_pred  )>>;

template<typename Iter>
auto end(const Iter& filtered) { return Iter(filtered.predicate(), filtered.end(), filtered.end()); }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

inline term_ptr var_of(const name2var n2v, const std::string& name) {
  const auto iter = n2v.find(name);
  requires_msg(iter != n2v.end(), "undefined variable (" << name << ")");
  return iter->second; }

inline bool valid_variable_name(const std::string& name) {
  return 
    !name.empty ()        &&
    std::isalpha(name[0]) &&
    std::all_of(name.begin(), name.end(), [](const auto c) { return std::isalnum(c); } ); }

inline auto parse_vars(const std::string& vars, const std::string& sep = ",") {
  var_set  set;
  name2var n2v;
  var_id id = 0;
  cmn::misc::utils::apply_to_tokens(vars, sep, [&set, &n2v, &id](const auto& token) {
    const auto name = cmn::misc::utils::trim(token);
    requires_msg(id < std::numeric_limits<var_id>::max(), "too many variables ("         <<  id  << ")"   )
    requires_msg(valid_variable_name(name)              , "invalid variable name (<<<"   << name << ">>>)")
    requires_msg(n2v.find(name) == n2v.end()            , "duplicate variable name (<<<" << name << ">>>)")
    const auto v = var(id, name);
    set.insert(v);
    n2v.insert({name, v});
    id++;
  });
  return std::make_tuple(set, n2v); }

inline auto map_with_prime_vars(const name2var& n2v, const std::string prime = "'") {
  name2var res;
  const auto dim = n2v.size();
  for(const auto kv : n2v) {
    auto pv = var(kv.second->variable().id() + dim, kv.first + prime); 
    res.insert(kv);
    res.insert({kv.first + prime, std::move(pv) } ); }
  return res; } 

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

inline void apply_params(const std::vector<std::tuple<std::string, std::string>> kws, const std::string key, pt::ptree& tr, const bool ordered) {
  static const auto skip_set = std::unordered_set<std::string>({"hybrid-automaton", "loc", "edge", "inits", "unsafes", "src", "dst"});
  for(auto& deep : tr)
    apply_params(kws, deep.first, deep.second, ordered);
  if(skip_set.find(key) == skip_set.end()) {
    auto val = tr.get_optional<std::string>("");
    if(!val) return;
const auto old = *val;
    if(ordered) 
      for(const auto& kw : kws)
        val = cmn::misc::utils::replace_all(*val, std::get<0>(kw), std::get<1>(kw));
    else val = cmn::misc::utils::replace_all(*val, kws);
    tr.put("", std::move(*val));
  } }

inline void apply_params(const pt::ptree& params, pt::ptree& tr, const bool ordered = true) {
  std::unordered_set<std::string> keys;
  std::vector<std::tuple<std::string, std::string>> kws;
  for(auto& param : params) {
    const auto& key = param.first;
    const auto  val = param.second.get<std::string>("", "");
    requires_msg(!ordered || keys.find(key) == keys.end(), "duplicate parameter (" << key << ")");
    keys.insert(key);
    kws.emplace_back(key, val); }
  if(!ordered)
    std::sort(kws.begin(), kws.end(), [](const auto& fst, const auto& snd) { return std::get<0>(fst) >  std::get<0>(snd); } );
  for(auto& ch : tr)
    if(ch.first != "params") 
      apply_params(kws, ch.first, ch.second, ordered);  }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** @brief very simple check to see if keys do not have typoes in them */
inline void check_node_names(const pt::ptree& tr) {
  static auto keys = std::unordered_set<std::string>({"params", "vars", "hybrid-automaton", "loc", "edge", "name", "inits", 
                                                      "unsafes", "inv", "flow", "src", "dst", "rel", "lbl", "props"});
  static auto stop_keys = std::unordered_set<std::string>({"params", "inits", "unsafes", "props", "flow"});
  for(const auto& ch : tr) {
    requires_msg(keys.find(ch.first) != keys.end(), "invalid key (<<<" << ch.first << ">>>>)")
    if(stop_keys.find(ch.first) == stop_keys.end())
      check_node_names(ch.second); } }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

inline auto parse_props(const pt::ptree& tr) {
  const auto child = tr.get_child_optional("props");
  return child ? *child : pt::ptree(); }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename P>
const auto poly_parser = [](const boost::optional<const pt::ptree&>& tr, const name2var& n2v) {
  auto text = tr ? tr->data() : "true"; 
  const auto term = ha::parser::parse_term(text.begin(), text.end(), n2v);
  return to_polyhedron<P>(term, n2v.size()); };

template<typename P> using poly_parser_t = std::remove_const_t<std::remove_reference_t<decltype(poly_parser<P>)>>;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename HAseq, typename P>
auto parse_par_loc_id(std::string par_id, const HAseq& ha_seq) {
  const auto len       = par_id.length();
  const auto single_ha = ha_seq.size() == 1;
  requires_msg(len > 0, "par_id cannot be empty")
  requires_msg(single_ha || par_id[0    ] == '[', "invalid par_id: " << par_id)
  requires_msg(single_ha || par_id[len-1] == ']', "invalid par_id: " << par_id)
  std::vector<par_loc_id> results({par_loc_id()}); // initially it has one empty par-id (empty vector)

  const auto add_to_res = [&results](cmn::pbv_t<loc_id> id) { for(auto& res : results) res.push_back(id); };
  const auto add_to_res_all_possiblities = [&results, &ha_seq]() { 
    std::vector<par_loc_id> new_results;
    for(auto& res : results) {
      const auto size = ha_seq[res.size()].locs().size();
      for(loc_id id = 0; id < size; id++) {
        auto cpy = res;
        cpy.push_back(id);
        new_results.push_back(std::move(cpy)); } }
    std::swap(results, new_results); };

  auto cpy = par_id;
  if(cpy[  0  ] == '[') cpy[  0  ] = ' ';
  if(cpy[len-1] == ']') cpy[len-1] = ' ';
  cmn::misc::utils::apply_to_tokens(cpy, std::string(","), [&add_to_res, &add_to_res_all_possiblities](const auto& str) { 
    const auto str_id = cmn::misc::utils::trim(str);
    if(str_id != "_") {
      loc_id id; 
      const bool succeed = boost::conversion::try_lexical_convert(str_id, id);
      requires_msg(succeed, "could not convert <<<" << str_id << ">>> to loc_id")
      if(!succeed) throw cmn::requires_error("could not convert str_id to loc_id"); 
      add_to_res(id);
    } else add_to_res_all_possiblities();
  });
  return results; }

template<typename HAseq, typename P>
const auto poly_map_parser = [](const boost::optional<pt::ptree&>& tr, const name2var& n2v, const HAseq& ha_seq) {
  poly_map<P> res;
  if(!tr) return res;
  for(const auto& q : *tr) {
    const auto poly = poly_parser<P>(q.second, n2v); 
    for(auto& par_id : parse_par_loc_id<HAseq, P>(q.first, ha_seq)) 
      res.emplace_back(std::move(par_id), poly); }
  return res; };

template<typename HAseq, typename P> using poly_map_parser_t = std::remove_const_t<std::remove_reference_t<decltype(poly_map_parser<HAseq, P>)>>;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename Edge, typename RelP>
auto parse_edge(const pt::ptree& tr, const name2var& primed_n2v, const RelP rel_parser) {
  const auto ends_width = [](const std::string& str, const std::string& w) { 
    const auto pos = str.rfind(w); 
    return pos != std::string::npos && 
           pos == str.length() - w.length(); };

  const auto rel   = tr.get("rel", "true");
  const auto lbl   = cmn::misc::utils::trim(tr.get<std::string>("", ""));
  const auto label = ends_width(lbl, "!!") ? std::make_tuple(lbl.substr(0, lbl.length() - 2), label_kind::BROADCAST) :
                     ends_width(lbl, "!" ) ? std::make_tuple(lbl.substr(0, lbl.length() - 1), label_kind::OUT      ) :
                     ends_width(lbl, "?" ) ? std::make_tuple(lbl.substr(0, lbl.length() - 1), label_kind::IN       ) :
                                             std::make_tuple(lbl                            , label_kind::SYNC     ) ;
  return Edge(
           tr.get<loc_id>("src"),
           tr.get<loc_id>("dst"),
           std::move(std::get<0>(label)), 
           std::move(std::get<1>(label)), 
           rel_parser(tr.get_child_optional("rel"), primed_n2v) ); }

template<typename Loc, typename InvP, typename FlowP>
auto parse_loc(const pt::ptree& tr, const name2var& n2v, const InvP& inv_parser, const FlowP& flow_parser) {
  std::string name = tr.get("name", ""); // name is optional
  const bool transient = name.length() > 0 || name[0] == '*';
  if(transient) name = name.substr(1);
  return Loc(
           tr.get<loc_id>(""),
           name,
           inv_parser (tr.get_child_optional("inv" ), n2v),
           flow_parser(tr.get_child_optional("flow"), n2v),
           transient); }

template<typename Loc, typename Edge, typename InvP, typename FlowP, typename RelP>
auto parse_locs_and_edges(const pt::ptree& tr        , const name2var& n2v        , const name2var& primed_n2v, 
                          const InvP     & inv_parser, const FlowP   & flow_parser, const RelP    & rel_parser) {
  std::vector<Loc > locs ;
  std::vector<Edge> edges;
  for(const auto ch : tr) 
         if(ch.first == "loc" ) locs .push_back(parse_loc <Loc , InvP, FlowP>(ch.second, n2v       , inv_parser, flow_parser));
    else if(ch.first == "edge") edges.push_back(parse_edge<Edge, RelP       >(ch.second, primed_n2v, rel_parser             ));
  return std::make_tuple(std::move(locs), std::move(edges)); }

template<typename Str, typename HA, typename Loc, typename Edge, typename InvP, typename FlowP, typename RelP>
auto parse_ha(      Str && name      , const pt::ptree& tr         , const var_set& vars      , const name2var& n2v, const name2var& primed_n2v,
              const InvP&  inv_parser, const FlowP    & flow_parser, const RelP   & rel_parser) {
  auto locs_edges = parse_locs_and_edges<Loc, Edge, InvP, FlowP, RelP>(tr, n2v, primed_n2v, inv_parser, flow_parser, rel_parser);
  return HA(std::move(name), vars, std::move(std::get<0>(locs_edges)), std::move(std::get<1>(locs_edges))); }

template<typename Iter, typename HA, typename Loc, typename Edge, typename InvP, typename FlowP, typename RelP>
auto parse_ha_seq(      Iter  iter      , const var_set& vars       , const name2var& n2v       , const name2var& primed_n2v,
                  const InvP& inv_parser, const FlowP  & flow_parser, const RelP    & rel_parser) {
  using ha_seq_t = std::vector<HA>;
  ha_seq_t res;
  const auto last = end(iter);
  while(iter != last) {
    const auto ch = * iter++;
    auto name = ch.second.get("", std::string()); // name is optioanl
    auto ha   = parse_ha<std::string&&,HA, Loc, Edge, InvP, FlowP, RelP>(std::move(name), ch.second, vars, n2v, primed_n2v, inv_parser, flow_parser, rel_parser);
    res.push_back(std::move(ha)); }
  return res; }

template<typename Safety, typename HA, typename Loc, typename Edge, typename InvP, typename FlowP, typename RelP, typename InitsP, typename UnsafesP>
Safety parse_safety( pt::ptree& tr, 
                     const InvP  & inv_parser   , const FlowP   & flow_parser   , const RelP& rel_parser, 
                     const InitsP& inits_parser , const UnsafesP& unsafes_parser) {
  check_node_names(tr);                                         // a simple check tor typos in keywords
  const auto params = tr.get_child_optional("params");          // if any parameter is defined, apply it right at the beginning
  if(params)
    apply_params(*params, tr);
  const auto vars = parse_vars(tr.get<std::string>("vars"));    // parse variables
  const auto vset = std::get<0>(vars);                          // set of declared variables
  const auto n2v  = std::get<1>(vars);                          // mapping from a name of variable to the variable itself
  using iter_t = boost::filter_iterator<ha_pred_t,pt::ptree::const_iterator>;
  const auto ha_iter = iter_t(ha_pred, tr.begin(), tr.end());
  auto ha_seq  = parse_ha_seq<iter_t, HA, Loc, Edge, InvP, FlowP, RelP>( ha_iter   , vset       , n2v       , map_with_prime_vars(n2v), 
                                                                         inv_parser, flow_parser, rel_parser                          );
  auto inits   = inits_parser  (tr.get_child_optional("inits"  ), n2v, ha_seq);
  auto unsafes = unsafes_parser(tr.get_child_optional("unsafes"), n2v, ha_seq);
  auto props   = parse_props(tr);
  return Safety(std::move(ha_seq), std::move(inits), std::move(unsafes), std::move(props)); }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename P>
auto parse_poly_safety(pt::ptree& tr) {
  using ha_seq_t = std::vector<poly_ha<P>>;
  return parse_safety<poly_safety  <P>, 
                      poly_ha      <P>, 
                      poly_loc     <P>, 
                      poly_edge    <P>, 
                      poly_parser_t<P>, 
                      poly_parser_t<P>, 
                      poly_parser_t<P>, 
                      poly_map_parser_t<ha_seq_t, P>, 
                      poly_map_parser_t<ha_seq_t, P>
                     > 
                     (tr, 
                      poly_parser<P>, 
                      poly_parser<P>, 
                      poly_parser<P>, 
                      poly_map_parser<ha_seq_t, P>, 
                      poly_map_parser<ha_seq_t, P>);  }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename P>
//poly_safety<P> parse_poly_safety_from_string(const std::string& str) {
auto parse_poly_safety_from_string(const std::string& str) {
  auto timer = pc::start_safety_parser();
  pt::ptree tree;
  std::istringstream buff(str);
  pt::info_parser::read_info(buff, tree);
  return parse_poly_safety<P>(tree); }

template<typename P>
poly_safety<P> parse_poly_safety_from_file(const std::string& file_path) {
  auto timer = pc::start_safety_parser();
  pt::ptree tree;
  std::filebuf fb;
  if (fb.open(file_path, std::ios::in)) {
    std::istream is(&fb);
    pt::info_parser::read_info(is, tree);
    fb.close();
  } else throw parse_error(strerror(errno)); 
  return parse_poly_safety<P>(tree); }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

} // ha::poly_ha::parser

#endif // HA__POLY_HA_PARSER__HPP
