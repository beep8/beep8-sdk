/**
 * @file argparse.h
 * @brief Command-line argument parser for C++.
 * 
 * This header file defines the structures and classes needed for parsing
 * command-line arguments in C++. It includes functionality to define 
 * arguments, parse them, and retrieve their values. The parser supports 
 * both long and short argument names, as well as different types of actions 
 * such as storing boolean values and displaying help messages.
 * 
 * Example usage:
 * @code
 * #include "argparse.h"
 * 
 * int main(int argc, char* argv[]) {
 *     argparse::ArgumentParser parser;
 *     parser.AddArgument("--verbose", "-v", "Enable verbose mode", argparse::Argument::store_true);
 *     parser.AddArgument("--output", "-o", "Output file name");
 *     auto result = parser.ParseArgs(std::vector<std::string>(argv + 1, argv + argc));
 *     
 *     if (result != argparse::ArgumentParser::OK) {
 *         return result;
 *     }
 *     
 *     bool verbose = parser.GetVarInt("--verbose", 0);
 *     std::string output = parser.GetVarStr("--output", "default.txt");
 *     
 *     // Your program logic here
 * }
 * @endcode
 * 
 * This module is primarily designed for tool development purposes rather 
 * than game development. It is not essential for game creation but is 
 * available for use if needed.
 * 
 * @version 1.0
 * @date 2022
 * @author ZooK
 * 
 * @copyright MIT License
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once
#include <string>
#include <vector>
#include <map>

namespace argparse {

/**
 * @brief Structure representing a command line argument.
 */
struct Argument {
  std::string _name;        ///< Full name of the argument (e.g., --help)
  std::string _short_alias; ///< Short alias for the argument (e.g., -h)
  std::string _help;        ///< Help description for the argument

  /**
   * @brief Enumeration of possible actions for an argument.
   */
  enum Action {
    do_nothing,    ///< No action
    store_false,   ///< Store false (0)
    store_true,    ///< Store true (1)
    show_help,     ///< Show help message
  };

  Action _action; ///< Action associated with the argument

  /**
   * @brief Display help information for the argument.
   */
  void ShowHelp() const;
};

/**
 * @brief Structure representing a variable associated with an argument.
 */
struct Var {
  int _val = 0;        ///< Integer value of the variable
  std::string _str = ""; ///< String value of the variable

  /**
   * @brief Clear the variable values.
   */
  void Clear() {
    _val = 0;
    _str.clear();
  }
};

/**
 * @brief Class for parsing command line arguments.
 */
class ArgumentParser {
  std::vector<Argument> _args; ///< List of arguments
  std::map<std::string, Var> _vars; ///< Map of variables associated with arguments

public:
  /**
   * @brief Enumeration of possible results from parsing arguments.
   */
  enum Result {
    OK = 0,           ///< Parsing successful
    SHOWED_HELP = -128, ///< Help message was shown
    UNKNOWN_FLAG,     ///< Unknown flag encountered
    BAD_PARAM,        ///< Bad parameter provided
    UNKNOWN_VAR,      ///< Unknown variable encountered
  };

  /**
   * @brief Get the integer value of a variable.
   * @param name_ Name of the variable
   * @param if_not_found_ Value to return if the variable is not found
   * @return Integer value of the variable or if_not_found_ if not found
   */
  int GetVarInt(std::string name_, int if_not_found_);

  /**
   * @brief Get the string value of a variable.
   * @param name_ Name of the variable
   * @param if_not_found_ Value to return if the variable is not found
   * @return String value of the variable or if_not_found_ if not found
   */
  std::string GetVarStr(std::string name_, std::string if_not_found_);

private:
  /**
   * @brief Search for an argument by its name.
   * @param name_ Name of the argument
   * @param dest_ Destination to store the found argument
   * @return True if found, false otherwise
   */
  bool _SearchName(std::string name_, Argument& dest_);

  /**
   * @brief Search for an argument by its short alias.
   * @param short_alias_ Short alias of the argument
   * @param dest_ Destination to store the found argument
   * @return True if found, false otherwise
   */
  bool _SearchShortAlias(std::string short_alias_, Argument& dest_);

private:
  std::string _cur_name; ///< Current argument name being processed

  /**
   * @brief Perform an action for an argument.
   * @param dest Argument for which the action is performed
   * @param it_ Current argument string
   * @return Result of the action
   */
  Result _Action(const Argument& dest, const std::string& it_);

public:
  /**
   * @brief Parse command line arguments.
   * @param args Vector of argument strings
   * @return Result of parsing
   */
  Result ParseArgs(const std::vector<std::string>& args);

  bool _showed_help = false; ///< Flag indicating whether help was shown

  /**
   * @brief Display help information for all arguments.
   */
  void ShowHelp();

  /**
   * @brief Add an argument to the parser.
   * @param name_ Full name of the argument
   * @param short_alias_ Short alias of the argument
   * @param help_ Help description for the argument
   * @param action_ Action associated with the argument (default is store_true)
   * @return Result of adding the argument
   */
  int AddArgument(
    std::string name_,
    std::string short_alias_,
    std::string help_,
    Argument::Action action_ = Argument::store_true
  );

  /**
   * @brief Constructor for ArgumentParser.
   */
  ArgumentParser();
};

} // namespace argparse
