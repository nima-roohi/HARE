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

#include <ppl.hh>

#include <vector>

/** Unfortunately I keep getting "undefined reference error" for "permute_space_dimensions" in Ubuntu. So here I copy pasted their implementation. */
#ifdef __linux__

// copy pasted from ppl-1.2 built folder src/Constraint.cc
void Parma_Polyhedra_Library::Constraint::permute_space_dimensions(const std::vector<Parma_Polyhedra_Library::Variable>& cycle) {
  if (cycle.size() < 2) {
    // No-op. No need to call sign_normalize().
    return; }
  expr.permute_space_dimensions(cycle);
  // *this is still normalized but may be not strongly normalized:
  // sign normalization is necessary.
  sign_normalize();
  PPL_ASSERT(OK());  }

// copy pasted from ppl-1.2 built folder src/Generator.cc
void Parma_Polyhedra_Library::Generator::permute_space_dimensions(const std::vector<Parma_Polyhedra_Library::Variable>& cycle) {
  if (cycle.size() < 2) {
    // No-op. No need to call sign_normalize().
    return; }
  expr.permute_space_dimensions(cycle);
  // *this is still normalized but may be not strongly normalized: sign
  // normalization is necessary.
  sign_normalize();
  PPL_ASSERT(OK());  }

#endif // __linux__
