/**
 * @file sched.h
 * @brief Minimal POSIX-compliant scheduling definitions for the BEEP-8 system.
 * 
 * This file provides minimal POSIX-compliant scheduling definitions and declarations 
 * necessary for the BEEP-8 system. It defines several scheduling policies and a basic 
 * `sched_param` structure for setting thread priorities.
 * 
 * The scheduling policies supported include:
 * - `SCHED_FIFO`: FIFO priority scheduling policy
 * - `SCHED_RR`: Round robin scheduling policy
 * - `SCHED_SPORADIC`: Sporadic scheduling policy
 * - `SCHED_IRQ`: IRQ handler scheduling policy
 * - `SCHED_OTHER`: Not supported
 * 
 * This file also provides a function prototype for retrieving the maximum priority 
 * value for a given scheduling policy.
 * 
 * Typically, users do not need to use this header directly. It is intended for internal 
 * use within the BEEP-8 system to handle scheduling-related operations.
 */
#pragma once

#ifdef  __cplusplus
extern  "C" {
#endif

// POSIX scheduling policies
#undef  SCHED_FIFO
#define SCHED_FIFO                1  // FIFO priority scheduling policy

#undef  SCHED_RR
#define SCHED_RR                  2  // Round robin scheduling policy

#undef  SCHED_SPORADIC
#define SCHED_SPORADIC            3  // Sporadic scheduling policy

#undef  SCHED_IRQ
#define SCHED_IRQ                 4  // Irq handler scheduling policy

#undef  SCHED_OTHER
#define SCHED_OTHER               5  // Not supported

#undef sched_param
#define sched_param b8_sched_param
struct b8_sched_param{
  int sched_priority; // Base thread priority
};

extern  int sched_get_priority_max(int policy);

#ifdef  __cplusplus
}
#endif