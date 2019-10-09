#ifndef __CMN__LOC_FILTER_HPP__
#define __CMN__LOC_FILTER_HPP__

#include <regex>
#include <string>
#include <tuple>
#include <vector>

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
namespace cmn::detail {
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

/** @brief Simple filter for filtering logs and dbc checks */
struct loc_filter {
  using type = loc_filter;
  using value_t = std::tuple<std::regex,bool>;
  using stack_t = std::vector<value_t>;

  auto size    () const { return stack.size (); }            ///< Return number of current filters
  bool is_empty() const { return stack.empty(); }            ///< Whether or not there is any filter
  void clear   ()       { stack.clear(); }                   ///< Clear the filter
  void fill    ()       { stack.emplace_back(".*", true); }  ///< Add a pattern that accepts everything (does not touch the current content).
  
  /** @brief First calls \c clear and then calls \c fill */
  void clear_and_fill() { 
    clear();
    fill (); }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  /** @brief Whether or not logging is enabled for the input address.
    *
    * The address is concatenation of macros `__FILE__` and `__FUNCTION__` with string `::` in the middle of them. 
    * 
    * If @ref is_empty returns \c true then this function returns \c true as well. Otherwise,
    * if the input \c addr matches any negative filter then returns \c false. Otherwise,
    * returns \c true iff at least one positive filter matches the input \c addr.
    * 
    * @note This method has no side-effect, which makes it thread-safe. However, simultaneous
    *       invocation of this method and initializations of filters is not thread-safe. 
    * @note The current implementation checks filters one by one. 
    * @note It is assumed that neither `__FILE__` nor `__FUNCTION__` contains "::" as their substring. */
  bool is_match(std::string const& addr);

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /** @brief Add the input file name to the filter.
    * 
    * If the input \c flt does not contain the dot character (".") then any file extension would be allowed. File extension is any 
    * string that starts with the dot character and does not contain any more dots (is defined by regular expression `\\.[^.]*`).
    * 
    * @requires \c flt must not contain any forward or backward slash. */
  void add_file_filter             (std::string const& flt, bool const positive = true);

  /** @brief Add the input method name to the filter. */
  void add_method_filter           (std::string const& flt, bool const positive = true);

  /** @brief Add the input path and every possible subpath of it to the filter */
  void add_recursive_path_filter   (std::string const& flt, bool const positive = true);

 /** @brief Add the input path and none of its subpaths to the filter 
   * @note It is assumed that folders are separated with a single forward or backward slash, and neither one of these two characters can
   *       appear in name of files or folders. */
  void add_nonrecursive_path_filter(std::string const& flt, bool const positive = true);

  /** @brief Add the input regular expression to the filter */
  void add_regex_filter            (std::string const& flt, bool const positive = true);

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /** @brief Remove the filter that has been added last. 
    * @requires @ref is_empty to return \c false. */
  void remove_last_filter();

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  /** @brief Set level of the filter. 
    * @return \c level (current value after the update) 
    * @note Having a non-void return type is to make it possible to easily call this function at the namespace level. */
  char unsigned set_level(char unsigned const level) { return this->level = level; }

  /** @brief Get the current level of the filter */
  char unsigned get_level() const { return level; }

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

  /** @brief Load filters from file. 
    * @return \c true iff file is loaded successfully. */
  bool load_from_file(std::string const& filename);

  /** @brief Same as @ref load_from_file, except that content of \c str is read instead of a file. */
  bool load_from_string(std::string const& str);

  /** @brief Same as @ref load_from_file, except that content of \c in is read instead of a file. */
  bool load_from_stream(std::istream& in);

  //-----------------------------------------------------------------------------------------------------------------------------------------------------------

private:

  struct raii_temp_filter_remover {
    raii_temp_filter_remover(loc_filter& flt) : flt(flt) { }
    ~raii_temp_filter_remover() { flt.remove_last_filter(); }
  private:
    loc_filter& flt; };

public:

  /** @brief Same as the same function with no \c _temp in its name. However, the filter will be removed when the returned object is destroyed. 
    * @note \c this must remain a valid (pointer to the) object while the return object is alive. */
  auto add_temp_file_filter             (std::string const& flt, bool const positive = true) { add_file_filter             (flt,positive); return raii_temp_filter_remover(*this); }
  auto add_temp_method_filter           (std::string const& flt, bool const positive = true) { add_method_filter           (flt,positive); return raii_temp_filter_remover(*this); } ///< Similar as @ref add_temp_file_filter
  auto add_temp_recursive_path_filter   (std::string const& flt, bool const positive = true) { add_recursive_path_filter   (flt,positive); return raii_temp_filter_remover(*this); } ///< Similar as @ref add_temp_file_filter
  auto add_temp_nonrecursive_path_filter(std::string const& flt, bool const positive = true) { add_nonrecursive_path_filter(flt,positive); return raii_temp_filter_remover(*this); } ///< Similar as @ref add_temp_file_filter
  auto add_temp_regex_filter            (std::string const& flt, bool const positive = true) { add_regex_filter            (flt,positive); return raii_temp_filter_remover(*this); } ///< Similar as @ref add_temp_file_filter

private:
  stack_t stack{};  

  /** @brief Current level, also known as severity. 
    * @note Initial value is same as \c  CMN_LOG_LEVEL_TRACE  
    * @note Strictly speaking, <em>level</em> does not really match with <em>location</em> filter. 
    *       We decided to put it here to prevent creation of another type. */
  char unsigned level = 0;
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
} // namespace cmn::detail
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif // __CMN__LOC_FILTER_HPP__
