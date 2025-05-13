/**
 * @file trace.h
 * @brief Debug tracing macros for logging and monitoring program execution.
 * 
 * This header file provides macros for tracing and debugging purposes. 
 * The macros allow you to log function calls, variable values, and checkpoints 
 * in the code, which can help in diagnosing issues and understanding program flow.
 * 
 * The primary macros are:
 * - PASS(): Logs a simple message indicating the passage of code execution through this point.
 * - TRACE(x): Logs a message with the string representation of the variable or expression `x`.
 * - WATCH(x): Logs a message with the name and string representation of the variable `x`.
 * 
 * Example usage:
 * 
 * @code
 * void exampleFunction(int value) {
 *     PASS(); // Logs the passage through this function
 *     TRACE(value); // Logs the value of the variable `value`
 *     int result = value * 2;
 *     WATCH(result); // Logs the name and value of the variable `result`
 * }
 * @endcode
 */

#pragma once

#include <tostr.h>

/**
 * @brief Logs a simple passage message.
 * 
 * This macro logs a message indicating the passage of code execution 
 * through this point. It includes the file name, line number, and function name.
 */
#define PASS()    printf( "[PASS] %s(%d) %s()\n",__FILE__,__LINE__, __func__ );

/**
 * @brief Logs a trace message with a variable or expression.
 * 
 * This macro logs a message with the string representation of the variable or 
 * expression `x`. It includes the file name, line number, function name, and 
 * the value of `x`.
 * 
 * @param x The variable or expression to trace.
 */
#define TRACE(x)  printf( "[TRACE] %s(%d) %s() %s\n",__FILE__,__LINE__, __func__ , tostr((x)).c_str() );

/**
 * @brief Logs a watch message with the name and value of a variable.
 * 
 * This macro logs a message with the name and string representation of the 
 * variable `x`. It includes the file name, line number, function name, the 
 * name of `x`, and its value.
 * 
 * @param x The variable to watch.
 */
#define WATCH(x)  printf( "[WATCH] %s(%d) %s() %s = %s\n",__FILE__,__LINE__, __func__ ,#x, tostr((x)).c_str() );
