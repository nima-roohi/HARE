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

#include "performance_counter.hpp"

namespace ha::pc {

serializers_t serializers   ({&string_serializer});
pointers_t    pointers      ({&curr_rec_name});
table_t       table         {};
header_t      header        {{"name"}};
std::string   curr_rec_name {};

/** types of header_##var and pointer_##var are irrelevant. I just want to make automatic initialization possible */
#define add_metric(var, name, type, serializer)                                                           \
  type var{};                                                                                             \
  typename serializers_t::iterator serializer_##var = serializers.insert(serializers.end(), &serializer); \
  typename pointers_t   ::iterator pointer_##var    = pointers   .insert(pointers   .end(), &var);        \
  typename header_t     ::iterator header_ ##var    = header     .insert(header     .end(), name);

#define add_counter_metric(var, name)      add_metric(counter_##var, name, counter_t, counter_serializer)
#define add_timer_metric(var, name)        add_metric(timer_##var  , name, timer_t  , timer_serializer  )
#define add_boolean_metric(var, name)      add_metric(bool_##var   , name, bool     , boolean_serializer)                
#define add_typed_metric(var, name, type)  add_metric(typed_##var  , name, type     , type##_serializer )                

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#define DEL_counter_metric(var, name)
#define DEL_timer_metric  (var, name)         
#define DEL_boolean_metric(var, name)         
#define DEL_typed_metric  (var, name, type)         

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** YES! this is the right place for inclusion, not the begining of this file */
#include "hare/performance_metrics.hpp"

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#undef add_metric
#undef add_counter_metric
#undef add_timer_metric
#undef add_boolean_metric
#undef add_typed_metric

#undef DEL_counter_metric
#undef DEL_timer_metric
#undef DEL_boolean_metric
#undef DEL_typed_metric

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

} // namespace ha::pc
