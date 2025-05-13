/**
 * @file clock.h
 * @brief Clock and time management functions for the BEEP-8 system.
 * 
 * This file provides definitions and function prototypes for clock and time management
 * in the BEEP-8 system. It includes macros for various clock IDs and functions for 
 * getting and setting clock time, as well as for suspending execution for a specified 
 * duration.
 * 
 * Macros provided:
 * - `CLOCK_REALTIME_COARSE`: Alias for `CLOCK_REALTIME`
 * - `CLOCK_MONOTONIC`: Monotonic clock, which cannot be set and represents monotonic time since some unspecified starting point
 * - `CLOCK_MONOTONIC_COARSE`: Coarse resolution version of `CLOCK_MONOTONIC`
 * - `CLOCK_MONOTONIC_RAW`: Raw hardware-based version of `CLOCK_MONOTONIC`
 * - `CLOCK_BOOTTIME`: Monotonic clock including time spent in suspend
 * - `CLOCK_PROCESS_CPUTIME_ID`: High-resolution per-process timer from the CPU
 * - `CLOCK_THREAD_CPUTIME_ID`: Thread-specific CPU-time clock
 * 
 * Functions provided:
 * - `clock_getres`: Get the resolution of the specified clock
 * - `clock_gettime`: Get the current time of the specified clock
 * - `clock_settime`: Set the current time of the specified clock
 * - `usleep`: Suspend execution for a specified number of microseconds
 * 
 * For more detailed information, please refer to the BEEP-8 data sheet.
 */

#pragma once

#ifdef  __cplusplus
extern  "C" {
#endif

#include <sys/types.h>

#define CLOCK_REALTIME_COARSE     CLOCK_REALTIME
#define CLOCK_MONOTONIC           (0x20)
#define CLOCK_MONOTONIC_COARSE    (CLOCK_MONOTONIC+1)
#define CLOCK_MONOTONIC_RAW       (CLOCK_MONOTONIC+2)
#define CLOCK_BOOTTIME            (CLOCK_MONOTONIC+3)
#define CLOCK_PROCESS_CPUTIME_ID  (0x30)
#define CLOCK_THREAD_CPUTIME_ID   (0x31)

/**
 * @brief Get the resolution of the specified clock.
 *        This function should not be called by users directly.
 * 
 * This function retrieves the resolution (precision) of the specified clock, identified
 * by `clk_id`. The resolution is stored in the `res` structure.
 * 
 * @param clk_id The identifier of the clock.
 * @param res A pointer to a `timespec` structure where the resolution will be stored.
 * @return 0 on success; -1 on error.
 */
extern int clock_getres(clockid_t clk_id, struct timespec *res);

/**
 * @brief Get the current time of the specified clock.
 *        This function should not be called by users directly.
 * 
 * This function retrieves the current time of the specified clock, identified by `clk_id`.
 * The current time is stored in the `tp` structure.
 * 
 * @param clk_id The identifier of the clock.
 * @param tp A pointer to a `timespec` structure where the current time will be stored.
 * @return 0 on success; -1 on error.
 */
extern int clock_gettime(clockid_t clk_id, struct timespec *tp);

/**
 * @brief Set the current time of the specified clock.
 *        This function should not be called by users directly.
 * 
 * This function sets the current time of the specified clock, identified by `clk_id`,
 * to the value specified in the `tp` structure.
 * 
 * @param clk_id The identifier of the clock.
 * @param tp A pointer to a `timespec` structure containing the new time value.
 * @return 0 on success; -1 on error.
 */
extern int clock_settime(clockid_t clk_id, const struct timespec *tp);

/**
 * @brief Suspend execution for a specified number of microseconds.
 *        This function could be called by users directly.
 * 
 * This function suspends the execution of the calling thread for at least the specified
 * number of microseconds.
 * 
 * @param useconds The number of microseconds to suspend execution.
 * @return 0 on success; -1 on error.
 */
extern int usleep(useconds_t useconds);

#ifdef  __cplusplus
}
#endif
