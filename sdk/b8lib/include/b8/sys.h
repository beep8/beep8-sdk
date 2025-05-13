/**
 * @file sys.h
 * @brief System utility functions for the BEEP-8 system.
 * 
 * This file provides various system utility functions and macros for the BEEP-8 system.
 * It includes functions for system halt, reset, outputting strings and numbers, 
 * and handling interrupts.
 * 
 * Functions provided:
 * - `b8SysHalt`: Halt the system
 * - `b8SysReset`: Reset the system
 * - `b8SysPuts`: Output a string
 * - `b8SysPutHex`: Output a hexadecimal number
 * - `b8SysPutNum`: Output a decimal number
 * - `b8SysPutCR`: Output a carriage return
 * - `b8SysGetCpuClock`: Get the CPU clock speed
 * - `b8SysSetupIrqWait`: Set up an IRQ wait handler
 * - `b8SysIrqWait`: Wait for an IRQ
 * 
 * These functions are intended for use under special conditions, such as in the bootloader,
 * operating system, or for handling exceptional halts. They should not be used in regular 
 * application code.
 * 
 */
#pragma once

#ifdef  __cplusplus
extern  "C" {
#endif

#include <b8/type.h>

/**
 * @brief Halt the system.
 * 
 * This function outputs a halt message and then halts the system.
 */
extern void b8SysHalt(void);

/**
 * @brief Reset the system.
 * 
 * This function resets the system by calling the `b8rst` function.
 */
extern void b8SysReset(void);

/**
 * @brief Output a string.
 * 
 * This function outputs the given string to the system console.
 * 
 * @param str The string to output.
 */
extern void b8SysPuts(const char* str);

/**
 * @brief Output a hexadecimal number.
 * 
 * This function outputs the given 32-bit hexadecimal number to the system console.
 * 
 * @param data The hexadecimal number to output.
 */
extern void b8SysPutHex(u32 data);

/**
 * @brief Output a decimal number.
 * 
 * This function outputs the given 32-bit decimal number to the system console.
 * 
 * @param data The decimal number to output.
 */
extern void b8SysPutNum(s32 data);

/**
 * @brief Output a carriage return.
 * 
 * This function outputs a newline character to the system console.
 */
extern void b8SysPutCR(void);

/**
 * @brief Get the CPU clock speed.
 * 
 * This function returns the current CPU clock speed.
 * 
 * @return The CPU clock speed.
 */
extern u32 b8SysGetCpuClock(void);

/**
 * @brief Set up an IRQ wait handler.
 * 
 * This function sets up an IRQ wait handler for the specified IRQ.
 * 
 * @param irq The IRQ number to set up.
 * @return 0 on success; an error code on failure.
 */
extern int b8SysSetupIrqWait(u32 irq);

/**
 * @brief Wait for an IRQ.
 *
 * This function waits for the specified IRQ to occur. It does not clear any
 * pending IRQ events that occurred before this call. If you need to discard
 * pending events before waiting, see @see b8SysIrqClearAndWait.
 *
 * @param irq The IRQ number to wait for.
 * @return 0 on success; -1 on failure (errno is set accordingly).
 * @see b8SysIrqClearAndWait
 */
extern int b8SysIrqWait(u32 irq);

/**
 * @brief Clear pending IRQ events and wait for the next occurrence.
 *
 * This function drains any pending sem_post backlog for the specified IRQ,
 * then waits for the next IRQ event. Use this API when you want to ensure
 * that only the next interrupt will cause the wait to return.
 *
 * @param irq The IRQ number to clear and wait for.
 * @return 0 on success; -1 on failure (errno is set to EINVAL if irq is invalid).
 * @see b8SysIrqWait
 */
extern int b8SysIrqClearAndWait(u32 irq);

/**
 * @brief Assert macro for system checks.
 * 
 * This macro checks the given expression and if it evaluates to false,
 * it outputs an assertion message and halts the system.
 * 
 * @param expr_ The expression to evaluate.
 * @param comment_ A custom comment to include in the assertion message.
 */
#define B8_SYS_ASSERT(expr_, comment_) \
    do { \
        if (!(expr_)) { \
          b8SysPuts( "[B8_SYS_ASSERT]" );\
          b8SysPuts( comment_ );\
          b8SysPutCR();\
          asm("hlt");\
        } \
    } while(0)

#ifdef  __cplusplus
}
#endif
