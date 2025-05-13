/**
 * @file handle.h
 * @brief Module for managing pointers using handles.
 * 
 * This module provides functions to manage pointers using handles. It allows
 * accessing pointers through handles, checking the validity of handles, and
 * removing handles. This is particularly useful for dynamic memory management
 * and resource management.
 * 
 * This module is provided as a helper for lower-level functions of cobj. While
 * users can use it directly, it is neither recommended nor mandatory.
 * 
 *  **Note**: This module is not thread-safe. Use appropriate synchronization mechanisms
 * if accessing it from multiple threads.
 * 
 * ### Features
 * 
 * - **Handle_Reset**: Resets the handle management table.
 * - **Handle_Dump**: Dumps the contents of the handle management table.
 * - **Handle_Entry**: Registers a pointer and returns a handle.
 * - **Handle_GetPointer**: Retrieves a pointer from a handle.
 * - **Handle_IsAlive**: Checks if a handle is valid.
 * - **Handle_Remove**: Removes a handle and frees the associated pointer.
 * 
 * ### Usage Example
 * 
 * Here is an example of how to use this module to manage pointers with handles:
 * 
 * @code
 * #include "handle.h"
 * #include <stdio.h>
 * #include <stdlib.h>
 * 
 * int main() {
 *     // Reset the handle management table
 *     Handle_Reset();
 * 
 *     // Allocate memory on the heap, register the pointer, and get a handle
 *     int* pHeap = (int*)malloc(sizeof(int));
 *     if (pHeap == NULL) {
 *         perror("Failed to allocate memory");
 *         return 1;
 *     }
 *     *pHeap = 10;
 *     u32 handle = Handle_Entry(pHeap);
 *     printf("Handle: %u\n", handle);
 * 
 *     // Retrieve the pointer from the handle
 *     int* p = (int*)Handle_GetPointer(handle);
 *     if (p) {
 *         printf("Value: %d\n", *p);
 *     } else {
 *         printf("Pointer not found\n");
 *     }
 * 
 *     // Check if the handle is valid
 *     if (Handle_IsAlive(handle)) {
 *         printf("Handle is alive\n");
 *     } else {
 *         printf("Handle is not alive\n");
 *     }
 * 
 *     // Free the pointer and remove the handle
 *     free(p); // Free the pointer before removing the handle
 *     Handle_Remove(handle);
 * 
 *     // Check that the handle has been successfully removed
 *     if (!Handle_IsAlive(handle)) {
 *         printf("Handle successfully removed\n");
 *     } else {
 *         printf("Handle removal failed\n");
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

#define HANDLE_NULL  (0)

/**
 * @brief Resets the handle management table.
 * 
 * This function resets the handle management table, initializing all entries
 * and resetting the next handle to be used.
 */
extern void Handle_Reset();

/**
 * @brief Dumps the contents of the handle management table.
 * 
 * This function dumps the contents of the handle management table, displaying
 * all valid entries.
 */
extern void Handle_Dump();

/**
 * @brief Registers a pointer and returns a handle.
 * 
 * This function registers a pointer and returns a handle that can be used to
 * access the pointer.
 * 
 * @param pPtr Pointer to be registered.
 * @return Handle corresponding to the registered pointer.
 */
extern u32 Handle_Entry(void* pPtr);

/**
 * @brief Retrieves a pointer from a handle.
 * 
 * This function retrieves the pointer associated with the given handle.
 * 
 * @param hdl Handle to be used to retrieve the pointer.
 * @return Pointer associated with the handle, or NULL if the handle is invalid.
 */
extern void* Handle_GetPointer(u32 hdl);

/**
 * @brief Checks if a handle is valid.
 * 
 * This function checks if the given handle is valid and has an associated pointer.
 * 
 * @param hdl Handle to be checked.
 * @return true if the handle is valid, false otherwise.
 */
extern bool Handle_IsAlive(u32 hdl);

/**
 * @brief Removes a handle and frees the associated pointer.
 * 
 * This function removes the given handle and frees the memory associated with the pointer.
 * 
 * @param hdl Handle to be removed.
 */
extern void Handle_Remove(u32 hdl);
