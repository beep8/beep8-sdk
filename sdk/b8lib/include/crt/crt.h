/**
 * @file crt.h
 * @brief Header file for C runtime initialization and system functions.
 * 
 * This header file provides declarations for functions and structures used 
 * during the C runtime initialization process and other system-level operations.
 * These functions and structures are typically reserved for system use and 
 * should not be called directly by user applications.
 */

#pragma once
#include <sys/stat.h>
#include <stddef.h>

#ifdef  __cplusplus
extern  "C" {
#endif

/**
 * @brief Represents a file in the file system.
 * 
 * This structure is used to store information about an open file.
 * Each file has its own private data and driver-specific private data.
 * The 'used' flag indicates whether the file is currently in use.
 * The 'mode' represents the access mode for the file.
 */
typedef struct
{
  void*   f_priv;     ///< File-specific private data
  void*   d_priv;     ///< Driver-specific private data
  uint8_t used;       ///< Flag indicating if the file is in use
  int     mode;       ///< Access mode for the file
} File;

/**
 * @brief Represents the operations that can be performed on a file.
 * 
 * This structure contains function pointers for various file operations such as
 * opening, closing, reading, writing, seeking, and performing IOCTL commands.
 */
typedef struct
{
  int     (*open)(  File* filep);             ///< Function pointer to open a file
  int     (*close)( File* filep);             ///< Function pointer to close a file
  ssize_t (*read)(  File* filep, char *buffer, size_t buflen); ///< Function pointer to read from a file
  ssize_t (*write)( File* filep, const char *buffer, size_t buflen); ///< Function pointer to write to a file
  off_t   (*seek)(  File* filep, int ptr,int dir); ///< Function pointer to seek within a file
  int     (*ioctl)( File* filep, unsigned int cmd, void* arg); ///< Function pointer to perform IOCTL operations on a file
} file_operations;

#define N_MAX_PATH  (32)
/**
 * @brief Represents a file system driver.
 * 
 * This structure contains information about a file system driver, including the
 * path it manages, the file operations it supports, its access mode, and private data.
 */
typedef struct {
  char _path[ N_MAX_PATH ];                   ///< Path managed by the driver
  const file_operations* _fops;               ///< Supported file operations
  mode_t _mode;                               ///< Access mode for the driver
  void* _priv;                                ///< Driver-specific private data
} FsDriver ;

/**
 * @brief Registers a file system driver.
 * 
 * This function registers a new file system driver with the specified path,
 * file operations, access mode, and private data.
 * 
 * @param path The path managed by the driver.
 * @param fops The file operations supported by the driver.
 * @param mode The access mode for the driver.
 * @param priv The driver-specific private data.
 * @return int 0 on success, negative value on failure.
 */
extern  int fs_register_driver(
  const char* path,
  const file_operations* fops,
  mode_t mode,
  void* priv
);

/**
 * @brief Performs an IOCTL operation on a file.
 * 
 * This function performs an IOCTL operation on the specified file descriptor
 * with the given command and argument.
 * 
 * @param fd The file descriptor.
 * @param cmd The IOCTL command.
 * @param arg The argument for the IOCTL command.
 * @return int 0 on success, negative value on failure.
 */
extern  int _ioctl(int fd, unsigned int cmd, void* arg);

#ifdef  __cplusplus
}
#endif