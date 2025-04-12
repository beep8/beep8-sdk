/**
 * @file tmr.h
 * @brief Timer functions for the BEEP-8 system.
 * 
 * This file provides function prototypes for timer setup and wait operations
 * in the BEEP-8 system. It includes functions to configure and wait for timer
 * interrupts.
 * 
 * Functions provided:
 * - `b8TmrSetup`: Set up a timer with a specified cycle value
 * - `b8TmrWait`: Wait for a timer interrupt
 * 
 * Example usage (prints 10 characteres every second):
 * @code
 * #define TIMER_CH (2)
 * #define TIMER_HZ (10)
 * 
 * int main(void){
 *   b8TmrSetup(TIMER_CH, (b8SysGetCpuClock() / TIMER_HZ) >> 8);
 *   while(1){
 *     b8TmrWait(TIMER_CH);
 *     printf("T");
 *   }
 *   return 0;
 * }
 * @endcode
 * 
 * For more detailed information, please refer to the BEEP-8 data sheet.
 */


#pragma once

#ifdef  __cplusplus
extern  "C" {
#endif

/**
 * @brief Set up a timer with a specified cycle value.
 * 
 * This function configures the specified timer channel with the given cycle value.
 * It sets the timer mode to periodic, initializes the counter and period registers,
 * and enables the timer. Additionally, it sets up an IRQ wait handler for the timer.
 * 
 * @param tmr_ch The timer channel to set up.
 * @param cycval The cycle value for the timer.
 * @return 0 on success; an error code on failure.
 */
extern int b8TmrSetup(u32 tmr_ch, u32 cycval);

/**
 * @brief Wait for a timer interrupt.
 * 
 * This function waits for an interrupt from the specified timer channel. It blocks
 * until the timer interrupt occurs.
 * 
 * @param tmr_ch The timer channel to wait for.
 * @return 0 on success; an error code on failure.
 */
extern int b8TmrWait(u32 tmr_ch);

#ifdef  __cplusplus
}
#endif
