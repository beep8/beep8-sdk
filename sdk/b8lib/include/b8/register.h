/**
 * @file register.h
 * @brief Peripheral register map definitions for the BEEP-8 system.
 * 
 * This file consolidates the register maps for various peripherals supported by the BEEP-8 system,
 * including FIFO, SCI, INF, and TMR (Timer). It provides macros for accessing and manipulating
 * the registers of these peripherals.
 * 
 * The defined macros allow easy access to the registers through volatile pointers, ensuring
 * proper memory-mapped I/O operations. Each peripheral is assigned a specific base address,
 * and the macros handle the offset calculations for individual register access.
 * 
 * For more detailed information, please refer to the BEEP-8 data sheet.
 */
#pragma once

#ifdef  __cplusplus
extern  "C" {
#endif

#include <stdint.h>

#define B8_DISABLE  (0)
#define B8_ENABLE   (1)

#define _B8_REG(addr)      (*(volatile u32* )(uintptr_t)(addr))
#define _B8_REG_U8(addr)   (*(volatile u8* )(uintptr_t)(addr))
#define _B8_REG_U16(addr)  (*(volatile u16* )(uintptr_t)(addr))

// int
#define B8_FIFO_INT_VBLANK_ADDR  (0xffffb000)
#define B8_FIFO_INT_VBLANK_RX    _B8_REG( B8_FIFO_INT_VBLANK_ADDR + 0 )
#define B8_FIFO_INT_VBLANK_TX    _B8_REG( B8_FIFO_INT_VBLANK_ADDR + 4 )
#define B8_FIFO_INT_VBLANK_LEN   _B8_REG( B8_FIFO_INT_VBLANK_ADDR + 8 )
#define B8_FIFO_INT_VBLANK_CLEAR _B8_REG( B8_FIFO_INT_VBLANK_ADDR + 12 )

// sci
#define B8_FIFO_SCI_ADDR (0xffffc000)

#define B8_FIFO_SCI_TX(n)      _B8_REG_U8( B8_FIFO_SCI_ADDR  + 64*(n) + 32*0 + 4 )
#define B8_FIFO_SCI_TX_LEN(n)  _B8_REG_U8( B8_FIFO_SCI_ADDR  + 64*(n) + 32*0 + 8 )

#define B8_FIFO_SCI_RX(n)      _B8_REG_U8( B8_FIFO_SCI_ADDR  + 64*(n) + 32*1 + 0 )
#define B8_FIFO_SCI_RX_LEN(n)  _B8_REG_U16( B8_FIFO_SCI_ADDR + 64*(n) + 32*1 + 8 )

// inf
#define B8_INF_ADDR     (0xffff2000)
#define B8_INF_CPUCLK   _B8_REG( B8_INF_ADDR + 0x00 )
#define B8_INF_CAL_L    _B8_REG( B8_INF_ADDR + 0x10 )
#define B8_INF_CAL_H    _B8_REG( B8_INF_ADDR + 0x14 )

// tmr
#define B8_TMR_NUM  (3)
#define B8_TMR_ADDR(n)  (0xffff3000 + ((n)<<5))
#define B8_TMR_CTRL(n)  _B8_REG( B8_TMR_ADDR(n) + 0x0 )

#define B8_TMR_MODE(n)  _B8_REG( B8_TMR_ADDR(n) + 0x4 )
#define B8_TMR_MODE_PERIODIC  (0)
#define B8_TMR_MODE_ONESHOT   (1)

#define B8_TMR_PER(n)   _B8_REG( B8_TMR_ADDR(n) + 0x8 )
#define B8_TMR_CNT(n)   _B8_REG( B8_TMR_ADDR(n) + 0xc )

#ifdef  __cplusplus
}
#endif