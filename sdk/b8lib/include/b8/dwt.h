/**
 * @file dwt.h
 * @brief Data Watchpoint and Trace Unit (DWT) definitions.
 * 
 * This file contains definitions for the Data Watchpoint and Trace Unit (DWT) in an ARM architecture.
 * The DWT is a feature of ARM processors that provides watchpoint and trace functionalities for debugging.
 * 
 * For more detailed information, please refer to the BEEP-8 data sheet.
 */
#pragma once
#include <b8/type.h>

#define B8_DWT_ADDR     (0xffff1000)
#define B8_DWT_CTRL     _B8_REG(B8_DWT_ADDR + 0x00)
#define B8_DWT_CYCCNT   _B8_REG(B8_DWT_ADDR + 0x04)
