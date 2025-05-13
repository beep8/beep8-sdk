/**
 * @file
 * @brief Header file for the b8array class.
 * 
 * This header defines a template class `b8array` that represents a fixed-size array with 
 * custom behavior for element initialization. The class supports basic functionalities 
 * such as adding elements, accessing elements, and iterating over the array. 
 * Additionally, it provides options for initializing the array with default-constructed elements.
 *
 * # Example Usage
 *
 * ## Basic Usage with Built-in Type (int)
 *
 * @code
 * #include "b8array.h"
 * 
 * b8array<int, 5> arr; // Creates an array with capacity of 5.
 * arr.push_back(1);    // Add elements to the array.
 * arr.push_back(2);
 * 
 * for (int x : arr) {  // Iterate through elements.
 *     std::cout << x << std::endl;
 * }
 * @endcode
 *
 * ## Basic Usage with Custom Class
 *
 * Suppose we have a custom class `MyClass` defined as follows:
 *
 * @code
 * class MyClass {
 *     int data;
 * public:
 *     MyClass(int value) : data(value) {}
 *     int getValue() const { return data; }
 * };
 * @endcode
 *
 * We can use the `b8array` class with `MyClass` in a similar way:
 *
 * @code
 * #include "b8array.h"
 * 
 * b8array<MyClass, 3> arr;          // Creates an array with capacity of 3.
 * arr.push_back(MyClass(1));        // Add MyClass objects to the array.
 * arr.push_back(MyClass(2));
 * 
 * for (const MyClass& obj : arr) {  // Iterate through elements.
 *     std::cout << obj.getValue() << std::endl;
 * }
 * @endcode
 *
 * ## Using Initialization Option
 *
 * @code
 * #include "b8array.h"
 *
 * // Creates an array with capacity of 5 and fills it with default-constructed MyClass objects.
 * b8array<MyClass, 5> arr(ArrayOptions::Fill); 
 * @endcode
 *
 * ## Element Access and Removal
 *
 * @code
 * #include "b8array.h"
 * 
 * b8array<MyClass, 5> arr(ArrayOptions::Fill);
 * arr.push_back(MyClass(1));
 * arr.push_back(MyClass(2));
 * 
 * const MyClass& firstElement = arr.at(0);  // Access element by index.
 * arr.erase(arr.begin());                   // Remove the first element.
 * @endcode
 *
 * ## Copying Arrays
 *
 * @code
 * #include "b8array.h"
 * 
 * b8array<MyClass, 5> arr1;
 * arr1.push_back(MyClass(1));
 * arr1.push_back(MyClass(2));
 * 
 * b8array<MyClass, 5> arr2 = arr1;    // Copy contents of arr1 into arr2.
 * @endcode
 */
#pragma once
#include <b8/assert.h>
#include <cstddef>
#include <stdio.h>
#include <stdlib.h>
#include <trace.h>

namespace ArrayOptions {
    /**
     * @brief Enum for specifying array initialization options.
     */
    enum InitializationOption {
        Fill,   ///< Fill the array with default-constructed elements.
        NoFill  ///< Do not fill the array.
    };
}

/**
 * @brief A fixed-size array with custom behavior for element initialization.
 *
 * @tparam T Type of the elements.
 * @tparam Capacity Maximum number of elements the array can hold.
 */
template <typename T, size_t Capacity>
class b8array {
private:
    T* _buff = nullptr;  ///< Buffer for storing array elements.
    size_t _size = 0;    ///< Current number of elements in the array.

    /**
     * @brief Allocate memory for the array buffer.
     */
    void alloc() {
        _buff = new T[Capacity + 1];
    }

    /**
     * @brief Clone the contents of another b8array.
     *
     * @param obj The b8array to clone from.
     */
    void clone(const b8array& obj) {
        if (_buff) {
            delete[] _buff;
        }
        _size = obj._size;
        alloc();
        for (size_t nn = 0; nn < _size; ++nn) {
            _buff[nn] = obj._buff[nn];
        }
    }

public:
    /**
     * @brief Construct a new b8array.
     *
     * @param option Specifies whether the array should be filled with default-constructed elements.
     */
    b8array(ArrayOptions::InitializationOption option = ArrayOptions::NoFill) {
        alloc();
        if (option == ArrayOptions::Fill) {
            T t{};
            for (size_t ii = 0; ii < Capacity; ++ii) {
                push_back(t);
            }
        }
    }

    /**
     * @brief Construct a new b8array with an initializer list.
     *
     * @param il Initializer list with elements to be added to the array.
     */
    b8array(std::initializer_list<T> il) {
        alloc();
        size_t index = 0;
        for (const auto& val : il) {
            _ASSERT(index < Capacity, "Capacity exceeded by initializer list");
            if (index < Capacity) {
                _buff[index++] = val;
                _size++;
            }
        }
    }

    /**
     * @brief Copy constructor for b8array.
     *
     * @param obj The b8array to copy from.
     */
    b8array(const b8array& obj) {
        clone(obj);
    }

    /**
     * @brief Copy assignment operator.
     *
     * @param obj The b8array to assign from.
     * @return *this
     */
    b8array& operator=(const b8array& obj) {
        if (this != &obj) {
            clone(obj);
        }
        return *this;
    }

    /**
     * @brief Destructor to release allocated memory.
     */
    ~b8array() {
        if (_buff) {
            delete[] _buff;
        }
    }

    /**
     * @brief Check if the array is empty.
     *
     * @return true if the array is empty, false otherwise.
     */
    bool empty() const {
        return _size == 0;
    }

    /**
     * @brief Get an iterator pointing to the first element.
     *
     * @return Pointer to the first element.
     */
    T* begin() {
        return _buff;
    }

    /**
     * @brief Get a const iterator pointing to the first element.
     *
     * @return Const pointer to the first element.
     */
    const T* cbegin() const {
        return _buff;
    }

    /**
     * @brief Get a const iterator pointing to one past the last element.
     *
     * @return Const pointer to one past the last element.
     */
    const T* cend() const {
        return _buff + _size;
    }

    /**
     * @brief Get an iterator pointing to one past the last element.
     *
     * @return Pointer to one past the last element.
     */
    T* end() {
        return _buff + _size;
    }

    /**
     * @brief Get a const iterator pointing to the first element.
     *
     * @return Const pointer to the first element.
     */
    const T* begin() const {
        return _buff;
    }

    /**
     * @brief Get a const iterator pointing to one past the last element.
     *
     * @return Const pointer to one past the last element.
     */
    const T* end() const {
        return _buff + _size;
    }

    /**
     * @brief Erase an element from the array.
     *
     * @param it_ Iterator pointing to the element to erase.
     * @return Iterator following the removed element.
     */
    T* erase(T* it_) {
        _ASSERT(it_ >= _buff && it_ < _buff + _size, "Invalid iterator");
        if (it_ < _buff || it_ >= _buff + _size) {
            return _buff + _size;
        }
        for (T* p = it_; p < _buff + _size - 1; ++p) {
            *p = *(p + 1);
        }
        --_size;
        return it_;
    }

    /**
     * @brief Clear the contents of the array.
     */
    void clear() {
        _size = 0;
    }

    /**
     * @brief Get the number of elements in the array.
     *
     * @return Number of elements in the array.
     */
    size_t size() const {
        return _size;
    }

    /**
     * @brief Add an element to the end of the array.
     *
     * @param x_ Element to be added.
     */
    void push_back(const T& x_) {
        _ASSERT(_size < Capacity, "over flow");
        _buff[_size++] = x_;
    }

    /**
     * @brief Access element at a specific position.
     *
     * @param n_ Index of the element to be accessed.
     * @return Reference to the element at index n_.
     */
    T& at(int n_) {
        _ASSERT(n_ >= 0, "array exception");
        _ASSERT(n_ < static_cast<int>(size()), "array exception");
        return _buff[n_];
    }

    /**
     * @brief Access element at a specific position (const version).
     *
     * @param n_ Index of the element to be accessed.
     * @return Const reference to the element at index n_.
     */
    const T& at(int n_) const {
        _ASSERT(n_ >= 0, "array exception");
        _ASSERT(n_ < static_cast<int>(size()), "array exception");
        return _buff[n_];
    }
    /**
     * @brief Overload array subscript operator for non-const objects.
     *
     * @param n_ Index of the element to be accessed.
     * @return Reference to the element at index n_.
     */
    T& operator[](int n_) {
        _ASSERT(n_ >= 0, "array exception");
        _ASSERT(n_ < static_cast<int>(size()), "array exception");
        return _buff[n_];
    }

    /**
     * @brief Overload array subscript operator for const objects.
     *
     * @param n_ Index of the element to be accessed.
     * @return Const reference to the element at index n_.
     */
    const T& operator[](int n_) const {
        _ASSERT(n_ >= 0, "array exception");
        _ASSERT(n_ < static_cast<int>(size()), "array exception");
        return _buff[n_];
    }
};
