/**
 * @file misc.h
 * @brief Miscellaneous functions and macros for the BEEP-8 system.
 * 
 * This file aggregates various functions and macros that do not fit into 
 * specific categories. It includes ASCII character definitions, utility 
 * macros, and external function declarations for general-purpose use.
 * 
 * The ASCII definitions provide easy references to control characters 
 * and other common ASCII values. Utility macros such as `fileno` and 
 * `UNUSED` offer convenient ways to handle file descriptors and suppress 
 * unused variable warnings, respectively.
 * 
 * For more detailed information, please refer to the BEEP-8 data sheet.
 */
#pragma once

#ifdef  __cplusplus
extern  "C" {
#endif

#define ASCII_NUL     (0x00)  ///< Null character
#define ASCII_ETX     (0x03)  ///< End of text
#define ASCII_BEL     (0x07)  ///< Bell
#define ASCII_BS      (0x08)  ///< Backspace
#define ASCII_CTRL_H  (0x08)  ///< Control-H, same as backspace
#define ASCII_HT      (0x09)  ///< Horizontal tab
#define ASCII_LF      (0x0a)  ///< Line feed
#define ASCII_VT      (0x0b)  ///< Vertical tab
#define ASCII_FF      (0x0c)  ///< Form feed
#define ASCII_CR      (0x0d)  ///< Carriage return
#define ASCII_EOT     (0x04)  ///< End of transmission
#define ASCII_TAB     (0x09)  ///< Tab, same as horizontal tab
#define ASCII_ESC     (0x1b)  ///< Escape
#define ASCII_SPACE   (0x20)  ///< Space
#define ASCII_DEL     (0x7e)  ///< Delete

#define	fileno(p) ((p)->_file)

/**
 * @brief Performs a variety of control functions on devices.
 *
 * The `ioctl` function manipulates the underlying device parameters of 
 * special files. In general, `ioctl` can control hardware devices, file 
 * descriptors, and other resources using a variety of commands.
 *
 * @param fd The file descriptor referring to the device.
 * @param request The device-specific request code that specifies the 
 *        operation to perform.
 * @param ... Additional arguments needed for the request. The number and 
 *        type of these arguments depend on the request code.
 * 
 * @return On success, 0 is returned. On error, -1 is returned and `errno` 
 *         is set to indicate the error.
 *
 * @note The `ioctl` function is highly device-specific. Each device can 
 *       have its own set of request codes and corresponding arguments. 
 *       Refer to the device's documentation for details.
 *
 * @see open()
 * @see close()
 * @see read()
 * @see write()
 */
extern int  ioctl(int fd, int request, ...);

#define UNUSED(x) ((void)x)

#ifdef  __cplusplus
}
#endif