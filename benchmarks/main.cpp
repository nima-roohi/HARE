
#define CMN_LOG_LEVEL CMN_LOG_LEVEL_TRACE

#include "cmn/dbc.hpp"
#include "hare/nlfpoly_ha_parser.hpp"
#include "hare/nlfpoly_ha_safety_checker.hpp"
#include "hare/poly_ha_parser.hpp"
#include "hare/poly_ha_safety_checker.hpp"
#include "hare/performance_counter.hpp"
#include "hare/safety_result.hpp"

#include <fstream> 
#include <ios>
#include <string>

using namespace hare;

using CPoly   = poly_ha::CPoly  ;
using NNCPoly = poly_ha::NNCPoly;

using nlf  = nlfpoly_safety_result;
using rect = poly_safety_result   ;

const std::string path_bench      = "./benchmarks/";
const std::string path_heats      = "heaters/"     ;
const std::string path_sats       = "satellites/"  ;
const std::string path_navs       = "navigations/" ;
const std::string path_sttt_navs  = "sttt-navs/"   ;
const std::string path_tanks      = "tanks/"       ;
const std::string path_hsolver    = "HSolver/"     ;

void check(const rect expected, const std::string& name, const std::string& fullname) { 
  const pc::Record record{name};
  log_info << "checking " << name;
  auto safety = poly_ha::parser::parse_poly_safety_from_file<NNCPoly>(fullname);
  const poly_ha::safety_checker<NNCPoly> mc(safety);
  const rect actual = mc.safety_result();
  if(actual == expected) log_info  << name << " is " << actual; 
  else                   log_error << name << " is " << actual; }

void check(const nlf expected, const std::string& name, const std::string& fullname) { 
  const pc::Record record{name};
  log_info << "checking " << name;
  auto safety = nlfpoly_ha::parser::parse_nlfpoly_safety_from_file<NNCPoly>(fullname);
  const nlfpoly_ha::safety_checker<NNCPoly> mc(safety);
  const nlf actual = mc.safety_result();
  if(actual == expected) log_info  << name << " is " << actual; 
  else                   log_error << name << " is " << actual; }

void print_and_save_table() {
  hare::pc::print_table(std::cout);
  std::ofstream out("metrics", std::ios::app);
  hare::pc::print_table(out);
  out << std::endl;
  out.close(); }

int main() {
  
//   check(rect::UNSAFE, "tank-01", path_bench + path_tanks + "tank-01/tank.prb");
//   check(rect::UNSAFE, "tank-02", path_bench + path_tanks + "tank-02/tank.prb");
//   check(rect::UNSAFE, "tank-03", path_bench + path_tanks + "tank-03/tank.prb");
//   check(rect::UNSAFE, "tank-04", path_bench + path_tanks + "tank-04/tank.prb");
// //check(rect::UNSAFE, "tank-05", path_bench + path_tanks + "tank-05/tank.prb"); // TOO SLOW
//   check(rect::UNSAFE, "tank-06", path_bench + path_tanks + "tank-06/tank.prb");
//   check(rect::UNSAFE, "tank-07", path_bench + path_tanks + "tank-07/tank.prb");
//   check(rect::UNSAFE, "tank-08", path_bench + path_tanks + "tank-08/tank.prb");
//   check(rect::UNSAFE, "tank-09", path_bench + path_tanks + "tank-09/tank.prb");
//   check(rect::UNSAFE, "tank-10", path_bench + path_tanks + "tank-10/tank.prb");
//   check(rect::UNSAFE, "tank-11", path_bench + path_tanks + "tank-11/tank.prb");
//   check(rect::UNSAFE, "tank-12", path_bench + path_tanks + "tank-12/tank.prb");
//   check(rect::UNSAFE, "tank-13", path_bench + path_tanks + "tank-13/tank.prb");
//   check(rect::UNSAFE, "tank-14", path_bench + path_tanks + "tank-14/tank.prb");
//   check(rect::UNSAFE, "tank-15", path_bench + path_tanks + "tank-15/tank.prb");
//   check(rect::SAFE  , "tank-16", path_bench + path_tanks + "tank-16/tank.prb");
//   check(rect::SAFE  , "tank-17", path_bench + path_tanks + "tank-17/tank.prb");

//   // @note technically, dReach does not support any of the satellite examples. The reason is that these examples all have noise variables which means their 
//   //       invariant is bounded and their flows are same as their invariant, which means flows are rectangular constraints not equality constraints. But we 
//   //       can check them all since the abstraction is safe, so there is no need to call dReach. 
//   // @note all of satellite examples will be proved safe in the first iteration of the cegar loop. Therefore, there is no need to bound duration of continouus 
//   //       transitions since no location will be splitted. This reduces number of edges and results in efficiency improvement. 
//   check(nlf::SAFE, "sat-01", path_bench + path_sats + "sat-01/sat.prb");
//   check(nlf::SAFE, "sat-02", path_bench + path_sats + "sat-02/sat.prb");
//   check(nlf::SAFE, "sat-03", path_bench + path_sats + "sat-03/sat.prb");
//   check(nlf::SAFE, "sat-04", path_bench + path_sats + "sat-04/sat.prb");
//   check(nlf::SAFE, "sat-05", path_bench + path_sats + "sat-05/sat.prb");
//   check(nlf::SAFE, "sat-06", path_bench + path_sats + "sat-06/sat.prb");
//   check(nlf::SAFE, "sat-07", path_bench + path_sats + "sat-07/sat.prb");
//   check(nlf::SAFE, "sat-08", path_bench + path_sats + "sat-08/sat.prb");
//   check(nlf::SAFE, "sat-09", path_bench + path_sats + "sat-09/sat.prb"); 
//   check(nlf::SAFE, "sat-10", path_bench + path_sats + "sat-10/sat.prb");
//   check(nlf::SAFE, "sat-11", path_bench + path_sats + "sat-11/sat.prb");
//   check(nlf::SAFE, "sat-12", path_bench + path_sats + "sat-12/sat.prb");
//   check(nlf::SAFE, "sat-13", path_bench + path_sats + "sat-13/sat.prb");
//   check(nlf::SAFE, "sat-14", path_bench + path_sats + "sat-14/sat.prb");
//   check(nlf::SAFE, "sat-15", path_bench + path_sats + "sat-15/sat.prb");

// //check(nlf::UNSAFE, "heater-01", path_bench + path_heats + "heater-01/heater.prb"); // TOO SLOW
//   check(nlf::UNSAFE, "heater-02", path_bench + path_heats + "heater-02/heater.prb");
//   check(nlf::SAFE  , "heater-03", path_bench + path_heats + "heater-03/heater.prb"); // needs 5 initial refinement
//   check(nlf::SAFE  , "heater-04", path_bench + path_heats + "heater-04/heater.prb"); // forward 2080s
//   check(nlf::SAFE  , "heater-05", path_bench + path_heats + "heater-05/heater.prb");
// //check(nlf::UNSAFE, "heater-06", path_bench + path_heats + "heater-06/heater.prb"); // TOO SLOW
// //check(nlf::SAFE  , "heater-07", path_bench + path_heats + "heater-07/heater.prb"); // TOO SLOW
// //check(nlf::SAFE  , "heater-08", path_bench + path_heats + "heater-08/heater.prb"); // TOO SLOW
//   check(nlf::SAFE  , "heater-09", path_bench + path_heats + "heater-09/heater.prb"); 
// //check(nlf::UNSAFE, "heater-10", path_bench + path_heats + "heater-10/heater.prb"); // TOO SLOW
// //check(nlf::SAFE  , "heater-11", path_bench + path_heats + "heater-11/heater.prb"); // TOO SLOW
//   check(nlf::UNSAFE, "heater-12", path_bench + path_heats + "heater-12/heater.prb");
// //check(nlf::SAFE  , "heater-13", path_bench + path_heats + "heater-13/heater.prb"); // TOO SLOW
// //check(nlf::UNSAFE, "heater-14", path_bench + path_heats + "heater-14/heater.prb"); // TOO SLOW
// //check(nlf::UNSAFE, "heater-15", path_bench + path_heats + "heater-15/heater.prb"); // TOO SLOW

//   check(nlf::SAFE  , "nav-01", path_bench + path_navs + "nav-01/nav.prb");
//   check(nlf::UNSAFE, "nav-02", path_bench + path_navs + "nav-02/nav.prb");
//   check(nlf::UNSAFE, "nav-03", path_bench + path_navs + "nav-03/nav.prb");
//   check(nlf::SAFE  , "nav-04", path_bench + path_navs + "nav-04/nav.prb"); 
//   check(nlf::SAFE  , "nav-05", path_bench + path_navs + "nav-05/nav.prb");
//   check(nlf::SAFE  , "nav-06", path_bench + path_navs + "nav-06/nav.prb"); 
//   check(nlf::UNSAFE, "nav-07", path_bench + path_navs + "nav-07/nav.prb");
//   check(nlf::SAFE  , "nav-08", path_bench + path_navs + "nav-08/nav.prb"); 
//   check(nlf::SAFE  , "nav-09", path_bench + path_navs + "nav-09/nav.prb"); 
//   check(nlf::SAFE  , "nav-10", path_bench + path_navs + "nav-10/nav.prb"); 
//   check(nlf::SAFE  , "nav-11", path_bench + path_navs + "nav-11/nav.prb"); 
//   check(nlf::SAFE  , "nav-12", path_bench + path_navs + "nav-12/nav.prb"); 
//   check(nlf::SAFE  , "nav-13", path_bench + path_navs + "nav-13/nav.prb"); 
//   check(nlf::SAFE  , "nav-14", path_bench + path_navs + "nav-14/nav.prb"); 
//   check(nlf::SAFE  , "nav-15", path_bench + path_navs + "nav-15/nav.prb"); 
//   check(nlf::SAFE  , "nav-16", path_bench + path_navs + "nav-16/nav.prb");
//   check(nlf::SAFE  , "nav-17", path_bench + path_navs + "nav-17/nav.prb");
//   check(nlf::SAFE  , "nav-18", path_bench + path_navs + "nav-18/nav.prb");
//   check(nlf::SAFE  , "nav-19", path_bench + path_navs + "nav-19/nav.prb");
//   check(nlf::SAFE  , "nav-20", path_bench + path_navs + "nav-20/nav.prb");
//   check(nlf::SAFE  , "nav-21", path_bench + path_navs + "nav-21/nav.prb");
//   check(nlf::SAFE  , "nav-22", path_bench + path_navs + "nav-22/nav.prb"); 
//   check(nlf::SAFE  , "nav-23", path_bench + path_navs + "nav-23/nav.prb"); 
//   check(nlf::SAFE  , "nav-24", path_bench + path_navs + "nav-24/nav.prb"); 
//   check(nlf::SAFE  , "nav-25", path_bench + path_navs + "nav-25/nav.prb"); 
//   check(nlf::SAFE  , "nav-26", path_bench + path_navs + "nav-26/nav.prb"); 
//   check(nlf::SAFE  , "nav-27", path_bench + path_navs + "nav-27/nav.prb"); 
//   check(nlf::SAFE  , "nav-28", path_bench + path_navs + "nav-28/nav.prb"); 
//   check(nlf::SAFE  , "nav-29", path_bench + path_navs + "nav-29/nav.prb"); 
//   check(nlf::SAFE  , "nav-30", path_bench + path_navs + "nav-30/nav.prb"); 
//   check(nlf::SAFE  , "nav-31", path_bench + path_navs + "nav-31/nav.prb"); 
//   check(nlf::SAFE  , "nav-32", path_bench + path_navs + "nav-32/nav.prb"); 
//   check(nlf::SAFE  , "nav-33", path_bench + path_navs + "nav-33/nav.prb"); 
//   check(nlf::SAFE  , "nav-34", path_bench + path_navs + "nav-34/nav.prb"); 

  // the only example can be added is "heating-old". Its relations contain many disjunctions.
  check(nlf::SAFE  , "1-flow"              , path_bench + path_hsolver + "1-flow.prb"              ); // rectangular
  check(nlf::SAFE  , "circuit"             , path_bench + path_hsolver + "circuit.prb"             ); // non-linear  
  check(nlf::SAFE  , "clock"               , path_bench + path_hsolver + "clock.prb"               ); // non-linear - needs 4 initial refinement
  check(nlf::SAFE  , "convoi-1"            , path_bench + path_hsolver + "convoi-1.prb"            ); // linear
  check(nlf::SAFE  , "cycle"               , path_bench + path_hsolver + "cycle.prb"               ); // rectangular
  check(nlf::SAFE  , "fischer2"            , path_bench + path_hsolver + "fischer2.prb"            ); // rectangular
  check(nlf::SAFE  , "fischer3"            , path_bench + path_hsolver + "fischer3.prb"            ); // rectangular
  check(nlf::SAFE  , "focus"               , path_bench + path_hsolver + "focus.prb"               ); // linear
//check(nlf::SAFE  , "mutant"              , path_bench + path_hsolver + "mutant.prb"              ); // linear - TOO SLOW
  check(nlf::SAFE  , "navigation-old"      , path_bench + path_hsolver + "navigation-old.prb"      ); // linear
  check(nlf::SAFE  , "navigation"          , path_bench + path_hsolver + "navigation.prb"          ); // linear
  check(nlf::SAFE  , "real-eigen"          , path_bench + path_hsolver + "real-eigen.prb"          ); // linear
  check(nlf::SAFE  , "sinusoid"            , path_bench + path_hsolver + "sinusoid.prb"            ); // non-linear - needs 5 initial refinement
  check(nlf::SAFE  , "trivial-hard"        , path_bench + path_hsolver + "trivial-hard.prb"        ); // rectangular
//check(nlf::SAFE  , "van-der-pol-hallstah", path_bench + path_hsolver + "van-der-pol-hallstah.prb"); // non-linear - TOO SLOW
  check(nlf::SAFE  , "van-der-pole"        , path_bench + path_hsolver + "van-der-pole.prb"        ); // non-linear

  print_and_save_table();
}
