// MIT License

// Copyright (c) 2022 ZooK

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
/**
 * @file huffman.h
 * @brief Module for Huffman encoding and decoding.
 * 
 * This module provides classes for Huffman encoding and decoding.
 * It uses the Pipe module for data transfer between different stages of the encoding and decoding process.
 * 
 * ### Huffman Format
 * The Huffman format used in this module is defined as follows:
 * 
 * - u8  _signature; // = 0x77 = Huffman::Signature
 * - u1  _flat;      // 0: HuffmanEncoded, 1: NotEncoded
 * 
 * if (_flat == 0) {
 *     - u9  _num_of_huffman_table;
 *     {
 *         u5  _num_of_bits_huffman_code;
 *         u$(_num_of_bits_huffman_code) _huffman_code;
 *         u8  _decode_data;
 *     }[_num_of_huffman_table];
 *     - u20 _byte_size_of_orgin;
 * } else {
 *     - u20 _byte_size_of_orgin;
 *     - u3  _reserved;
 *     - u8  _flat_data[_byte_size_of_orgin];
 * }
 * 
 * ### Usage Example
 * 
 * Here is an example of how to use this module to perform Huffman encoding and decoding:
 * 
 * @code
 * #include <iostream>
 * #include <memory>
 * #include <huffman.h>
 * #include <pipe.h>
 * 
 * using namespace std;
 * using namespace Pipe;
 * using namespace Huffman;
 * 
 * int main() {
 *     // Prepare input data
 *     const char* input_data = "example data to compress";
 *     size_t input_size = strlen(input_data);
 * 
 *     // Create memory reader and buffer pipes
 *     auto pipe_mem_reader = make_shared<CMemReaderPipe>(reinterpret_cast<const u8*>(input_data), input_size);
 *     auto pipe_mem_packed = make_shared<CMemBufferPipe>();
 * 
 *     // Create Huffman encoder and set input and output pipes
 *     CHuffmanEncoder encoder;
 *     encoder.SetIn(pipe_mem_reader);
 *     encoder.SetOut(pipe_mem_packed);
 * 
 *     // Perform Huffman encoding
 *     encoder.Encode();
 * 
 *     // Dump encoded data
 *     cout << "Encoded Data Dump:" << endl;
 *     pipe_mem_packed->Dump();
 * 
 *     // Create memory buffer and reader pipes for decoding
 *     auto decode_pipe_in = make_shared<CMemBufferPipe>();
 *     auto decode_pipe_out = make_shared<CMemBufferPipe>();
 * 
 *     // Move encoded data to the decoder's input pipe
 *     Pipe::Move(pipe_mem_packed, decode_pipe_in);
 * 
 *     // Create Huffman decoder and set input and output pipes
 *     CHuffmanDecoder decoder;
 *     decoder.SetIn(decode_pipe_in);
 *     decoder.SetOut(decode_pipe_out);
 * 
 *     // Perform Huffman decoding
 *     if (decoder.Decode() != CHuffmanDecoder::DECODE_OK) {
 *         cerr << "Decoding failed!" << endl;
 *         return 1;
 *     }
 * 
 *     // Dump decoded data
 *     cout << "Decoded Data Dump:" << endl;
 *     decode_pipe_out->Dump();
 * 
 *     return 0;
 * }
 * @endcode
 * 
 * @version 1.0
 * @date 2024
 */

#pragma once

#include <memory>
#include <b8/type.h>
#include <pipe.h>

namespace Huffman {
constexpr u8  Signature = 0x77;

/**
 * @class CHuffmanEncoder
 * @brief Class for Huffman encoding.
 * 
 * This class provides methods to encode data using Huffman coding.
 */
class CHuffmanEncoder {
  std::shared_ptr<Pipe::CPipe> _pipe_in = std::make_shared<Pipe::CNullPipe>();
  std::shared_ptr<Pipe::CPipe> _pipe_out = std::make_shared<Pipe::CNullPipe>();
  void _Output(std::shared_ptr<Pipe::CPipe> pipe_mem_huf_packed_);

public:
  /**
   * @brief Sets the input pipe for the encoder.
   * 
   * @param pipe_in_ The input pipe.
   */
  void SetIn(std::shared_ptr<Pipe::CPipe> pipe_in_);

  /**
   * @brief Sets the output pipe for the encoder.
   * 
   * @param pipe_out_ The output pipe.
   */
  void SetOut(std::shared_ptr<Pipe::CPipe> pipe_out_);

  /**
   * @brief Performs Huffman encoding on the input data.
   */
  void Encode();
};

/**
 * @class CHuffmanDecoder
 * @brief Class for Huffman decoding.
 * 
 * This class provides methods to decode data that was encoded using Huffman coding.
 */
class CHuffmanDecoder {
  std::shared_ptr< Pipe::CPipe > _pipe_in  = std::make_shared< Pipe::CNullPipe >();
  std::shared_ptr< Pipe::CPipe > _pipe_out = std::make_shared< Pipe::CNullPipe >();
public:
  /**
   * @enum DecodeResult
   * @brief Enumeration for the result of the decoding process.
   */
  enum DecodeResult{
    DECODE_OK,
    DECODE_ERR_INVALID_SIGNATURE,
    DECODE_INVALID_DATA,
  };
private:
  /**
   * @brief Helper method to decode flat (non-Huffman encoded) data.
   * 
   * @return The result of the decoding process.
   */
  CHuffmanDecoder::DecodeResult  _DoDecodeFlat();

public:
  /**
   * @brief Sets the input pipe for the decoder.
   * 
   * @param pipe_in_ The input pipe.
   */
  void  SetIn ( std::shared_ptr< Pipe::CPipe > pipe_in_  );

  /**
   * @brief Sets the output pipe for the decoder.
   * 
   * @param pipe_out_ The output pipe.
   */
  void  SetOut( std::shared_ptr< Pipe::CPipe > pipe_out_ );

  /**
   * @brief Performs Huffman decoding on the input data.
   * 
   * @return The result of the decoding process.
   */
  CHuffmanDecoder::DecodeResult  Decode();
};

} // namespace Huffman