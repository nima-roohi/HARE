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

#ifndef HA__SAFETY_RESULT__HPP
#define HA__SAFETY_RESULT__HPP

#include <cstdint>

namespace hare {

enum class poly_safety_result {
  UNSAFE,
  SAFE,
  BOUNDED_SAFE
};

enum class nlfpoly_safety_result {
  UNKNOWN,
  UNSAFE,
  SAFE,
  ABSTRACT_BOUNDED_SAFE
};

template<typename Out>
inline Out& operator<<(Out& os, const poly_safety_result result) {
  switch(result) {
  case poly_safety_result::UNSAFE       : os << "UNSAFE"      ; return os;
  case poly_safety_result::SAFE         : os << "SAFE"        ; return os;
  case poly_safety_result::BOUNDED_SAFE : os << "BOUNDED_SAFE"; return os;
  }
}

template<typename Out>
inline Out& operator<<(Out& os, const nlfpoly_safety_result result) {
  switch(result) {
  case nlfpoly_safety_result::UNKNOWN               : os << "UNKNOWN"              ; return os;
  case nlfpoly_safety_result::UNSAFE                : os << "UNSAFE"               ; return os;
  case nlfpoly_safety_result::SAFE                  : os << "SAFE"                 ; return os;
  case nlfpoly_safety_result::ABSTRACT_BOUNDED_SAFE : os << "ABSTRACT_BOUNDED_SAFE"; return os;
  }
}


} // namespace hare

#endif // HA__SAFETY_RESULT__HPP
