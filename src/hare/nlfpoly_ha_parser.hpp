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

#ifndef HA__NLFPOLY_HA_PARSER__HPP
#define HA__NLFPOLY_HA_PARSER__HPP

#include "hare/nlfpoly_ha.hpp"
#include "hare/poly_ha_parser.hpp"
#include "hare/performance_counter.hpp"
#include "hare/term.hpp"
#include "hare/term_parser.hpp"

#include "cmn/dbc.hpp"

#include <boost/iterator/filter_iterator.hpp>
#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>

#include <fstream>      // filebuf
#include <istream>      // istream
#include <ios>          // ios
#include <sstream>      // istringstream
#include <string>
#include <tuple>        // get
#include <type_traits>
#include <utility>
#include <vector>

namespace ha::nlfpoly_ha::parser {
  
namespace pt = boost::property_tree;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

const auto flow_parser = [](const boost::optional<const pt::ptree&>& tr, const name2var& n2v) {
  flow_map res;
  if(!tr) return res;
  for(const auto& ch : *tr) {
    const auto& name     = ch.first;
    const auto var_iter = n2v.find(name);
    requires_msg(var_iter != n2v.end(), "variable " << name << " is not defined")
    const auto& var = var_iter->second;
    requires_msg(res.find(var) == res.end(), "duplicate flow definition for variable " << name)
    const auto& str = ch.second.data();
    auto flow = ha::parser::parse_term(str.begin(), str.end(), n2v);
    res.emplace(var, std::move(flow)); }
  return res; };

using flow_parser_t = std::remove_const_t<std::remove_reference_t<decltype(flow_parser)>>; 

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename P>
auto parse_nlfpoly_safety(pt::ptree& tr) {
  namespace pparser = poly_ha::parser;
  using ha_seq_t = std::vector<nlfpoly_ha<P>>;
  return pparser::parse_safety<nlfpoly_safety<P>, 
                               nlfpoly_ha    <P>, 
                               nlfpoly_loc   <P>, 
                               nlfpoly_edge  <P>, 
                               pparser::poly_parser_t<P>, 
                               flow_parser_t            , 
                               pparser::poly_parser_t<P>, 
                               pparser::poly_map_parser_t<ha_seq_t, P>, 
                               pparser::poly_map_parser_t<ha_seq_t, P>
                              > 
                              (tr, 
                               pparser::poly_parser<P>, 
                               flow_parser            , 
                               pparser::poly_parser<P>, 
                               pparser::poly_map_parser<ha_seq_t, P>, 
                               pparser::poly_map_parser<ha_seq_t, P>);  }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename P>
nlfpoly_safety<P> parse_nlfpoly_safety_from_string(const std::string& str) {
  auto timer = pc::start_safety_parser();
  pt::ptree tree;
  std::istringstream buff(str);
  pt::info_parser::read_info(buff, tree);
  return parse_nlfpoly_safety<P>(tree); }

template<typename P>
nlfpoly_safety<P> parse_nlfpoly_safety_from_file(const std::string& file_path) {
  auto timer = pc::start_safety_parser();
  pt::ptree tree;
  std::filebuf fb;
  if (fb.open(file_path, std::ios::in)) {
    std::istream is(&fb);
    pt::info_parser::read_info(is, tree);
    fb.close();
  } else throw poly_ha::parser::parse_error(strerror(errno)); 
  return parse_nlfpoly_safety<P>(tree); }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

} // ha::nlfpoly_ha::parser

#endif // HA__NLFPOLY_HA_PARSER__HPP
