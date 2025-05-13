/**
 * @file beep8.h
 * @brief Includes all necessary headers for developing applications on the BEEP-8 system.
 *
 * This header file aggregates all the essential headers required for creating applications
 * for the BEEP-8 system. By including this single file, developers can access all the
 * necessary functionalities provided by the BEEP-8 SDK, simplifying the development process.
 *
 * The following headers are included:
 * - <stdio.h>: Standard input/output library
 * - <b8/type.h>: BEEP-8 type definitions
 * - <b8/register.h>: BEEP-8 register definitions
 * - <b8/assert.h>: BEEP-8 assertion macros
 * - <b8/sys.h>: BEEP-8 system functions
 * - <b8/ppu.h>: BEEP-8 PPU (Picture Processing Unit) interface
 * - <b8/apu.h>: BEEP-8 APU (Audio Processing Unit) interface
 * - <b8/hif.h>: BEEP-8 hardware interface functions
 * - <b8/irq.h>: BEEP-8 interrupt request handling
 * - <b8/pic.h>: BEEP-8 programmable interrupt controller functions
 * - <b8/dwt.h>: BEEP-8 data watch and trace unit interface
 * - <b8/tmr.h>: BEEP-8 timer functions
 * - <b8/os.h>: BEEP-8 operating system interface
 * - <b8/errno.h>: BEEP-8 error number definitions
 * - <b8/semaphore.h>: BEEP-8 semaphore functions
 * - <b8/pthread.h>: BEEP-8 pthread functions
 * - <b8/syscall.h>: BEEP-8 system call interface
 * - <b8/misc.h>: Miscellaneous BEEP-8 functions
 *
 * @note Ensure that this header is included at the beginning of your source files to access
 * all the functionalities of the BEEP-8 SDK.
 */
#pragma once
#include <stdio.h>
#include <b8/type.h>
#include <b8/register.h>
#include <b8/assert.h>
#include <b8/sys.h>
#include <b8/ppu.h>
#include <b8/apu.h>
#include <b8/hif.h>
#include <b8/irq.h>
#include <b8/pic.h>
#include <b8/dwt.h>
#include <b8/tmr.h>
#include <b8/os.h>
#include <b8/errno.h>
#include <b8/semaphore.h>
#include <b8/pthread.h>
#include <b8/syscall.h>
#include <b8/misc.h>