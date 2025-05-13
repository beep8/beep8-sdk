/**
 *  @file tokenizer.h
 *  @brief Header file for the tokenizer module.
 *
 *  This file contains the declarations for the tokenizer class and supporting structures
 *  used to parse and handle comma-separated equations.
 *
 *  The tokenizer module allows for defining and retrieving key-value pairs from a string of equations.
 *  Each equation is in the format of "key=value" and multiple equations can be separated by commas.
 *  Additionally, macros can be used to replace placeholders in the equations.
 *
 *  Example usage:
 *  @code
 *  CTokenizer tokenizer("xx = +123 ; yy = -3 ; zz = -987 ; str = test_string");
 *  const char* fn = "CTestTokenizer";
 *
 *  _ASSERT(tokenizer.GetNumber("xx") == 123, fn);
 *  _ASSERT(tokenizer.GetNumber("yy") == -3, fn);
 *  _ASSERT(tokenizer.GetNumber("zz") == -987, fn);
 *  _ASSERT(tokenizer.GetNumber("str") == 0, fn);
 *
 *  _ASSERT(tokenizer.GetString("xx") == "+123", fn);
 *  _ASSERT(tokenizer.GetString("yy") == "-3", fn);
 *  _ASSERT(tokenizer.GetString("zz") == "-987", fn);
 *  _ASSERT(tokenizer.GetString("str") == "test_string", fn);
 *  @endcode
 *
 *  @author Your Name
 *  @date 2024
 */
#include <cstr.h>
#include <vector>
#include <map>

/**
 * @struct Macro
 * @brief Structure representing a macro with a key-value pair.
 */
struct Macro{
  const char* _key;  ///< The key of the macro.
  const char* _data; ///< The value associated with the macro key.
};

/**
 * @class MacroDict
 * @brief A dictionary for storing and retrieving macros.
 */
struct MacroDict {
  std::map< str8 , const char* , cmp_str > _dict; ///< Internal storage for macros.
  
  /**
   * @brief Dump all macros to the console.
   */
  void DumpAll() const;

  /**
   * @brief Get the value associated with a macro key.
   * 
   * @param key The key of the macro.
   * @return The value associated with the key.
   */
  const char* Get(const char* key) const;

  /**
   * @brief Constructor to initialize the dictionary with macros.
   * 
   * @param num_of_macro The number of macros to initialize.
   * @param macros An array of Macro structures.
   */
  MacroDict(size_t num_of_macro, const Macro* macros);
};

/**
 * @class CFormula
 * @brief Class representing a formula with a left-hand side (LHS) and a right-hand side (RHS).
 */
class CFormula{
public:
  str16 _lhs; ///< The left-hand side of the formula.
  str16 _rhs; ///< The right-hand side of the formula.
  
  /**
   * @brief Clear the contents of the formula.
   */
  void Clear(){
    _lhs.clear();
    _rhs.clear();
  }
};

/**
 * @class CTokenizer
 * 
 * @brief A tokenizer class for handling comma-separated equations.
 *
 * This class allows for handling multiple equations separated by a comma through its constructor CTokenizer::CTokenizer(). 
 * Right-hand side values of the equations can be obtained via CTokenizer::GetString() and CTokenizer::GetNumber().
 *
 * For instance, if a string like "x=test,y=345,z=something" is passed into the constructor, the values can be retrieved as follows:
 * - CTokenizer::GetString("x") will return "test"
 * - CTokenizer::GetNumber("y") will return 345
 * - CTokenizer::GetString("z") will return "something"
 */
class CTokenizer{
  enum State {
    STS_LHS, ///< State for parsing the left-hand side of an equation.
    STS_RHS  ///< State for parsing the right-hand side of an equation.
  };
  std::vector< CFormula > _formulas; ///< Storage for parsed formulas.
public:
  /**
   * @brief Get the right-hand side string associated with a left-hand side key.
   * 
   * @param lhs_ The left-hand side key.
   * @param def_ The default value if the key is not found.
   * @return The right-hand side string.
   */
  str16 GetString(const char* lhs_, const char* def_ = "");

  /**
   * @brief Get the right-hand side number associated with a left-hand side key.
   * 
   * @param lhs_ The left-hand side key.
   * @param def_ The default value if the key is not found.
   * @return The right-hand side number.
   */
  s32 GetNumber(const char* lhs_, s32 def_ = 0);

  /**
   * @brief Constructor to initialize the tokenizer with a string of equations and an optional macro dictionary.
   * 
   * @param sz The string containing comma-separated equations.
   * @param dict_ The optional macro dictionary.
   */
  CTokenizer(const char* sz, const MacroDict* dict_ = nullptr);
};
