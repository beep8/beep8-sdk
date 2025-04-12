/**
 * @file fcast.h
 * @brief Module for performing forced type casts between different data types.
 * 
 * This module provides functions for forced type casting, enabling conversions
 * between various data types such as pointers and integers. It is primarily used
 * for scenarios where normal casting is not feasible or safe, offering utility
 * functions to handle these conversions.
 * 
 * ### Overview of Features
 * 
 * - **ForceCast_u32(void* a)**: Converts any pointer type to `u32`.
 * - **ForceCast_pu8(u32 a)**: Converts a `u32` value to `u8*` pointer.
 * - **ForceCast_u32(u8* a)**: Converts a `u8*` pointer to `u32`.
 * - **ForceCast_u32(char* a)**: Converts a `char*` pointer to `u32`.
 * - **ForceCast_u32(u32* a)**: Converts a `u32*` pointer to `u32`.
 * - **ForceCast_u32(s16 a)**: Converts a `s16` value to `u32`.
 * - **ForceCast_s16(u32 a)**: Converts a `u32` value to `s16`.
 * - **ForceCast_s32(u32 a)**: Converts a `u32` value to `s32`.
 * 
 * ### Detailed Description
 * 
 * #### ForceCast Struct
 * 
 * This module uses unions internally to reinterpret data between different types.
 * Unions allow the same memory space to be treated as different types.
 * 
 * - **ForceCast**: A union for handling 32-bit data types, allowing conversion
 *   between `u32`, `u32*`, `u8*`, `char*`, `s32`, and `void*`.
 * - **ForceCast16**: A union for handling 16-bit data types, allowing conversion
 *   between `u16` and `s16`.
 * 
 * #### Safety Check
 * 
 * All conversion functions call the `SAFE_ASSERT` function at the beginning to
 * ensure that the size of a pointer is 4 bytes, guaranteeing that the target system
 * is a 32-bit environment.
 * 
 * ### Usage Example
 * 
 * Here is an example of how to use this module:
 * 
 * @code
 * #include "fcast.h"
 * #include <stdio.h>
 * 
 * int main() {
 *     void* ptr = (void*)0x12345678;
 *     u32 value = 0x12345678;
 *     u8* byte_ptr;
 *     s16 short_value;
 *     s32 int_value;
 * 
 *     // Convert void* to u32
 *     u32 u32_from_ptr = ForceCast_u32(ptr);
 *     printf("u32 from void*: %u\n", u32_from_ptr);
 * 
 *     // Convert u32 to u8*
 *     byte_ptr = ForceCast_pu8(value);
 *     printf("u8* from u32: %p\n", (void*)byte_ptr);
 * 
 *     // Convert s16 to u32
 *     short_value = -12345;
 *     u32 u32_from_s16 = ForceCast_u32(short_value);
 *     printf("u32 from s16: %u\n", u32_from_s16);
 * 
 *     // Convert u32 to s16
 *     short_value = ForceCast_s16(value);
 *     printf("s16 from u32: %d\n", short_value);
 * 
 *     // Convert u32 to s32
 *     int_value = ForceCast_s32(value);
 *     printf("s32 from u32: %d\n", int_value);
 * 
 *     return 0;
 * }
 * @endcode
 * 
 * This example demonstrates the conversion of `void*` to `u32`, `u32` to `u8*`,
 * and `s16` to `u32`, showcasing how to use the provided utility functions for
 * safe type conversions.
 * 
 * @version 1.0
 * @date 2024
 * 
 * @note This module depends on the b8/type.h header for type definitions.
 */

#pragma once
#include <b8/type.h>

extern	u8*		ForceCast_pu8( u32 a );
extern	u32		ForceCast_u32( u8* a );
extern	u32		ForceCast_u32( u32* a );
extern	u32		ForceCast_u32( s16 a );
extern	u32		ForceCast_u32( char* a );
extern	u32		ForceCast_u32( void* a );
extern	s16		ForceCast_s16( u32 a );
extern	s32		ForceCast_s32( u32 a );
