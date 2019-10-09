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

#ifndef HA__UTILS_PROPS__HPP
#define HA__UTILS_PROPS__HPP

#include "hare/performance_counter.hpp"
#include "hare/poly_ha.hpp"
#include "hare/utils_poly_ha.hpp"
#include "hare/utils_ppl.hpp"
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

namespace ha {

using key_t      = const char*;
using val_str_t  = const char*;
using val_bool_t = bool;
using val_num_t  = constant;
using val_uint_t = std::uint64_t;

template<typename T>
using actualT = typename boost::mpl::if_<std::is_same<T, val_str_t>, std::string, T>::type;

template<typename T>
auto prop_value(const boost::property_tree::ptree& props, cmn::pbv_t<key_t> key, cmn::pbv_t<T> def) { return props.get<actualT<T>>(key, def); }

template<typename T>
auto prop_value(const boost::property_tree::ptree& props, cmn::pbv_t<key_t> key) {
  const auto res = props.get_optional<actualT<T>>(key);
  requires_msg(res, "key <<<" << key << ">>> is not defined")
  return *res; }

} // namespace ha

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif // HA__UTILS_PROPS__HPP
