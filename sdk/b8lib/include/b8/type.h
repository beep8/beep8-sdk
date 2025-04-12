/**
 * @file types.h
 * @brief Type definitions for the BEEP-8 system.
 * 
 * This file provides type definitions used in the BEEP-8 system. These definitions 
 * include various fixed-width integer types, floating-point types, and size types 
 * to ensure consistency and portability across different platforms and compilers.
 * 
 * The defined types include:
 * - `u1`: Unsigned 1-byte integer
 * - `u8`: Unsigned 8-bit integer
 * - `u16`: Unsigned 16-bit integer
 * - `u32`: Unsigned 32-bit integer
 * - `u64`: Unsigned 64-bit integer
 * - `s8`: Signed 8-bit integer
 * - `s16`: Signed 16-bit integer
 * - `s32`: Signed 32-bit integer
 * - `s64`: Signed 64-bit integer
 * - `f32`: 32-bit floating point
 * - `f64`: 64-bit floating point
 * - `ssize_t`: Signed size type
 * 
 * For more detailed information, please refer to the BEEP-8 data sheet.
 */
#pragma once
#ifdef  __cplusplus
extern  "C" {
#endif
typedef unsigned long       u32;
typedef unsigned long long  u64;
typedef long long           s64;
typedef unsigned short      u16;
typedef unsigned char       u8;
typedef unsigned char       u1;
typedef int                 s32;
typedef short               s16;
typedef signed char         s8;
typedef float               f32;
typedef double              f64;
typedef s32                 ssize_t;
#ifdef  __cplusplus
}
#endif