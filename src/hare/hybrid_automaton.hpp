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

#ifndef HA__HYBRID_AUTOMATON__HPP
#define HA__HYBRID_AUTOMATON__HPP

#include "hare/term.hpp"
#include "cmn/type_traits.hpp"
#include "cmn/misc/utils.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>

#include <cstdint>
#include <string>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

namespace hare {

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** @tparam ID    identifier type
  * @tparam Inv   invariant type
  * @tparam Flow  flow type
  * @tparam InvP  invariant printer type
  * @tparam FlowP flow printer type 
  * @tparam TInv  trivial invariant predicate type (only used not to print trivial invariant) 
  * @tparam TFlow trivial flow predicate type      (only used not to print trivial flow) */
template<typename ID, typename Inv, typename Flow, typename InvP, typename FlowP, typename TInv, typename TFlow>
struct loc { 
  static_assert(std::is_integral<ID>::value);
  static_assert(std::is_unsigned<ID>::value);
  using str_t = std::string;

  using type                = loc  ;
  using id_t                = ID   ;
  using inv_t               = Inv  ;
  using flow_t              = Flow ;
  using inv_printer_t       = InvP ;
  using flow_printer_t      = FlowP;
  using trivial_inv_pred_t  = TInv ;
  using trivial_flow_pred_t = TFlow;
  
  explicit loc(const id_t id, const bool trans = false) : id_(id), trans_(trans) { }

  template<typename SS>
  explicit loc(const id_t id, SS&& name, const bool trans = false) : 
    id_(id), name_(std::forward<SS>(name)), trans_(trans) { }

  template<typename II, typename FF>
  explicit loc(const id_t id, II&& inv, FF&& flow, const bool trans = false) : 
    id_(id), inv_(std::forward<II>(inv)), flow_(std::forward<FF>(flow)), trans_(trans) { }

  template<typename SS, typename II, typename FF>
  explicit loc(const id_t id, SS&& name, II&& inv, FF&& flow, const bool trans = false) : 
    id_(id), name_(std::forward<SS>(name)), inv_(std::forward<II>(inv)), flow_(std::forward<FF>(flow)), trans_(trans) { }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------


  id_t  & id      () noexcept { return id_   ; }
  str_t & name    () noexcept { return name_ ; }
  inv_t & inv     () noexcept { return inv_  ; }
  flow_t& flow    () noexcept { return flow_ ; }
  bool  & is_trans() noexcept { return trans_; }

  cmn::pbv_t<id_t  > id      () const noexcept { return id_   ; }
  cmn::pbv_t<str_t > name    () const noexcept { return name_ ; }
  cmn::pbv_t<inv_t > inv     () const noexcept { return inv_  ; }
  cmn::pbv_t<flow_t> flow    () const noexcept { return flow_ ; }
  bool               is_trans() const noexcept { return trans_; }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  /** @return \c true iff some properties of \c this and \c that locs are different */
  bool nq(const type& that) const { return !eq(that); }

  /** @return \c true iff all properties of \c this and \c that locs are the same */
  bool eq(const type& that) const { return this == &that || (id_    == that.id_   && 
                                                             name_  == that.name_ && 
                                                             inv_   == that.inv_  && 
                                                             flow_  == that.flow_ && 
                                                             trans_ == that.trans_ ); }

  /** @return \c true iff identifiers of \c this and \c that locs are different */
  bool operator!=(const type& that) const { return !operator==(that); }

  /** @return \c true iff identifiers of \c this and \c that locs are the same */
  bool operator==(const type& that) const { return this == &that || id_ == that.id_; }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  template<typename Out>
  Out& print(Out& out, const std::uint64_t indent, const id2var& i2v, const name2var& n2v) const { 
    namespace utils = cmn::misc::utils;
    utils::indent(out, indent);
    out << "loc " << id_;
    if(trans_) out << "*";
    out << " {";
    if(!name_.empty())                    utils::indent(out << std::endl, indent + 4) << "name " << "\"" << name_ << "\"";
    if(!trivial_inv (inv_ )) inv_printer (utils::indent(out << std::endl, indent + 4) << "inv  ", inv_ , indent + 8, i2v, n2v);
    if(!trivial_flow(flow_)) flow_printer(utils::indent(out << std::endl, indent + 4) << "flow ", flow_, indent + 8, i2v, n2v);
    out << " }" << std::endl;
    return out;  }

  template<typename Out>
  Out& print(Out& out) const { return print(out, 0, id2var(), name2var()); } 

private: 
  id_t   id_   ; //< the identifier of the loc (equality and hash values only use this property)
  str_t  name_ ; 
  inv_t  inv_  ;
  flow_t flow_ ;
  bool   trans_; //< transient (transient locs does not have positive time transitions)

  static const InvP  inv_printer ;
  static const FlowP flow_printer;
  static const TInv  trivial_inv ;
  static const TFlow trivial_flow;
};

template<typename ID, typename Inv, typename Flow, typename InvP, typename FlowP, typename TInv, typename TFlow>
const InvP loc<ID, Inv, Flow, InvP, FlowP, TInv, TFlow>::inv_printer { };

template<typename ID, typename Inv, typename Flow, typename InvP, typename FlowP, typename TInv, typename TFlow>
const FlowP loc<ID, Inv, Flow, InvP, FlowP, TInv, TFlow>::flow_printer { };

template<typename ID, typename Inv, typename Flow, typename InvP, typename FlowP, typename TInv, typename TFlow>
const TInv loc<ID, Inv, Flow, InvP, FlowP, TInv, TFlow>::trivial_inv { };

template<typename ID, typename Inv, typename Flow, typename InvP, typename FlowP, typename TInv, typename TFlow>
const TFlow loc<ID, Inv, Flow, InvP, FlowP, TInv, TFlow>::trivial_flow { };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

enum class label_kind : std::uint8_t { IN, OUT, BROADCAST, SYNC };

template<typename Out>
inline Out& operator<<(Out& os, const label_kind type) {
  switch(type) {
  case label_kind::IN        : { os << "?"   ; return os; }
  case label_kind::OUT       : { os << "!"   ; return os; }
  case label_kind::BROADCAST : { os << "!!"  ; return os; }
  case label_kind::SYNC      : { os << "sync"; return os; }  }  }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** @tparam ID   identifier type
  * @tparam Lbl  label type
  * @tparam Rel  relation type
  * @tparam RelP relation printer type
  * @tparam TRel trivial relation predicate type */
template<typename ID, typename Lbl, typename Rel, typename RelP, typename TRel>
struct edge {

  using type               = edge;
  using id_t               = ID  ;
  using lbl_t              = Lbl ;
  using knd_t              = label_kind;
  using rel_t              = Rel ;
  using rel_printer_t      = RelP;
  using trivial_rel_pred_t = TRel;

  template<typename L, std::enable_if_t< std::is_fundamental<L>::value>* = nullptr>
  explicit edge(const id_t src, const id_t dst, const L   lbl, const knd_t knd) : src_(src), dst_(dst), lbl_(lbl), knd_(knd) { }

  template<typename L, std::enable_if_t<!std::is_fundamental<L>::value>* = nullptr>
  explicit edge(const id_t src, const id_t dst,       L&& lbl, const knd_t knd) : src_(src), dst_(dst), lbl_(std::forward<L>(lbl)), knd_(knd) { }

  template<typename L, typename R, std::enable_if_t< std::is_fundamental<L>::value>* = nullptr>
  explicit edge(const id_t src, const id_t dst, const L   lbl, const knd_t knd, R&& rel) : 
    src_(src), dst_(dst), lbl_(lbl), knd_(knd), rel_(std::forward<R>(rel)) { }

  template<typename L, typename R, std::enable_if_t<!std::is_fundamental<L>::value>* = nullptr>
  explicit edge(const id_t src, const id_t dst,       L&& lbl, const knd_t knd, R&& rel) : 
    src_(src), dst_(dst), lbl_(std::forward<L>(lbl)), knd_(knd), rel_(std::forward<R>(rel)){ }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  id_t & src() noexcept { return src_; }
  id_t & dst() noexcept { return dst_; }
  lbl_t& lbl() noexcept { return lbl_; }
  knd_t& knd() noexcept { return knd_; }
  rel_t& rel() noexcept { return rel_; }

  cmn::pbv_t<id_t > src() const noexcept { return src_; }
  cmn::pbv_t<id_t > dst() const noexcept { return dst_; }
  cmn::pbv_t<lbl_t> lbl() const noexcept { return lbl_; }
  cmn::pbv_t<knd_t> knd() const noexcept { return knd_; }
  cmn::pbv_t<rel_t> rel() const noexcept { return rel_; }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  bool operator!=(const type& that) const { return !operator==(that); }
  bool operator==(const type& that) const { return this == &that || (src_ == that.src_ && 
                                                                     dst_ == that.dst_ && 
                                                                     lbl_ == that.lbl_ && 
                                                                     knd_ == that.knd_ && 
                                                                     rel_ == that.rel_ ); }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  template<typename Out>
  Out& print(Out& out, const std::uint64_t indent, const id2var& i2v, const name2var& n2v) const { 
    namespace utils = cmn::misc::utils;
    utils::indent(out, indent);
    out << "edge ";
    switch(knd_) {
      case label_kind::IN        : out << "\"" << lbl_ << "?"  << "\" "; break;
      case label_kind::OUT       : out << "\"" << lbl_ << "!"  << "\" "; break;
      case label_kind::BROADCAST : out << "\"" << lbl_ << "!!" << "\" "; break;
      case label_kind::SYNC      : if(!lbl_.empty()) 
                                     out << "\"" << lbl_ << "\" "; 
                                   break; };
    out << "{";
    utils::indent(out << std::endl, indent + 4) << "src " << src_;
    utils::indent(out << std::endl, indent + 4) << "dst " << dst_;
    if(!trivial_rel(rel_)) 
      rel_printer(out, rel_ , indent + 4, i2v, n2v);
    out << " }" << std::endl;
    return out;  }

  template<typename Out>
  Out& print(Out& out) const { return print(out, 0, id2var(), name2var()); } 

private: 
  id_t  src_;
  id_t  dst_;
  lbl_t lbl_;
  knd_t knd_;
  rel_t rel_;

  static const RelP rel_printer;
  static const TRel trivial_rel;
};

template<typename ID, typename Lbl, typename Rel, typename RelP, typename TRel> const RelP edge<ID, Lbl, Rel, RelP, TRel>::rel_printer { };
template<typename ID, typename Lbl, typename Rel, typename RelP, typename TRel> const TRel edge<ID, Lbl, Rel, RelP, TRel>::trivial_rel { };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

struct non_trivial_condition_t {
  template<typename T>
  bool operator()(const T& /*cond*/) const { return false; }
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename Grd, typename Rst, typename GrdP, typename RstP, typename TGrd, typename TRst>
struct guard_reset_printer {
  using grd_t              = Grd ;
  using rst_t              = Rst ;
  using grd_printer_t      = GrdP;
  using rst_printer_t      = RstP;
  using trivial_grd_pred_t = TGrd;
  using trivial_rst_pred_t = TRst;

  template<typename Out>
  Out& operator()(Out& out, const std::tuple<grd_t, rst_t>& rel, const std::uint64_t indent, const id2var& i2v, const name2var& n2v) const {
    namespace utils = cmn::misc::utils;
    const auto& grd = std::get<0>(rel);
    const auto& rst = std::get<1>(rel);
    if(!trivial_grd(grd)) grd_printer(utils::indent(out << std::endl, indent) << "grd ", grd, indent + 8, i2v, n2v);
    if(!trivial_rst(rst)) rst_printer(utils::indent(out << std::endl, indent) << "rst ", rst, indent + 8, i2v, n2v);
    return out;  }

private:
  static const GrdP grd_printer;
  static const RstP rst_printer;
  static const TGrd trivial_grd;
  static const TRst trivial_rst;
};

template<typename Grd, typename Rst, typename GrdP, typename RstP, typename TGrd, typename TRst>
const GrdP guard_reset_printer<Grd, Rst, GrdP, RstP, TGrd, TRst>::grd_printer { };

template<typename Grd, typename Rst, typename GrdP, typename RstP, typename TGrd, typename TRst>
const RstP guard_reset_printer<Grd, Rst, GrdP, RstP, TGrd, TRst>::rst_printer { };

template<typename Grd, typename Rst, typename GrdP, typename RstP, typename TGrd, typename TRst>
const TGrd guard_reset_printer<Grd, Rst, GrdP, RstP, TGrd, TRst>::trivial_grd { };

template<typename Grd, typename Rst, typename GrdP, typename RstP, typename TGrd, typename TRst>
const TRst guard_reset_printer<Grd, Rst, GrdP, RstP, TGrd, TRst>::trivial_rst { };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** @tparam ID   identifier type
  * @tparam Lbl  label type
  * @tparam Grd  guard type
  * @tparam Rst  reset type
  * @tparam GrdP guard printer type
  * @tparam RstP reset printer type
  * @tparam TGrd trivial guard predicate type
  * @tparam TRst trivial reset predicate type */
template<typename ID, typename Lbl, typename Grd, typename Rst, typename GrdP, typename RstP, typename TGrd, typename TRst>
struct simple_edge : edge<ID, Lbl, std::tuple<Grd, Rst>, guard_reset_printer<Grd, Rst, GrdP, RstP, TGrd, TRst>, non_trivial_condition_t> {
  using parent =     edge<ID, Lbl, std::tuple<Grd, Rst>, guard_reset_printer<Grd, Rst, GrdP, RstP, TGrd, TRst>, non_trivial_condition_t>;

  using id_t               = ID  ;
  using grd_t              = Grd  ;
  using rst_t              = Rst  ;
  using grd_printer_t      = GrdP ;
  using rst_printer_t      = RstP ;
  using trivial_grd_pred_t = TGrd ;
  using trivial_rst_pred_t = TRst ;

  template<typename L, std::enable_if_t< std::is_fundamental<L>::value>* = nullptr>
  simple_edge(const id_t src, const id_t dst, const L   lbl, const label_kind knd) : parent(src, dst, lbl, knd) { }

  template<typename L, std::enable_if_t<!std::is_fundamental<L>::value>* = nullptr>
  simple_edge(const id_t src, const id_t dst,       L&& lbl, const label_kind knd) : parent(src, dst, std::forward<L>(lbl), knd) { }
 
  template<typename GG, typename RR, typename L, std::enable_if_t< std::is_fundamental<L>::value>* = nullptr>
  simple_edge(const id_t src, const id_t dst, const L   lbl, const label_kind knd, GG&& grd, RR&& rst) : 
    parent(src, dst, lbl, knd, std::tuple<Grd, Rst>(std::forward<GG>(grd), std::forward<RR>(rst))) { }
 
  template<typename GG, typename RR, typename L, std::enable_if_t<!std::is_fundamental<L>::value>* = nullptr>
  simple_edge(const id_t src, const id_t dst,       L&& lbl, const label_kind knd, GG&& grd, RR&& rst) : 
    parent(src, dst, std::forward<L>(lbl), knd, std::tuple<Grd, Rst>(std::forward<GG>(grd), std::forward<RR>(rst))) { }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  grd_t& guard() noexcept { return std::get<0>(this->rel()); }
  rst_t& reset() noexcept { return std::get<1>(this->rel()); }

  cmn::pbv_t<grd_t> guard() const noexcept { return std::get<0>(this->rel()); }
  cmn::pbv_t<rst_t> reset() const noexcept { return std::get<1>(this->rel()); }
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** @tparam Q location type
  * @tparam E edge type */
template<typename Q, typename E>
struct hybrid_automaton {

  using type               = hybrid_automaton;
  using loc_t              = Q;
  using edge_t             = E;
  using str_t              = typename loc_t::str_t;
  using loc_id_t           = typename loc_t::id_t ;
  using locs_t             = std::vector<loc_t >;    // must be random access
  using edges_t            = std::vector<edge_t>;    // must be random access

  using loc_index_t        = typename locs_t::size_type;
  using loc_indices_t      = std::vector<loc_index_t>  ;

  using edge_index_t       = typename edges_t::size_type;
  using edge_indices_t     = std::vector<edge_index_t>  ;
  using edge_indices_vec_t = std::vector<edge_indices_t>;

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  template<typename SS, typename VV, typename QQ, typename EE>
  hybrid_automaton(SS&& name, VV&& vars, QQ&& locs, EE&& edges) : 
    name_(std::forward<SS>(name)), vars_(std::forward<VV>(vars)), i2v_(to_i2v(vars_)), 
    n2v_(to_n2v(vars_))          , locs_(std::forward<QQ>(locs)), edges_(std::forward<EE>(edges)) { 
    requires(locs_.size() > 0)
    requires(check_loc_ids())
    refill_edge_indices(); }

  template<typename VV, typename QQ, typename EE>
  hybrid_automaton(VV&& vars, QQ&& locs, EE&& edges) : 
    hybrid_automaton("", std::forward<VV>(vars), std::forward<QQ>(locs), std::forward<EE>(edges)) { }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------
  
  bool check_loc_ids() {
    static_assert(std::is_integral<loc_id_t>::value);
    static_assert(std::is_unsigned<loc_id_t>::value);
    const auto locs_size = locs_.size();
    for(id_t i = 0; i < locs_size; i++)
      requires_msg(locs_[i].id() == i, "identifier of the locs are not sequential (id of the " << i << "-th location is " << locs_[i].id() << ")");
    return true;  }

  void refill_edge_indices() {
    sync_edges_ .clear();
    in_edges_   .clear();
    out_edges_  .clear();
    broad_edges_.clear();

    const auto locs_size  = locs_ .size();
    const auto edges_size = edges_.size();

    sync_edges_ .resize(locs_size);
    in_edges_   .resize(locs_size);
    out_edges_  .resize(locs_size);
    broad_edges_.resize(locs_size);

    for(edge_index_t i = 0; i < edges_size; i++) {
      cmn::pbv_t<loc_id_t> src = edges_[i].src();
      switch(edges_[i].knd()) {
        case label_kind::SYNC      : sync_edges_ [src].push_back(i); break;
        case label_kind::IN        : in_edges_   [src].push_back(i); break;
        case label_kind::OUT       : out_edges_  [src].push_back(i); break;
        case label_kind::BROADCAST : broad_edges_[src].push_back(i); break; } }
  }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  str_t    & name () noexcept { return name_ ; }
  var_set  & vars () noexcept { return vars_ ; }
  id2var   & i2v  () noexcept { return i2v_  ; }
  name2var & n2v  () noexcept { return n2v_  ; }
  locs_t   & locs () noexcept { return locs_ ; }
  edges_t  & edges() noexcept { return edges_; }

  edge_indices_vec_t& sync_edges () noexcept { return sync_edges_ ; }
  edge_indices_vec_t& in_edges   () noexcept { return in_edges_   ; }
  edge_indices_vec_t& out_edges  () noexcept { return out_edges_  ; }
  edge_indices_vec_t& broad_edges() noexcept { return broad_edges_; }

  loc_t &         loc        (cmn::pbv_t<loc_id_t> id) noexcept { return locs_       [id]; }
  edge_t&         edge       (cmn::pbv_t<loc_id_t> id) noexcept { return edges_      [id]; }
  edge_indices_t& sync_edges (cmn::pbv_t<loc_id_t> id) noexcept { return sync_edges_ [id]; }
  edge_indices_t& in_edges   (cmn::pbv_t<loc_id_t> id) noexcept { return in_edges_   [id]; }
  edge_indices_t& out_edges  (cmn::pbv_t<loc_id_t> id) noexcept { return out_edges_  [id]; }
  edge_indices_t& broad_edges(cmn::pbv_t<loc_id_t> id) noexcept { return broad_edges_[id]; }

  cmn::pbv_t<str_t   > name () const noexcept { return name_ ; }
  cmn::pbv_t<var_set > vars () const noexcept { return vars_ ; }
  cmn::pbv_t<id2var  > i2v  () const noexcept { return i2v_  ; }
  cmn::pbv_t<name2var> n2v  () const noexcept { return n2v_  ; }
  cmn::pbv_t<locs_t  > locs () const noexcept { return locs_ ; }
  cmn::pbv_t<edges_t > edges() const noexcept { return edges_; }

  cmn::pbv_t<edge_indices_vec_t> sync_edges () const noexcept { return sync_edges_ ; }
  cmn::pbv_t<edge_indices_vec_t> in_edges   () const noexcept { return in_edges_   ; }
  cmn::pbv_t<edge_indices_vec_t> out_edges  () const noexcept { return out_edges_  ; }
  cmn::pbv_t<edge_indices_vec_t> broad_edges() const noexcept { return broad_edges_; }

  cmn::pbv_t<loc_t >         loc        (cmn::pbv_t<loc_id_t> id) const noexcept { return locs_       [id]; }
  cmn::pbv_t<edge_t>         edge       (cmn::pbv_t<loc_id_t> id) const noexcept { return edges_      [id]; }
  cmn::pbv_t<edge_indices_t> sync_edges (cmn::pbv_t<loc_id_t> id) const noexcept { return sync_edges_ [id]; }
  cmn::pbv_t<edge_indices_t> in_edges   (cmn::pbv_t<loc_id_t> id) const noexcept { return in_edges_   [id]; }
  cmn::pbv_t<edge_indices_t> out_edges  (cmn::pbv_t<loc_id_t> id) const noexcept { return out_edges_  [id]; }
  cmn::pbv_t<edge_indices_t> broad_edges(cmn::pbv_t<loc_id_t> id) const noexcept { return broad_edges_[id]; }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  bool operator!=(const type& that) const { return !operator==(that); }
  bool operator==(const type& that) const { return this == &that || (name_          == that.name_          && 
                                                                     vars_          == that.vars_          && 
                                                                     to_set(locs_ ) == to_set(that.locs_ ) && 
                                                                     to_set(edges_) == to_set(that.edges_) ); }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  template<typename Out, typename T>
  static void print_seq(Out& out, const T& list, const std::uint64_t indents, const id2var& i2v, const name2var& n2v) {
    const auto end = list.end();
    for(auto it = list.begin(); it != end; it++) 
      it->print(out, indents, i2v, n2v);  }

  template<typename Out>
  Out& print(Out& out, const std::uint64_t indent) const { 
    namespace utils = cmn::misc::utils;
    utils::indent(out, indent) << "hybrid-automaton ";
    if(!name_.empty()) out << "\"" << name_ << "\"";
    out << " {" << std::endl;
    print_seq(out, locs_ , indent + 4, i2v_, n2v_);
    print_seq(out, edges_, indent + 4, i2v_, n2v_);
    utils::indent(out, indent) << "}" << std::endl;
    return out;  }

  template<typename Out>
  Out& print(Out& out) const { return print(out, 0); } 

  template<typename T>
  static std::unordered_set<T> to_set(const std::vector<T>& vec) {
    return std::unordered_set<T>(vec.begin(), vec.end()); }

private:
  str_t    name_ ;
  var_set  vars_ ;
  id2var   i2v_  ;
  name2var n2v_  ;
  locs_t   locs_ ;
  edges_t  edges_;

  edge_indices_vec_t sync_edges_ ;
  edge_indices_vec_t in_edges_   ;
  edge_indices_vec_t out_edges_  ;
  edge_indices_vec_t broad_edges_;
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** @tparam HA      hybrid-automaton type
  * @tparam Init    initial state type
  * @tparam Unsafe  unsafe  state type 
  * @tparam InitP   initial state printer
  * @tparam UnsafeP unsafe  state printer
  * @tparam TInit   trivial initial state predicate
  * @tparam TUnsafe trivial unsafe  state predicate  */
template<typename HA, typename Init, typename Unsafe, typename InitP, typename UnsafeP, typename TInit, typename TUnsafe>
struct safety {

  using type              = safety ;
  using ha_t              = HA     ;
  using ha_seq_t          = std::vector<ha_t>;      // must be random access
  using ha_index_t        = typename ha_seq_t::size_type;
  using inits_t           = Init   ;
  using unsafes_t         = Unsafe ;
  using inits_printer_t   = InitP  ;
  using unsafes_printer_t = UnsafeP;
  using trivial_inits_t   = TInit  ;
  using trivial_unsafes_t = TUnsafe;
  using props_t           = boost::property_tree::ptree;
  using edge_index_t      = typename ha_t::edge_index_t;
  using loc_index_t       = typename ha_t::loc_index_t;
  using par_loc_t         = std::vector<loc_index_t>;
  using par_edge_t        = std::vector<std::tuple<ha_index_t, edge_index_t>>;

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  template<typename HH>
  using is_ha = std::is_same<ha_t, std::remove_const_t<std::remove_reference_t<HH>>>;

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  template<typename HAs, typename II, typename UU, typename PP, std::enable_if_t<!is_ha<HAs>::value>* = nullptr>
  safety(HAs&& ha_seq, II&& inits, UU&& unsafes, PP&& props) : 
    ha_seq_(std::forward<HAs>(ha_seq)), inits_(std::forward<II>(inits)), unsafes_(std::forward<UU>(unsafes)), props_(std::forward<PP>(props)),
    vars_(all_vars_of(ha_seq_)), i2v_(to_i2v(vars_)), n2v_(to_n2v(vars_)) { 
    requires(ha_seq_.size() > 0) }

  template<typename HAs, typename II, typename UU, std::enable_if_t<!is_ha<HAs>::value>* = nullptr>
  safety(HAs&& ha_seq, II&& inits, UU&& unsafes) : 
    safety(std::forward<HAs>(ha_seq), std::forward<II>(inits), std::forward<UU>(unsafes), props_t()) { }

  template<typename HH, typename II, typename UU, typename PP, std::enable_if_t<is_ha<HH>::value>* = nullptr>
  safety(HH&& ha, II&& inits, UU&& unsafes, PP&& props) : 
    safety(ha_seq_t({std::forward<HH>(ha)}), std::forward<II>(inits), std::forward<UU>(unsafes), std::forward<PP>(props)) { }

  template<typename HH, typename II, typename UU, std::enable_if_t<is_ha<HH>::value>* = nullptr>
  safety(HH&& ha, II&& inits, UU&& unsafes) : 
    safety(std::forward<HH>(ha), std::forward<II>(inits), std::forward<UU>(unsafes), props_t()) { }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  ha_seq_t & ha_seq () noexcept { return ha_seq_ ; }
  inits_t  & inits  () noexcept { return inits_  ; }
  unsafes_t& unsafes() noexcept { return unsafes_; }
  props_t  & props  () noexcept { return props_  ; }
  var_set  & vars   () noexcept { return vars_   ; }
  id2var   & i2v    () noexcept { return i2v_    ; }
  name2var & n2v    () noexcept { return n2v_    ; }

  ha_t&            ha(cmn::pbv_t<ha_index_t> index)       noexcept { return ha_seq_[index] ; }
  cmn::pbv_t<ha_t> ha(cmn::pbv_t<ha_index_t> index) const noexcept { return ha_seq_[index] ; }

  cmn::pbv_t<ha_seq_t > ha_seq () const noexcept { return ha_seq_ ; }
  cmn::pbv_t<inits_t  > inits  () const noexcept { return inits_  ; }
  cmn::pbv_t<unsafes_t> unsafes() const noexcept { return unsafes_; }
  cmn::pbv_t<props_t  > props  () const noexcept { return props_  ; }
  cmn::pbv_t<var_set  > vars   () const noexcept { return vars_   ; }
  cmn::pbv_t<id2var   > i2v    () const noexcept { return i2v_    ; }
  cmn::pbv_t<name2var > n2v    () const noexcept { return n2v_    ; }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  bool operator!=(const type& that) const { return !operator==(that); }
  bool operator==(const type& that) const { return this == &that || (ha_seq_  == that.ha_seq_  &&
                                                                     inits_   == that.inits_   &&
                                                                     unsafes_ == that.unsafes_ && 
                                                                     props_   == that.props_   );  }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  template<typename Out>
  Out& print(Out& out, const std::uint64_t indent) const { 
    namespace utils = cmn::misc::utils;

    utils::indent(out, indent) << "vars \"";
    bool first = true;
    for(auto& v : vars_) {
      if(first) first = false; else out << ", ";
      out << v; }
    out << "\"" << std::endl;

    for(cmn::pbv_t<ha_t> ha : ha_seq_) 
      ha.print(out, indent);
    if(!trivial_inits  (inits_  )) 
        utils::indent(inits_printer  (utils::indent(out, indent) << "inits   {" << std::endl, inits_  , indent + 4, i2v_, n2v_), indent) << "}" << std::endl;
    if(!trivial_unsafes(unsafes_)) 
        utils::indent(unsafes_printer(utils::indent(out, indent) << "unsafes {" << std::endl, unsafes_, indent + 4, i2v_, n2v_), indent) << "}" << std::endl; 
    print_props(out, props_, indent);
    return out;  }

  template<typename Out>
  Out& print(Out& out) const { return print(out, 0); } 
  
private:

  static auto all_vars_of(const ha_seq_t& seq) {
    var_set res;
    for(const auto& ha : seq)
      for(const auto& v : ha.vars()) {
        const auto pair = res.insert(v);
        const auto& old = (*std::get<0>(pair))->variable();
        requires_msg(
            old.name() == v->variable().name(), 
            "variable (" << v->variable().name() << ") and variable (" << old.name() << ") have the same id (" << old.id() << ")") }
    return res; }

private:
  static std::string add_indent(const std::string& str, const std::uint64_t indent) {
    namespace utils = cmn::misc::utils;
    std::stringstream buff;
    buff << std::endl;
    const auto eof = buff.str();
    utils::indent(buff, indent);
    return utils::replace_all(utils::rtrim(str), eof, buff.str()); }

  template<typename Out, typename PT>
  static Out& print_props(Out& out, const PT& pt, const std::uint64_t indent) { 
    namespace utils = cmn::misc::utils;
    utils::indent(utils::indent(out, indent) << "props { " << std::endl, indent + 4);
    std::stringstream buff;
    boost::property_tree::info_parser::info_writer_settings<char> prop_writer_info(' ', 4);
    boost::property_tree::info_parser::write_info(buff, pt, prop_writer_info);
    out << add_indent(buff.str(), indent + 4) << std::endl;
    utils::indent(out, indent) << "}" << std::endl;
    return out;  }
  
private:
  ha_seq_t  ha_seq_ ;
  inits_t   inits_  ;
  unsafes_t unsafes_;
  props_t   props_  ;
  var_set   vars_   ;
  id2var    i2v_    ;
  name2var  n2v_    ;

  static const InitP   inits_printer  ;
  static const UnsafeP unsafes_printer;
  static const TInit   trivial_inits  ;
  static const TUnsafe trivial_unsafes;
};

template<typename HA, typename Init, typename Unsafe, typename InitP, typename UnsafeP, typename TInit, typename TUnsafe>
const InitP safety<HA, Init, Unsafe, InitP, UnsafeP, TInit, TUnsafe>::inits_printer { };

template<typename HA, typename Init, typename Unsafe, typename InitP, typename UnsafeP, typename TInit, typename TUnsafe>
const UnsafeP safety<HA, Init, Unsafe, InitP, UnsafeP, TInit, TUnsafe>::unsafes_printer { };

template<typename HA, typename Init, typename Unsafe, typename InitP, typename UnsafeP, typename TInit, typename TUnsafe>
const TInit safety<HA, Init, Unsafe, InitP, UnsafeP, TInit, TUnsafe>::trivial_inits { };

template<typename HA, typename Init, typename Unsafe, typename InitP, typename UnsafeP, typename TInit, typename TUnsafe>
const TUnsafe safety<HA, Init, Unsafe, InitP, UnsafeP, TInit, TUnsafe>::trivial_unsafes { };

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename Out, typename HA, typename Init, typename Unsafe, typename InitP, typename UnsafeP, typename TInit, typename TUnsafe>
inline Out& operator<<(Out& out, const safety<HA, Init, Unsafe, InitP, UnsafeP, TInit, TUnsafe>& safety) { safety.print(out); return out; }

template<typename Out, typename Q, typename E>
inline Out& operator<<(Out& out, const hybrid_automaton<Q,E>& hybrid_automaton) { hybrid_automaton.print(out); return out; }

template<typename Out, typename ID, typename Lbl, typename Rel, typename RelP, typename TRel>
inline Out& operator<<(Out& out, const edge<ID,Lbl, Rel, RelP, TRel>& edge) { edge.print(out); return out; }

template<typename Out, typename ID, typename Inv, typename Flow, typename InvP, typename FlowP, typename TInv, typename TFlow>
inline Out& operator<<(Out& out, const loc<ID, Inv, Flow, InvP, FlowP, TInv, TFlow> & loc) { loc.print(out); return out; }

} // namespace ha

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace std {

template<typename ID, typename Inv, typename Flow, typename InvP, typename FlowP, typename TInv, typename TFlow>
struct hash<hare::loc<ID, Inv, Flow, InvP, FlowP, TInv, TFlow>> {
  using argument_type = hare::loc<ID, Inv, Flow, InvP, FlowP, TInv, TFlow>;
  using result_type   = size_t;
  result_type operator()(const argument_type& that) const {
    return that.id();  }  };

template<typename ID, typename Lbl, typename Rel, typename RelP, typename TRel>
struct hash<hare::edge<ID, Lbl, Rel, RelP, TRel>> {
  using argument_type = hare::edge<ID, Lbl, Rel, RelP, TRel>;
  using result_type   = size_t;
  result_type operator()(const argument_type& that) const { 
    return hash<ID>{}(that.src()) ^ hash<ID>{}(that.dst()) ^ hash<Lbl>{}(that.lbl()) ^ hash<hare::label_kind>{}(that.knd()) /*^ hash<Rel>{}(that.rel())*/; } };

template<typename ID, typename Lbl, typename Grd, typename Rst, typename GrdP, typename RstP, typename TGrd, typename TRst>
struct hash<hare::simple_edge<ID, Lbl, Grd, Rst, GrdP, RstP, TGrd, TRst>> {
  using argument_type = hare::simple_edge<ID, Lbl, Grd, Rst, GrdP, RstP, TGrd, TRst>;
  using result_type   = size_t;
  result_type operator()(const argument_type& that) const { 
    return hash<ID>            {}(that.src()) ^ hash<ID> {}(that.dst  ()) ^ hash<Lbl>{}(that.lbl  ()) ^ 
           hash<hare::label_kind>{}(that.knd()) /*^ hash<Grd>{}(that.guard()) ^ hash<Rst>{}(that.reset())*/ ; } };

} // namespace std

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif // HA__HYBRID_AUTOMATON__HPP
