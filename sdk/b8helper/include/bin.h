/**
 * @file bin.h
 * @brief Module for managing binary data with named registrations.
 * 
 * This module provides functionality to manage binary data using the
 * `CBin` and `CBinHolder` classes. Specifically, it allows for the registration
 * of binary data with a name and provides methods to retrieve this data as needed.
 * 
 * The main components of this module are:
 * 
 * - **CBin Class**: This class holds information about the binary data, including
 *   its address, size, and name. The class provides methods to retrieve the address
 *   and size of the binary data.
 * 
 * - **CBinHolder Class**: This class manages `CBin` objects by registering them
 *   with a name and providing methods to search and retrieve `CBin` objects by name.
 * 
 * ### CBin Class
 * 
 * The `CBin` class is designed to hold the address, size, and name of binary data.
 * It is initialized through its constructor and provides methods to access the
 * address and size of the binary data.
 * 
 * - **Constructor**: `CBin(const char* name_, const void* addr_, size_t size_)`
 * - **Methods**:
 *   - `Addr() const`: Returns the address of the data.
 *   - `Size() const`: Returns the size of the data.
 * 
 * ### CBinHolder Class
 * 
 * The `CBinHolder` class is responsible for managing `CBin` objects. It supports
 * registering `CBin` objects with a name and provides static methods to retrieve
 * the objects by name. It also offers a method to get a dictionary of all registered
 * `CBin` objects.
 * 
 * - **Methods**:
 *   - `Register(CBin* bin_)`: Registers a `CBin` object.
 *   - `static Get(const char* name_)`: Retrieves a `CBin` object by name.
 *   - `static const std::unordered_map<std::string, CBin*>& GetDictionary()`: 
 *     Returns a dictionary of all registered `CBin` objects.
 * 
 * ### Usage Example
 * 
 * Below is an example of how to use this module:
 * 
 * @code
 * #include "bin.h"
 * 
 * int main() {
 *     // Define binary data (example data as strings)
 *     const char data1[] = "BinaryData1";
 *     const char data2[] = "BinaryData2";
 * 
 *     // Register binary data as CBin objects
 *     CBin bin1("data1", data1, sizeof(data1));
 *     CBin bin2("data2", data2, sizeof(data2));
 * 
 *     // Retrieve registered binary data by name
 *     CBin* retrievedBin1 = CBinHolder::Get("data1");
 *     CBin* retrievedBin2 = CBinHolder::Get("data2");
 * 
 *     // Display information about the retrieved binary data
 *     if (retrievedBin1) {
 *         printf("Name: data1, Addr: %p, Size: %zu\n", retrievedBin1->Addr(), retrievedBin1->Size());
 *     }
 *     if (retrievedBin2) {
 *         printf("Name: data2, Addr: %p, Size: %zu\n", retrievedBin2->Addr(), retrievedBin2->Size());
 *     }
 * 
 *     return 0;
 * }
 * @endcode
 * 
 * @version 1.0
 * @date 2024
 * 
 * @note This module depends on the b8/assert.h and trace.h headers for 
 * assertions and debugging.
 */
#pragma once
#include <cstddef>
#include <string>
#include <unordered_map>
class   CBin;
class   CBinHolder{
  std::unordered_map< std::string , CBin* > _dict;
public:
  void    Register( CBin* bin_ );
  static  CBin*   Get( const char* name_ );
  static  const std::unordered_map< std::string , CBin* >& GetDictionary();
  CBinHolder();
};

class CBin{
  friend class CBinHolder;
  const void* _addr;
  size_t      _size;
  std::string _name;
public:
  const void*   Addr()  const;
  size_t        Size()  const;
  CBin( const char* name_ , const void* addr_ , size_t size_ );
};