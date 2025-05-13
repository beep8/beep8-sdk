/**
 * @file cstr.h
 * @brief Module for managing fixed-size strings.
 * 
 * This module provides functionality for managing strings using the
 * `cstr` class and associated functions. The `cstr` class offers a fixed-size
 * buffer to store strings, simplifying memory management by avoiding dynamic
 * allocations. It provides various string operations such as assignment, concatenation,
 * comparison, and more.
 * 
 * The main components of this module are:
 * 
 * - **cstr Class**: A template class that manages strings using a fixed-size buffer.
 *   Unlike `std::string`, which uses dynamic memory, `cstr` uses a static array
 *   whose size is defined at compile time. This can lead to performance improvements
 *   in scenarios where memory allocation and deallocation are frequent.
 * 
 * - **Helper Functions**: `cstr_strlen` and `cstr_strcmp` are utility functions
 *   for measuring string length and comparing strings, respectively.
 * 
 * - **Comparison Function Object**: The `cmp_str` struct provides comparison
 *   operators for different sizes of `cstr` objects.
 * 
 * - **Utility Functions**: The `tostr` functions convert `cstr` objects to
 *   `std::string`.
 * 
 * ### cstr Class Details
 * 
 * The `cstr` class is a template class that holds a fixed-size character buffer.
 * For example, `cstr<16>` can hold a string of up to 15 characters plus a null terminator.
 * It uses the `_set` method internally for string operations.
 * 
 * - **Constructors**:
 *   - `cstr(const char* sz)`: Initializes the string with a C-style string.
 *   - `cstr()`: Default constructor.
 * - **Methods**:
 *   - `const char* c_str() const`: Returns the C-style string.
 *   - `void clear()`: Clears the string.
 *   - `void push_back(char cc)`: Appends a character to the string.
 *   - `void pop_back()`: Removes the last character from the string.
 *   - `cstr& operator=(const char* sz)`: Assigns a C-style string to the `cstr` object.
 *   - `cstr& operator+=(char ch)`: Appends a character to the string.
 *   - `cstr operator+(const cstr& s) const`: Concatenates two `cstr` strings.
 *   - `bool operator==(const cstr& s) const`: Compares two `cstr` strings.
 *   - `char at(u32 pos) const`: Returns the character at the specified position.
 *   - `size_t size() const`: Returns the length of the string.
 *   - `bool empty() const`: Checks if the string is empty.
 * 
 * ### Differences from std::string
 * 
 * - **Fixed Size vs. Variable Size**: The `cstr` class uses a fixed-size buffer,
 *   while `std::string` can dynamically change its size.
 * - **Memory Management**: `cstr` does not involve dynamic memory allocation,
 *   making it more efficient in scenarios with frequent memory operations.
 * - **Performance**: `cstr` can offer performance benefits due to the absence of
 *   dynamic memory allocation and deallocation. However, `std::string` is more
 *   flexible for handling longer strings.
 * - **Flexibility**: `std::string` can handle strings of arbitrary length,
 *   whereas `cstr` is limited by the size specified at compile time.
 * 
 * ### Usage Example
 * 
 * Below is an example of how to use this module:
 * 
 * @code
 * #include "cstr.h"
 * 
 * int main() {
 *     // Define fixed-length strings
 *     cstr<16> str1("Hello");
 *     cstr<16> str2("World");
 * 
 *     // Concatenate strings
 *     cstr<16> str3 = str1 + str2;
 * 
 *     // Display strings
 *     printf("str1: %s\n", str1.c_str());
 *     printf("str2: %s\n", str2.c_str());
 *     printf("str3: %s\n", str3.c_str());
 * 
 *     // Compare strings
 *     if (str1 == str2) {
 *         printf("str1 and str2 are equal\n");
 *     } else {
 *         printf("str1 and str2 are not equal\n");
 *     }
 * 
 *     return 0;
 * }
 * @endcode
 * 
 * @version 1.0
 * @date 2024
 * 
 * @note This module depends on the b8/type.h header for type definitions.
 */

#pragma once
#include <stdio.h>
#include <b8/type.h>
#include <cstddef>
#include <cstring>
#include <string>
#include <charconv>

extern  size_t  cstr_strlen( const char* sz_ );
extern  int     cstr_strcmp(const char *s1, const char *s2);
template <unsigned int TBYTES>
class cstr{
  char  _buff[ TBYTES ];

  void  _set( u32 idx_ , const char* sz ){
    u32 nn=idx_;
    if( sz ){
      for( ; nn<=TBYTES-2 ; ++nn,++sz){
        if( 0 == *sz ) break;
        _buff[ nn ] = *sz;
      }
    }
    _buff[ nn ] = 0x00;
  }

public:
  const char* c_str() const {
    return _buff;
  }

  const char* data() const {
    return _buff;
  }

  void  clear() {
    _buff[0] = 0x00;
  }

  void push_back( char cc ){
    char buff[2]={cc,0x00};
    _set(size(),buff);
  }

  void pop_back(){
    if(empty()) return;
    const size_t idx =  size()-1;
    _buff[ idx ] = 0x00;
  }

  cstr& operator = (const char* sz) {
    _set( 0, sz );
    return *this;
  }

  cstr& operator = (const std::string& str) {
    _set( 0, str.c_str() );
    return *this;
  }

  cstr& operator += (char ch) {
    char sz[2];
    sz[0]=ch;
    sz[1]=0x00;
    _set(size(),sz);
    return *this;
  }

  cstr& operator += (const char* sz) {
    if( nullptr == sz ) return *this;
    _set(size(),sz);
    return *this;
  }

  cstr& operator += (const cstr& str ) {
    _set(size(),str.c_str());
    return *this;
  }

  cstr operator+(const cstr& s) const {
    cstr ret = *this;
    ret += s;
    return ret;
  }

  bool operator == ( const cstr& s ) const {
    return cstr_strcmp( this->c_str(), s.c_str()) == 0;
  }

  char at( u32 pos ) const {
    if( pos < size()){
      return _buff[pos];
    }
    return 0x00;
  }

  size_t size() const{
    return cstr_strlen( _buff );
  }

  bool  empty() const{
    return  0 == size() ? true : false;
  }
  cstr( const char* sz ){ _set(0, sz ); }
  cstr(){ _set(0,""); }
};

using str8  = cstr<8>;
using str16 = cstr<16>;
using str32 = cstr<32>;
using str64 = cstr<64>;

struct cmp_str {
   bool operator()(const str8& a, const str8& b) const {
      return cstr_strcmp(a.c_str(), b.c_str()) < 0;
   }
   bool operator()(const str16& a, const str16& b) const {
      return cstr_strcmp(a.c_str(), b.c_str()) < 0;
   }
   bool operator()(const str32& a, const str32& b) const {
      return cstr_strcmp(a.c_str(), b.c_str()) < 0;
   }
   bool operator()(const str64& a, const str64& b) const {
      return cstr_strcmp(a.c_str(), b.c_str()) < 0;
   }
};

inline  std::string tostr( const str8 & x_ ){ return x_.c_str(); }
inline  std::string tostr( const str16& x_ ){ return x_.c_str(); }
inline  std::string tostr( const str32& x_ ){ return x_.c_str(); }
inline  std::string tostr( const str64& x_ ){ return x_.c_str(); }

template<typename String, typename T>
std::from_chars_result from_chars(const String& str, T& value) {
    return std::from_chars(str.data(), str.data() + str.size(), value);
}
