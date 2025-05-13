/**
 * @file sublibc.h
 * @brief Utility functions for memory management and string operations.
 * 
 * This module provides a collection of helper functions for memory management 
 * and string operations in C++ programs. The functions include:
 * 
 * - `print_mallinfo`: Displays current dynamic memory allocation statistics.
 * - `mallocz`: Allocates a block of memory and initializes it to zero.
 * - `fs`: Formats a string with variable arguments similar to `printf`.
 * - `memsetz`: Zero-initializes a block of memory.
 * - `numof`: Returns the number of elements in a static array.
 * 
 * These functions are intended to simplify common tasks and improve code 
 * readability and maintainability.
 * 
 * Note: This module is not thread-safe and is intended for use in single-threaded 
 * environments or where thread-safety is managed externally.
 * 
 * Example usage:
 * @code
 * #include <sublibc.h>
 * #include <iostream>
 * 
 * int main() {
 *     // Allocate and zero-initialize memory
 *     size_t size = 100;
 *     void* ptr = mallocz(size);
 *     if (ptr != nullptr) {
 *         std::cout << "Memory allocated and zero-initialized." << std::endl;
 *         free(ptr);
 *     }
 * 
 *     // Print memory allocation information
 *     print_mallinfo();
 * 
 *     // Format a string
 *     std::string formatted = fs("Hello, %s! You have %d new messages.", "Alice", 5);
 *     std::cout << formatted << std::endl;
 * 
 *     // Get the number of elements in an array
 *     int arr[10];
 *     size_t arr_size = numof(arr);
 *     std::cout << "Array size: " << arr_size << std::endl;
 * 
 *     return 0;
 * }
 * @endcode
 * 
 * The functions provided by this module are not mandatory for use but are 
 * designed to aid in the development and debugging of applications by 
 * offering convenient utilities.
 * 
 * @note This module is intended for debugging purposes and its use is optional.
 */
#pragma once
#include <string.h>
#include <cstddef>
#include <cstdio>
#include <string>
#include <beep8.h>
#include <mt.h>

/**
 * @brief Retrieve the size of an array.
 * 
 * This template function returns the number of elements in a given array.
 * 
 * @tparam TYPE The type of the array elements.
 * @tparam SIZE The size of the array.
 * 
 * @param array The array whose size is to be determined.
 * 
 * @return The number of elements in the array.
 */
template < typename TYPE, std::size_t SIZE >
std::size_t numof(const TYPE (&)[SIZE]) {
    return SIZE;
}

/**
 * @brief Returns a wrapped reference to an element at index @p i in container @p c.
 *
 * This function accesses the element at \c i modulo the size of the container,
 * allowing cyclic (wrapping) indexing. If \c i is larger than \c c.size(),
 * it wraps around to the beginning.
 *
 * @tparam T The type of the container. It must provide:
 *           - a `size()` member function returning a `std::size_t` count of elements
 *           - an index operator \c operator[]
 * @param c  The container to be accessed.
 * @param i  The index to be wrapped. If `i >= c.size()`, the index wraps around
 *           so the final access is \c c[i % c.size()].
 * @return A const reference to the element at position `i % c.size()`.
 *
 * @note The behavior is undefined if \c c is empty (i.e., if `c.size() == 0`).
 *
 * @code
 *   // Example container for animation frames
 *   constexpr std::array<u8, 7> animFrames = {0,1,2,2,0,3,4};
 *
 *   // Example usage:
 *   // Suppose we want to index based on xPos shifted right by 1,
 *   // but in a cyclical manner.
 *   // We can retrieve the wrapped element like so:
 *   u8 frame = wrap_at(animFrames, x);
 * 
 * @endcode
 */
template <class T>
constexpr auto wrap_at(const T& c, std::size_t i) -> const typename T::value_type&
{
  return c[i % c.size()];
}

/**
 * @brief Returns a clamped reference to an element at index @p i in container @p c.
 *
 * This function accesses the element at index \c i in the container. If \c i is 
 * greater than or equal to \c c.size(), the function returns the last element 
 * (\c c.back()). If the container is empty, the behavior is undefined.
 *
 * @tparam T The type of the container. It must provide:
 *           - a `size()` member function returning a `std::size_t` count of elements
 *           - an index operator \c operator[]
 *           - a `back()` member function returning a reference to the last element
 * @param c  The container to be accessed.
 * @param i  The index to be clamped. If \c i is within the range `[0, c.size() - 1]`,
 *           the function returns \c c[i]. Otherwise, it returns \c c.back().
 * @return A const reference to the element at position \c i if \c i is in range,
 *         or a reference to the last element if \c i is out of range.
 *
 * @note The behavior is undefined if \c c is empty (i.e., if `c.size() == 0`).
 *
 * @code
 *   // Example container for animation frames
 *   constexpr std::array<u8, 7> animFrames = {0,1,2,2,0,3,4};
 *
 *   // Example usage:
 *   // Suppose we want to safely access a frame at position x,
 *   // clamping out-of-range indices to the last frame:
 *   u8 frame = clamp_to_edge(animFrames, x);
 * 
 *   // If x = 9 (out of range), the function returns animFrames.back() (i.e., 4).
 *   // If x = 3 (in range), the function returns animFrames[3] (i.e., 2).
 * @endcode
 */
template <class T>
constexpr auto clamp_to_edge(const T& c, std::size_t i) -> const typename T::value_type& {
    if (i >= c.size()) {
        return c.back(); // 最後の要素
    }
    return c[i];
}

/**
 * @brief Selects a random element from a fixed-size array.
 *
 * This function selects a random element from a fixed-size array using a custom random number generator.
 * The array size must be greater than 0; otherwise, a build error will occur.
 *
 * @tparam T The type of the array elements.
 * @tparam N The size of the array.
 * @param array The fixed-size array to select an element from.
 * @return A reference to the randomly selected element.
 *
 * @note The array must have at least one element. If an empty array is passed,
 *       this function will result in a build error due to the use of the `% N` operation.
 *       Always ensure that the array is non-empty before calling this function.
 *
 * @example
 * // Example 1: Using a predefined fixed-size array (two-line usage)
 * static const int numbers[] = {1, 2, 3, 4};
 * int selected = random_at(numbers);
 *
 * // Example 2: One-liner usage with a temporary array
 * int selected = random_at((int[]){10, 20, 30, 40});
 */
template <typename T, std::size_t N>
constexpr const T& random_at(const T (&array)[N]) {
    uint32_t random_value = genrand_int32();
    std::size_t index = random_value % N;
    return array[index];
}

/**
 * @brief Selects a random element from a standard container.
 *
 * This function selects a random element from a standard container (e.g., `std::vector`, `std::string`, etc.)
 * using a custom random number generator. If the container is empty, a specified default value is returned.
 *
 * @tparam T The type of the container.
 * @param container The container to select an element from.
 * @param default_value The value to return if the container is empty. Defaults to a default-constructed value of the container's value type.
 * @return A reference to the randomly selected element or the default value if the container is empty.
 *
 * @note This function relies on an external random number generator `genrand_int32()`
 *       that generates a random 32-bit integer in the range [0, 0xffffffff].
 *
 * @example
 * // Example 1: Using a std::vector
 * std::vector<int> numbers = {1, 2, 3, 4};
 * int selected = random_at(numbers);
 *
 * // Example 2: Using a std::string
 * std::string text = "hello";
 * char selected_char = random_at(text);
 */
template <typename T>
constexpr const typename T::value_type& random_at(const T& container, const typename T::value_type& default_value = typename T::value_type{}) {
    if (container.empty()) {
        return default_value;
    }
    uint32_t random_value = genrand_int32();
    std::size_t index = random_value % container.size();
    return container[index];
}

#define memsetz(addr_,bytesize_)  memset(addr_,0x00,bytesize_)

/**
 * @brief Format a string with variable arguments.
 * 
 * This function takes a format string and a variable number of arguments,
 * formats the string, and returns the resulting std::string.
 * 
 * @param format The format string (same format as printf)
 * @param ... Variable arguments to be used in the format string
 * 
 * @return The formatted std::string
 * 
 * @note The function uses a fixed-size internal buffer of 256 characters.
 *       Output that exceeds this size will be truncated.
 */
extern  std::string fs(const char* format, ...);

/**
 * @brief Allocate and zero-initialize a block of memory.
 * 
 * This function allocates a block of memory of the specified size,
 * and then initializes it to zero.
 * 
 * @param bytesize_ The size of the memory block to allocate, in bytes.
 * 
 * @return A pointer to the allocated and zero-initialized memory block,
 *         or NULL if the allocation failed.
 */
extern  void* mallocz( size_t bytesize_ );

/**
 * @brief Print memory allocation information.
 * 
 * This function retrieves memory allocation information from the system
 * and prints it to the standard output. The information includes various
 * metrics related to the current state of dynamic memory allocation.
 */
extern  void  print_mallinfo();