/**
 * @file pic.h
 * @brief Programmable Interrupt Controller (PIC) definitions for the BEEP-8 system.
 * 
 * This file contains the definitions for the Programmable Interrupt Controller (PIC) in the BEEP-8 system.
 * It provides macros for accessing PIC registers, which are used to control and handle hardware interrupts.
 * 
 * The PIC is located at a specific memory address and includes registers for 
 * device control (DCR), interrupt acknowledge (IAR), and end of interrupt (EOIR).
 * 
 * For more detailed information, please refer to the BEEP-8 data sheet.
 */
#pragma once
#include <b8/type.h>

#define B8_PIC_ADDR (0xffff0000)
#define B8_PIC_DCR  _B8_REG(B8_PIC_ADDR + 0x00)
#define B8_PIC_IAR  _B8_REG(B8_PIC_ADDR + 0x04)
#define B8_PIC_EOIR _B8_REG(B8_PIC_ADDR + 0x08)
