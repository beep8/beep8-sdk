/**
 * @file irq.h
 * @brief Interrupt Request (IRQ) definitions for BEEP-8 system.
 * 
 * This file contains definitions related to Interrupt Requests (IRQs) for the BEEP-8 system.
 * It defines the number of interrupts and their respective identifiers for handling various
 * interrupt sources.
 * 
 * The BEEP-8 supports a maximum of 32 interrupts. If the number of interrupts exceeds
 * this limit, a compilation error will be triggered.
 * 
 * Interrupt sources include timers, vertical blanking, audio processing units, and various
 * other system events.
 * 
 * For more detailed information, please refer to the BEEP-8 data sheet.
 */
#pragma once
#ifdef  __cplusplus
extern  "C" {
#endif

#define B8_IRQ_NUM_OF_INTERRUPTS (32)

#if B8_IRQ_NUM_OF_INTERRUPTS > 32
#error Too many interrupts.
#endif

#define B8_IRQ_TMR0 (1)
#define B8_IRQ_TMR1 (2)
#define B8_IRQ_TMR2 (3)
#define B8_IRQ_TMR3 (4)
#define B8_IRQ_VBLK (5)
#define B8_IRQ_APUS (6)
#define B8_IRQ_UNDF (7)
#define B8_IRQ_DABT (8)

#ifdef  __cplusplus
}
#endif