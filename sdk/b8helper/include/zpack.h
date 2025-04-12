/**
 * @file zpack.h
 * @brief Header file for ZPack compression and decompression classes.
 * 
 * This module provides functionality for compressing and decompressing data 
 * using various methods including Huffman encoding, Run-Length Encoding (RLE), 
 * and their combinations. The primary classes are CZPackEncoder and CZPackDecoder, 
 * which handle the encoding and decoding of data streams respectively.
 * 
 * The ZPack format supports the following compression methods:
 * - Huffman followed by RLE (HuffmanToRle)
 * - Huffman only (Huffman)
 * - No compression (Flat)
 * - No compression with original size (FlatWithSize)
 * 
 * Example usage:
 * 
 * @code
 * // Create pipes for input and output
 * auto pipe_in = std::make_shared<Pipe::CMemBufferPipe>();
 * auto pipe_out = std::make_shared<Pipe::CMemBufferPipe>();
 * 
 * // Fill the input pipe with data
 * pipe_in->Push('a');
 * pipe_in->Push('b');
 * pipe_in->Push('c');
 * 
 * // Create and configure the encoder
 * ZPack::CZPackEncoder encoder;
 * encoder.SetIn(pipe_in);
 * encoder.SetOut(pipe_out);
 * encoder.Encode();
 * 
 * // Create and configure the decoder
 * auto decoded_pipe = std::make_shared<Pipe::CMemBufferPipe>();
 * ZPack::CZPackDecoder decoder;
 * decoder.SetIn(pipe_out);
 * decoder.SetOut(decoded_pipe);
 * auto result = decoder.Decode();
 * 
 * if (result == ZPack::CZPackDecoder::DECODE_OK) {
 *     // Process the decoded data
 * }
 * @endcode
 * 
 * Note: This module is designed for compression and decompression of data streams 
 * and is not thread-safe.
 */

/*
  ZPack format:
    u8  _signature; // = 0x99 = ZPack::Signature
    u8  _compression_method[2:0]  // = 0:huffman->rle 1:huffman 2:flat 3:flat with size
        reserved[7:3]  // reserved

    // only when 3:flat with size
    u8  size[ 7 : 0]
    u8  size[15 : 8]
    u8  size[23 :16]
*/

#pragma once

#include <memory>
#include <b8/type.h>
#include <pipe.h>

namespace ZPack {
constexpr u8  Signature = 0x99;

/**
 * @enum CompressionMethod
 * @brief Enum representing the available compression methods.
 */
enum CompressionMethod {
  HuffmanToRle, ///< Huffman encoding followed by Run-Length Encoding (RLE)
  Huffman,      ///< Huffman encoding only
  Flat,         ///< No compression
  FlatWithSize  ///< No compression, includes original size
};

/**
 * @class CZPackEncoder
 * @brief Class for encoding data streams using various compression methods.
 * 
 * This class provides methods to set input and output pipes and to perform 
 * the encoding of data streams using Huffman encoding, RLE, or no compression.
 */
class CZPackEncoder {
  std::shared_ptr< Pipe::CPipe > _pipe_in  = std::make_shared< Pipe::CNullPipe >();
  std::shared_ptr< Pipe::CPipe > _pipe_out = std::make_shared< Pipe::CNullPipe >();
public:
  /**
   * @brief Sets the input pipe for the encoder.
   * @param pipe_in_ Shared pointer to the input pipe.
   */
  void  SetIn ( std::shared_ptr< Pipe::CPipe > pipe_in_  );
  
  /**
   * @brief Sets the output pipe for the encoder.
   * @param pipe_out_ Shared pointer to the output pipe.
   */
  void  SetOut( std::shared_ptr< Pipe::CPipe > pipe_out_ );
  
  /**
   * @brief Encodes the data from the input pipe and writes it to the output pipe.
   */
  void  Encode();
};

/**
 * @class CZPackDecoder
 * @brief Class for decoding data streams encoded with ZPack compression methods.
 * 
 * This class provides methods to set input and output pipes and to perform 
 * the decoding of data streams that were compressed using Huffman encoding, 
 * RLE, or no compression.
 */
class CZPackDecoder {
  std::shared_ptr< Pipe::CPipe > _pipe_in  = std::make_shared< Pipe::CNullPipe >();
  std::shared_ptr< Pipe::CPipe > _pipe_out = std::make_shared< Pipe::CNullPipe >();
public:
  /**
   * @enum DecodeResult
   * @brief Enum representing the result of the decoding process.
   */
  enum DecodeResult{
    DECODE_OK,                   ///< Decoding was successful
    DECODE_ERR_INVALID_SIGNATURE,///< Invalid signature in the encoded data
    DECODE_INVALID_DATA,         ///< Invalid data in the encoded stream
  };
  
  /**
   * @brief Sets the input pipe for the decoder.
   * @param pipe_in_ Shared pointer to the input pipe.
   */
  void  SetIn ( std::shared_ptr< Pipe::CPipe > pipe_in_  );
  
  /**
   * @brief Sets the output pipe for the decoder.
   * @param pipe_out_ Shared pointer to the output pipe.
   */
  void  SetOut( std::shared_ptr< Pipe::CPipe > pipe_out_ );

  /**
   * @brief Decodes the data from the input pipe and writes it to the output pipe.
   * @return The result of the decoding process.
   */
  CZPackDecoder::DecodeResult Decode();
};

} // namespace ZPack
