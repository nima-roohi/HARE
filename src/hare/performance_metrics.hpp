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

// If you want to disable a metric, simply replace "add_" by "DEL_"

add_timer_metric( mc_nlfpoly_total         , "mc-nlfpoly-total-time"       )
add_timer_metric( mc_nlfpoly_initialization, "mc-nlfpoly-initialization"   )
add_timer_metric( mc_nlfpoly_reachability  , "mc-nlfpoly-reachability"     )
add_timer_metric( mc_nlfpoly_abstraction   , "mc-nlfpoly-abstraction-time" )
add_timer_metric( mc_nlfpoly_refinement    , "mc-nlfpoly-refinement-time"  )
add_timer_metric( mc_nlfpoly_ce_check      , "mc-nlfpoly-ce-check-time"    )
add_timer_metric( mc_dreach_total          , "mc-dreach-time"              )

add_timer_metric( mc_poly_total         , "mc-poly-total-time"          )
add_timer_metric( mc_poly_fixpoint      , "mc-poly-fixpoint-time"       )
add_timer_metric( mc_poly_cont_post     , "mc-poly-cont-post-time"      )
add_timer_metric( mc_poly_disc_post     , "mc-poly-dist-post-time"      )
add_timer_metric( mc_poly_unsafe_check  , "mc-poly-unsafe-check-time"   )
add_timer_metric( mc_poly_initialization, "mc-poly-initialization-time" )
add_timer_metric( mc_poly_reachablity   , "mc-poly-reachability-time"   ) //< time taken after initialization
add_timer_metric(         safety_parser , "safety-parse-time"           ) 

add_counter_metric( mc_nlfpoly_iteration         , "mc-nlfpoly-iterations"         )

add_counter_metric(    poly_dim                  , "poly-dimension"                )
add_counter_metric(    poly_init_polies_count    , "poly-init-polyhedra-count"     ) //< total number of initial polyhedra
add_counter_metric(    poly_unsafe_polies_count  , "poly-unsafe-polyhedra-count"   ) //< total number of unsafe  polyhedra
add_counter_metric(    poly_unsafe_loc_count     , "poly-unsafe-loc-count"         ) //< total number of unsafe  locations
add_counter_metric(    poly_trivial_unsafe_count , "poly-trivial-unsafe-count"     ) //< total number of unsafe locations with trivial (true) sets
add_counter_metric( mc_poly_iteration            , "mc-poly-iterations"            )
add_counter_metric( mc_poly_cont_post            , "mc-poly-cont-posts"            )
add_counter_metric( mc_poly_disc_post            , "mc-poly-disc-posts"            )
add_counter_metric( mc_poly_successful_disc_post , "mc-poly-successful-disc-posts" ) //< result of discrete post is not an empty set
add_counter_metric( mc_poly_fixpoint_check       , "mc-poly-fixpoint-checks"       )
add_counter_metric( mc_poly_fixpoint_hit         , "mc-poly-fixpoint-hits"         ) //< fixpoint check returns true (the set has already been considered)
add_counter_metric( mc_poly_unsafe_check         , "mc-poly-unsafe-checks"         ) //< checking intersection with the unsafe set
add_counter_metric( mc_poly_reached_loc_count    , "mc-poly-reached-loc-count"     ) //< total number of visited locations
add_counter_metric( mc_poly_reached_edge_count   , "mc-poly-reached-edge-count"    ) //< total number of visited parallel edges
add_counter_metric( mc_poly_nonempty_edge_count  , "mc-poly-nonempty-edge-count"   ) //< total number of visited parallel edges with nonempty relation

add_typed_metric( mc_nlfpoly_safety_result       , "mc-nlfpoly-safety_result", nlfpoly_safety_result )
add_typed_metric( mc_poly_safety_result          , "mc-poly-safty_result"    ,    poly_safety_result )

add_boolean_metric( mc_poly_inversed_time          , "mc-poly-inversed-time"           )
add_boolean_metric( mc_poly_separated_id_resets    , "mc-poly-separated-id-resets"     )
add_boolean_metric( mc_poly_check_unsafe_after_disc, "mc-poly-check-unsafe-after-disc" )

