// MIT License
// 
// Copyright (c) 2022 ZooK
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/**
 * @file rle.h
 * @brief Header file for the RLE (Run-Length Encoding) compression and decompression module.
 * 
 * This module provides classes for compressing and decompressing data using RLE.
 * The CRleEncoder class encodes data from an input pipe and writes the compressed data to an output pipe.
 * The CRleDecoder class decodes compressed data from an input pipe and writes the decompressed data to an output pipe.
 * 
 * Usage example:
 * 
 * @code
 * #include <iostream>
 * #include <memory>
 * #include <vector>
 * #include <pipe.h>
 * #include <rle.h>
 * 
 * using namespace std;
 * using namespace Pipe;
 * using namespace Rle;
 * 
 * int main() {
 *     // Prepare input data
 *     vector<u8> input_data = {0, 0, 0, 0, 1, 2, 3, 3, 3, 3, 3, 0, 0, 0};
 * 
 *     // Load input data into memory pipe
 *     auto pipe_mem_reader = make_shared<CMemBufferPipe>();
 *     for (auto data : input_data) {
 *         pipe_mem_reader->Push(data);
 *     }
 * 
 *     // Prepare encoder and output pipe
 *     auto pipe_mem_packed = make_shared<CMemBufferPipe>();
 *     CRleEncoder encoder;
 *     encoder.SetIn(pipe_mem_reader);
 *     encoder.SetOut(pipe_mem_packed);
 * 
 *     // Perform RLE encoding
 *     encoder.Encode();
 * 
 *     // Dump the encoded result
 *     cout << "RLE Encoded Data:" << endl;
 *     pipe_mem_packed->Dump();
 * 
 *     // Prepare pipes for decoding
 *     auto pipe_mem_unpacked = make_shared<CMemBufferPipe>();
 *     CRleDecoder decoder;
 *     decoder.SetIn(pipe_mem_packed);
 *     decoder.SetOut(pipe_mem_unpacked);
 * 
 *     // Perform RLE decoding
 *     decoder.Decode();
 * 
 *     // Dump the decoded result
 *     cout << "RLE Decoded Data:" << endl;
 *     pipe_mem_unpacked->Dump();
 * 
 *     return 0;
 * }
 * @endcode
 * 
 * @note This file is part of the RLE module.
 */

#pragma once

#include <memory>
#include <b8/type.h>
#include <pipe.h>

/**
 * @namespace Rle
 * @brief The Rle namespace provides classes for run-length encoding (RLE) compression and decompression.
 */
namespace Rle {

/**
 * @class CRleEncoder
 * @brief Class for encoding data using run-length encoding (RLE).
 * 
 * This class reads data from an input pipe, performs RLE compression, and writes the compressed data to an output pipe.
 */
class CRleEncoder {
  std::shared_ptr< Pipe::CPipe > _pipe_in  = std::make_shared< Pipe::CNullPipe >();
  std::shared_ptr< Pipe::CPipe > _pipe_out = std::make_shared< Pipe::CNullPipe >();
public:
  /**
   * @brief Sets the input pipe for the encoder.
   * @param pipe_in_ Shared pointer to the input pipe.
   */
  void SetIn(std::shared_ptr< Pipe::CPipe > pipe_in_);

  /**
   * @brief Sets the output pipe for the encoder.
   * @param pipe_out_ Shared pointer to the output pipe.
   */
  void SetOut(std::shared_ptr< Pipe::CPipe > pipe_out_);

  /**
   * @brief Enum representing the result of the encoding process.
   */
  enum EncodeResult {
    ENCODE_OK,                ///< Encoding completed successfully.
    ENCODE_ERR_INVALID_PIPE,  ///< Invalid input or output pipe.
  };

  /**
   * @brief Performs the RLE encoding process.
   * @return Result of the encoding process.
   */
  CRleEncoder::EncodeResult Encode();
};

/**
 * @class CRleDecoder
 * @brief Class for decoding data using run-length encoding (RLE).
 * 
 * This class reads compressed data from an input pipe, performs RLE decompression, and writes the decompressed data to an output pipe.
 */
class CRleDecoder {
  std::shared_ptr< Pipe::CPipe > _pipe_in  = std::make_shared< Pipe::CNullPipe >();
  std::shared_ptr< Pipe::CPipe > _pipe_out = std::make_shared< Pipe::CNullPipe >();

public:
  /**
   * @brief Sets the input pipe for the decoder.
   * @param pipe_in_ Shared pointer to the input pipe.
   */
  void SetIn(std::shared_ptr< Pipe::CPipe > pipe_in_);

  /**
   * @brief Sets the output pipe for the decoder.
   * @param pipe_out_ Shared pointer to the output pipe.
   */
  void SetOut(std::shared_ptr< Pipe::CPipe > pipe_out_);

  /**
   * @brief Enum representing the result of the decoding process.
   */
  enum DecodeResult {
    DECODE_OK,                    ///< Decoding completed successfully.
    DECODE_ERR_INVALID_SIGNATURE, ///< Invalid RLE signature.
    DECODE_ERR_INVALID_PIPE,      ///< Invalid input or output pipe.
    DECODE_ERR_INVALID_DATA,      ///< Invalid compressed data.
  };

  /**
   * @brief Performs the RLE decoding process.
   * @return Result of the decoding process.
   */
  CRleDecoder::DecodeResult Decode();
};

} // namespace Rle
