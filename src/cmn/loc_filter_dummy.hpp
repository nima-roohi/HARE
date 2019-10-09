#ifndef __CMN__LOC_FILTER_DUMMY_HPP__
#define __CMN__LOC_FILTER_DUMMY_HPP__

#include <string>
#include <ios>      // ios_base
	
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
namespace cmn::detail {
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** @brief A dummy filter that does nothing. It is only here to make the code compilable even when runtime filtering is disabled. 
  * @see \c filter */
struct dummy_loc_filter {
  using type = dummy_loc_filter;

  constexpr unsigned size          () const { return 0   ; }
  constexpr bool     is_empty      () const { return true; }
  constexpr void     fill          ()       { }
  constexpr void     clear         ()       { }
  constexpr void     clear_and_fill()       { } 

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  constexpr bool is_match(std::string const&) { return true; }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  constexpr void add_file_filter             (std::string const&, bool const = true) { }
  constexpr void add_method_filter           (std::string const&, bool const = true) { }
  constexpr void add_recursive_path_filter   (std::string const&, bool const = true) { }
  constexpr void add_nonrecursive_path_filter(std::string const&, bool const = true) { }
  constexpr void add_regex_filter            (std::string const&, bool const = true) { }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  constexpr void remove_last_filter() { }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  constexpr char unsigned set_level(char unsigned const)       { return 0; }
  constexpr char unsigned get_level()                    const { return 0; }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  constexpr bool load_from_file  (std::string   const&) { return true; }
  constexpr bool load_from_string(std::string   const&) { return true; }
  constexpr bool load_from_stream(std::ios_base      &) { return true; }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  constexpr auto add_temp_file_filter             (std::string const&, bool const = true) { return 0; }
  constexpr auto add_temp_method_filter           (std::string const&, bool const = true) { return 0; }
  constexpr auto add_temp_recursive_path_filter   (std::string const&, bool const = true) { return 0; }
  constexpr auto add_temp_nonrecursive_path_filter(std::string const&, bool const = true) { return 0; }
  constexpr auto add_temp_regex_filter            (std::string const&, bool const = true) { return 0; }
  constexpr auto add_temp_negated_regex_filter    (std::string const&, bool const = true) { return 0; }

};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
} // namespace cmn::detail
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif // __CMN__LOC_FILTER_DUMMY_HPP__
