/**
 * @file shell.h
 * @brief This module provides a simple command line interface (CLI) for running specific commands within a program.
 * 
 * The `shell` module allows users to execute commands in a CLI environment, useful for debugging and tool development.
 * Users can register new commands, execute them, display help for commands, maintain a history of commands, and run shell scripts.
 * 
 * This module is designed for use in tool and debug environments. It is not intended for use in actual games, as there is no serial console in such scenarios.
 * The primary purpose of this module is for debugging, and its usage is optional.
 * 
 * Example usage:
 * @code
 * #include <iostream>
 * #include <memory>
 * #include <string>
 * #include <vector>
 * #include "shell.h"
 * 
 * // Custom command class
 * class CMyShellCmd : public CShellCmd {
 * public:
 *     CMyShellCmd() : CShellCmd("mycmd") {}
 * 
 *     int vOnMain(const std::vector<str16>& tokens) override {
 *         if (ShowHelp(tokens, "Usage: mycmd [options]\n")) {
 *             return 0;
 *         }
 *         // Command execution logic here
 *         std::cout << "My Command Executed with tokens: ";
 *         for (const auto& token : tokens) {
 *             std::cout << token.c_str() << " ";
 *         }
 *         std::cout << std::endl;
 *         return 0;
 *     }
 * };
 * 
 * int main() {
 *     CShellObj shell;
 * 
 *     // Register custom command
 *     shell.Register(new CMyShellCmd());
 * 
 *     // Execute shell script
 *     auto script = std::make_shared<std::string>("mycmd arg1 arg2\nhelp\nclear\n");
 *     shell.ExecShellScript(script);
 * 
 *     // Simulate command input
 *     std::vector<u32> inputs = {'m', 'y', 'c', 'm', 'd', ' ', 'a', 'r', 'g', '1', ' ', 'a', 'r', 'g', '2', B8_HIF_KB_CODE_ENTER};
 *     for (u32 input : inputs) {
 *         shell.Input(input);
 *     }
 * 
 *     return 0;
 * }
 * @endcode
 */
#pragma once
#include <memory>
#include <stdio.h>
#include <cobj.h>
#include <cstr.h>
#include <vector>
#include <string>
class CShellObj;
class CShellCmd{
  friend  class CShellObj;
  str16 _name;
  /**
   * @brief This method is called to execute the main functionality of the command.
   * @param tokens The command line arguments passed to the command.
   * @return The return value indicates the success or failure of the command.
   */
  virtual int vOnMain( const std::vector< str16 >& tokens )=0;
protected:
  CShellObj* _pShellObj = nullptr;
  /**
   * @brief Displays help information for the command.
   * @param tokens The command line arguments passed to the command.
   * @param help The help message to display.
   * @return Returns true if help was displayed, false otherwise.
   */
  bool  ShowHelp( const std::vector< str16 >& tokens , const char* help );
public:
  /**
   * @brief Executes the command with the provided tokens.
   * @param tokens The command line arguments passed to the command.
   * @return The return value indicates the success or failure of the command.
   */
  int   Main( const std::vector< str16 >& tokens ){
    return  vOnMain( tokens );
  }
  /**
   * @brief Constructor for CShellCmd.
   * @param name_ The name of the command.
   */
  CShellCmd( const char* name_ );
  virtual ~CShellCmd(){}
};

class CHelpShellCmd;
class ImplCShellObj;
class CShellObj : public CObj {
  friend  class CHelpShellCmd;
  ImplCShellObj* _impl;
  bool  _no_history_registration = false;
  void  vOnStep() override;
  int   _Parse();
  void  _SetLineBuff( const char* sz );
public:
  /**
   * @brief Displays the command prompt.
   */
  void  Prompt();
  /**
   * @brief Retrieves a list of available commands.
   * @return A vector of available command names.
   */
  std::vector< str16 >  GetAvailableCommands();
  /**
   * @brief Registers a new command with the shell.
   * @param cmd_ The command to register.
   */
  void  Register( CShellCmd* cmd_ );
  /**
   * @brief Processes an input keycode.
   * @param keycode_ The keycode to process.
   */
  void  Input( u32 keycode_ );
  /**
   * @brief Executes a shell script.
   * @param script_ The script to execute.
   */
  void  ExecShellScript( std::shared_ptr< std::string > script_ );
  CShellObj();
  ~CShellObj();
};
