/**
 * @file pipe.h
 * @brief Module for data pipeline processing using pipes.
 * 
 * This module provides functions and classes for data pipeline processing using pipes.
 * The Pipe namespace contains various classes and functions to manage and transfer data
 * between different stages of a pipeline.
 * 
 * ### Features
 * 
 * - **CNullPipe**: A null pipe class that does nothing.
 * - **CMemBufferPipe**: A memory buffer pipe class that holds data in a buffer.
 * - **Move**: A function to move data from one pipe to another.
 * 
 * ### Usage Example
 * 
 * Here is an example of how to use this module to transfer data between pipes:
 * 
 * @code
 * #include <iostream>
 * #include <memory>
 * #include "pipe.h"
 * 
 * using namespace std;
 * using namespace Pipe;
 * 
 *  * int main() {
 *     // Create input and output pipes
 *     auto pipe_in = std::make_shared<Pipe::CMemBufferPipe>();
 *     auto pipe_out = std::make_shared<Pipe::CMemBufferPipe>();
 * 
 *     // Push data into the input pipe
 *     string input_data = "hello, world!";
 *     for (char c : input_data) {
 *         pipe_in->Push(static_cast<u8>(c));
 *     }
 * 
 *     // Dump the contents of the input pipe
 *     std::cout << "Input Pipe Dump:" << std::endl;
 *     pipe_in->Dump();
 * 
 *     // Move data from the input pipe to the output pipe while applying filtering
 *     u8 reg8;
 *     while (pipe_in->Pop(reg8)) {
 *         // Convert lowercase letters to uppercase
 *         reg8 = std::toupper(reg8);
 *         pipe_out->Push(reg8);
 *     }
 * 
 *     // Dump the contents of the output pipe
 *     std::cout << "Output Pipe Dump:" << std::endl;
 *     pipe_out->Dump();
 * 
 *     return 0;
 * }
 * @endcode
 * 
 * @version 1.0
 * @date 2022
 */

#pragma once
#include <cstddef>
#include <vector>
#include <memory>
#include <b8/type.h>
namespace Pipe {

/**
 * @brief Base class for all pipe implementations.
 */
class CPipe {
protected:
  size_t _pushed_in_bytes = 0;
private:
  virtual bool  vOnPush( u8 x_ ){
    (void)x_;
    return false;
  }
public:
  /**
   * @brief Pushes a string of data into the pipe.
   * 
   * @param str_ The string to push.
   * @return true if all data was pushed successfully, false otherwise.
   */
  bool Push(const std::string& str_) {
    bool failed = false;
    for( auto it : str_ ){
      if( ! Push( it ) ){
        failed = true;
      }
    }
    return failed ? false : true;
  }

  /**
   * @brief Pushes a byte of data into the pipe.
   * 
   * @param x_ The byte to push.
   * @return true if the data was pushed successfully, false otherwise.
   */
  bool Push(u8 x_) {
    bool result = vOnPush(x_);
    if (result) ++_pushed_in_bytes;
    return result;
  }

protected:
  size_t _poped_in_bytes = 0;
private:
  virtual bool vOnPop(u8& x_) {
    (void)x_;
    return false;
  }
public:
  /**
   * @brief Sets the position for the next Pop operation.
   * 
   * @param offset_ The offset to set for the next Pop operation.
   */
  void SeekPop(size_t offset_ = 0) {
    _poped_in_bytes = offset_;
  }

  /**
   * @brief Pops a byte of data from the pipe.
   * 
   * @param x_ The byte to pop.
   * @return true if the data was popped successfully, false otherwise.
   */
  bool Pop(u8& x_) {
    const bool result = vOnPop(x_);
    if (result) ++_poped_in_bytes;
    return result;
  }

public:
  virtual ~CPipe() {}
};

/**
 * @brief A null pipe class that does nothing.
 */
class CNullPipe : public CPipe {
public:
  CNullPipe();
  virtual ~CNullPipe();
};

/**
 * @brief A memory reader pipe class that reads data from a memory buffer.
 */
class CMemReaderPipe : public CPipe {
  const u8* _addr;
  size_t _bytesize;

  bool vOnPop(u8& x_) override {
    if (_poped_in_bytes < _bytesize) {
      x_ = *(_addr + _poped_in_bytes);
      return true;
    }
    return false;
  }

public:
  /**
   * @brief Gets the size of the memory buffer.
   * 
   * @return The size of the memory buffer.
   */
  size_t Size() const {
    return _bytesize;
  }

  /**
   * @brief Constructs a memory reader pipe.
   * 
   * @param addr_ The address of the memory buffer.
   * @param bytesize_ The size of the memory buffer.
   */
  CMemReaderPipe(const u8* addr_, size_t bytesize_)
    : _addr(addr_),
      _bytesize(bytesize_) {}
};

/**
 * @brief A memory buffer pipe class that holds data in a buffer.
 */
class CMemBufferPipe : public CPipe {
public:
  std::vector<u8> _buff;

  /**
   * @brief Dumps the contents of the memory buffer.
   */
  void Dump();

  /**
   * @brief Gets the size of the memory buffer.
   * 
   * @return The size of the memory buffer.
   */
  size_t Size() const {
    return _buff.size();
  }
private:
  bool vOnPush(u8 x_) override {
    _buff.push_back(x_);
    return true;
  }

  bool vOnPop(u8& x_) override {
    if (_poped_in_bytes < _buff.size()) {
      x_ = _buff[_poped_in_bytes];
      return true;
    }
    return false;
  }
};

/**
 * @brief A file pipe class that reads and writes data to a file.
 */
class CFilePipe : public CPipe {
  FILE* _fp = nullptr;
private:
  bool vOnPush(u8 x_) override {
    fputc(x_, _fp);
    return true;
  }

  bool vOnPop(u8& x_) override {
    int c = fgetc(_fp);
    if (c != EOF) {
      x_ = c;
      return true;
    } else {
      return false;
    }
  }
public:
  /**
   * @brief Constructs a file pipe.
   * 
   * @param fp_ The file pointer to use for reading and writing.
   */
  CFilePipe(FILE* fp_) {
    _fp = fp_;
  }
};

/**
 * @brief Moves data from one pipe to another.
 * 
 * This function moves data from the input pipe to the output pipe until the input pipe is empty.
 * 
 * @param pipe_in_ The input pipe from which data is read.
 * @param pipe_out_ The output pipe to which data is written.
 */
extern void Move(
  std::shared_ptr< Pipe::CPipe > pipe_in_,
  std::shared_ptr< Pipe::CPipe > pipe_out_
);

} // namespace Pipe