/**
 * @file base64.h
 * @brief Base64url (RFC 4648 §5) encode/decode -- no padding, URL-safe alphabet.
 *
 * Uses '-' and '_' in place of '+' and '/' and omits '=' padding, so the
 * output can be embedded directly in a URL query string with no percent-
 * escaping (same convention already used by the score server's submit
 * token, see b8helper/leaderboard.cpp). Intended companion to cloudsave.h
 * for storing binary blobs (e.g. PNG bytes) as opaque strings.
 */
#pragma once

namespace base64 {

  // Encoded length (excluding the NUL terminator) for `len` input bytes.
  int encodedLen( int len );

  // Decoded length (excluding the NUL terminator) for `len` input chars.
  int decodedLen( int len );

  // Encode src[0..srclen) into dst as base64url text + NUL terminator.
  // dst must be at least encodedLen(srclen)+1 bytes. Returns the encoded
  // length (excluding the NUL).
  int encode( const void* src, int srclen, char* dst, int dstcap );

  // Decode src[0..srclen) (base64url text, no padding) into dst bytes.
  // dst must be at least decodedLen(srclen) bytes. Returns the decoded
  // byte count, or -1 on malformed input.
  int decode( const char* src, int srclen, void* dst, int dstcap );

}
