/**
 * @file tostr.h
 * @brief Utility functions for converting various data types to std::string.
 *
 * This header file provides a set of inline functions to convert different data types
 * to their string representations using std::to_string and custom formatting.
 * The provided functions handle conversions for integral types, floating-point types,
 * pointers, and C-style strings.
 *
 * Example usage:
 * @code
 * int num = 42;
 * std::string str = tostr(num); // Converts integer to string
 *
 * float fnum = 3.14f;
 * std::string fstr = tostr(fnum); // Converts float to string
 *
 * void* ptr = static_cast<void*>(&num);
 * std::string ptrstr = tostr(ptr); // Converts pointer to hexadecimal string
 *
 * const char* cstr = "hello";
 * std::string str = tostr(cstr); // Converts C-style string to std::string
 * @endcode
 */
#pragma once
#include <string>

/**
 * @brief Converts an integer to a std::string.
 * @param val The integer value to convert.
 * @return The string representation of the integer.
 */
inline std::string tostr(int val) {
    return std::to_string(val);
}

/**
 * @brief Converts an unsigned integer to a std::string.
 * @param val The unsigned integer value to convert.
 * @return The string representation of the unsigned integer.
 */
inline std::string tostr(unsigned int val) {
    return std::to_string(val);
}

/**
 * @brief Converts a long integer to a std::string.
 * @param val The long integer value to convert.
 * @return The string representation of the long integer.
 */
inline std::string tostr(long val) {
    return std::to_string(val);
}

/**
 * @brief Converts an unsigned long integer to a hexadecimal std::string.
 * @param val The unsigned long integer value to convert.
 * @return The hexadecimal string representation of the unsigned long integer.
 */
inline std::string tostrhex(unsigned long val) {
    std::string str("0x");
    for (int sft = 28; sft >= 0; sft -= 4) {
        str.push_back("0123456789abcdef"[(val >> sft) & 0xf]);
    }
    return str;
}

/**
 * @brief Converts a pointer to a hexadecimal std::string.
 * @param val The pointer value to convert.
 * @return The hexadecimal string representation of the pointer.
 */
inline std::string tostr(void* val) {
    return tostrhex(reinterpret_cast<unsigned int>(val));
}

/**
 * @brief Converts an unsigned long integer to a std::string.
 * @param val The unsigned long integer value to convert.
 * @return The string representation of the unsigned long integer.
 */
inline std::string tostr(unsigned long val) {
    return std::to_string(val);
}

/**
 * @brief Converts a long long integer to a std::string.
 * @param val The long long integer value to convert.
 * @return The string representation of the long long integer.
 */
inline std::string tostr(long long val) {
    return std::to_string(val);
}

/**
 * @brief Converts an unsigned long long integer to a std::string.
 * @param val The unsigned long long integer value to convert.
 * @return The string representation of the unsigned long long integer.
 */
inline std::string tostr(unsigned long long val) {
    return std::to_string(val);
}

/**
 * @brief Converts a float to a std::string.
 * @param val The float value to convert.
 * @return The string representation of the float.
 */
inline std::string tostr(float val) {
    return std::to_string(val);
}

/**
 * @brief Converts a double to a std::string.
 * @param val The double value to convert.
 * @return The string representation of the double.
 */
inline std::string tostr(double val) {
    return std::to_string(val);
}

/**
 * @brief Converts a long double to a std::string.
 * @param val The long double value to convert.
 * @return The string representation of the long double.
 */
inline std::string tostr(long double val) {
    return std::to_string(val);
}

/**
 * @brief Converts a C-style string to a std::string.
 * @param sz The C-style string to convert.
 * @return The std::string representation of the C-style string.
 */
inline std::string tostr(const char* sz) {
    return std::string(sz);
}

/**
 * @brief Returns the given std::string.
 * @param str_ The std::string to return.
 * @return The input std::string.
 */
inline std::string tostr(std::string str_) {
    return str_;
}
