/**
 * @file errno.h
 * @brief Error handling functions and definitions for the BEEP-8 system.
 * 
 * This file provides definitions and function prototypes for error handling in the BEEP-8 system.
 * It includes an enumeration of error codes and functions to set and get the current error number.
 * 
 * The error handling mechanism is designed to be POSIX-compliant.
 * 
 * For more detailed information, please refer to the BEEP-8 data sheet.
 */

#pragma once

#ifdef  __cplusplus
extern  "C" {
#endif

#include <stdint.h>

/**
 * @brief Enumeration of error codes.
 * 
 * This enumeration defines various error codes used in the BEEP-8 system.
 * - `B8_OK`: No error
 * - `B8_NO_ERROR`: No error (alias for `B8_OK`)
 * - `B8_INVALID_VALUE`: Invalid value error
 * - `B8_INVALID_ENUM`: Invalid enumeration error
 */
typedef enum  {
  B8_OK = 0,
  B8_NO_ERROR = 0,
  B8_INVALID_VALUE = 2000,
  B8_INVALID_ENUM,
} b8Err;

/**
 * @brief Set the current error code.
 * 
 * This function sets the current error code to the specified value.
 * The error code follows the UNIX convention: 0 indicates no error, and positive values represent specific error conditions.
 * Error codes are based on those defined in sys/errno.h.
 * 
 * The error code is maintained per thread, meaning this function sets the error code for the thread that invokes it.
 * 
 * If the function fails to set the error code, it returns -1. Otherwise, it returns 0 to indicate success.
 * 
 * @param errcode The error code to set (0 for no error, positive for error).
 * @return 0 on success, -1 on failure.
 */
extern int set_errno(int errcode);

/**
 * @brief Retrieve the current error code.
 * 
 * This function returns the current error code.
 * A value of 0 indicates no error, while a positive value signifies an error, following UNIX conventions.
 * Error codes are based on those defined in sys/errno.h.
 * 
 * The error code is maintained per thread, and this function retrieves the error code for the thread that invokes it.
 * 
 * @return The current error code (0 for no error, positive for error).
 */
extern int get_errno(void);


#ifdef  __cplusplus
}
#endif
