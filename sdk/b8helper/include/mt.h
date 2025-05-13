/**
 * @file mt.h
 * @brief Header file for the Mersenne Twister random number generator.
 *
 * This header file declares functions for initializing and generating random numbers
 * using the Mersenne Twister algorithm (MT19937). The implementation is based on
 * the original code by Takuji Nishimura and Makoto Matsumoto.
 *
 * The Mersenne Twister is a pseudorandom number generator that is widely used
 * due to its high period (2^19937-1) and good statistical properties.
 * 
 * This implementation is not thread-safe. For multithreaded applications, 
 * ensure that each thread has its own instance of the Mersenne Twister state.
 *
 * ### Usage
 * 
 * Before generating random numbers, initialize the state using either `init_genrand` or `init_by_array`.
 * 
 * ### Example
 * @code
 * #include <mt.h>
 * #include <stdio.h>
 *
 * int main() {
 *     init_genrand(1234); // Initialize with a seed
 *     
 *     for (int i = 0; i < 10; ++i) {
 *         printf("%u\n", genrand_int32()); // Generate random numbers
 *     }
 *     
 *     return 0;
 * }
 * @endcode
 *
 * @version 1.0
 * @date 2024
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief Initializes the state of the Mersenne Twister with a seed.
 *
 * This function initializes the internal state array with a seed.
 *
 * @param s The seed value.
 */
extern void init_genrand(unsigned long s);

/**
 * @brief Initializes the state of the Mersenne Twister with an array.
 *
 * This function initializes the internal state array using an array of seeds.
 *
 * @param init_key The array of seed values.
 * @param key_length The length of the seed array.
 */
extern void init_by_array(unsigned long init_key[], int key_length);

/**
 * @brief Generates a random number on [0, 0xffffffff] interval.
 *
 * This function generates a 32-bit random number in the range [0, 0xffffffff].
 *
 * @return A 32-bit unsigned random number.
 */
extern uint32_t genrand_int32(void);

/**
 * @brief Generates a random number on [0, 0x7fffffff] interval.
 *
 * This function generates a 31-bit random number in the range [0, 0x7fffffff].
 *
 * @return A 31-bit signed random number.
 */
extern int32_t genrand_int31(void);

/**
 * @brief Generates a random number in the specified range [min_, max_].
 *
 * This function generates a random number in the inclusive range [min_, max_].
 *
 * @param min_ The minimum value of the range.
 * @param max_ The maximum value of the range.
 * @return A random number in the specified range.
 */
extern int32_t genrand_min_max(int min_, int max_);

#ifdef __cplusplus
}
#endif
