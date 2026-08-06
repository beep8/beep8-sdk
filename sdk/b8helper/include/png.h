/**
 * @file png.h
 * @brief Minimal, dependency-free PNG encoder for small indexed-color images.
 *
 * Emits a valid PNG (color type 3, 8-bit palette indices) from an array of
 * per-pixel color indices plus an RGB palette. Aimed at BEEP-8's tiny
 * PICO-8-palette sprites (e.g. PixArt's 128x128 canvas), where the source data
 * is *already* palette indices, so an indexed PNG round-trips the exact colors
 * with no quantization and stays small.
 *
 * The zlib/deflate stream uses **stored (uncompressed) blocks only** -- no real
 * compression. For a 128x128 image that is ~16.6 KB, which is fine here and
 * keeps the codec small enough to live entirely ROM-side (no host/JS bridge).
 * CRC-32 (chunk checksums) and Adler-32 (zlib checksum) are computed inline.
 *
 * The output is a plain byte buffer; the caller decides what to do with it
 * (base64 + cloudsave::Set for a cloud save, a host download driver for a local
 * save, etc.). Nothing here touches SCI or the filesystem.
 *
 * @code
 *   #include <png.h>
 *   uint8_t out[PNG_ENCODE_CAP(128, 128)];
 *   int n = png::EncodeIndexed(&canvas[0][0], 128, 128,
 *                              png::kPico8Palette, 16, out, sizeof(out));
 *   // n > 0 -> `out[0..n)` is a complete .png file
 * @endcode
 */
#pragma once
#include <stdint.h>

// Worst-case encoded size for a `w` x `h` 8-bit indexed image with a 16-entry
// palette, using stored deflate blocks. Safe as a fixed buffer size. Breakdown:
//   8  PNG signature
//   25 IHDR chunk (12 overhead + 13 data)
//   60 PLTE chunk (12 overhead + 48 data, 16 RGB triples)
//   IDAT: 12 overhead + 2 zlib header + 4 adler + deflate(stored) of the raw
//         image. Raw image = h rows of (1 filter byte + w) = h*(w+1). Stored
//         deflate adds a 5-byte header per <=65535-byte block.
//   12 IEND chunk
#define PNG_ENCODE_CAP(w, h) \
  (8 + 25 + 60 + 12 + 2 + 4 + \
   ((h) * ((w) + 1)) + 5 * (1 + ((h) * ((w) + 1)) / 65535) + 12)

namespace png {

  // The 16 default PICO-8 colors as R,G,B triples (48 bytes), index order
  // matching pico8::Color / the emulator palette (see Ppu.js). Handy default
  // palette for BEEP-8 sprites; pass to EncodeIndexed as `palette`.
  extern const uint8_t kPico8Palette[16 * 3];

  // Encode a `w` x `h` 8-bit indexed image to a PNG in `out`.
  //   idx      : w*h bytes, row-major (top-to-bottom), each 0..palette_count-1.
  //   palette  : palette_count RGB triples (3 bytes each), palette_count 1..256.
  //   out      : caller buffer; use PNG_ENCODE_CAP(w,h) to size it.
  //   out_cap  : capacity of `out` in bytes.
  // Returns the number of bytes written (a complete PNG), or 0 on bad arguments
  // or if `out_cap` is too small. Does not allocate.
  int EncodeIndexed(const uint8_t* idx, int w, int h,
                    const uint8_t* palette, int palette_count,
                    uint8_t* out, int out_cap);

  // Decode an indexed PNG (as produced by EncodeIndexed) back into palette
  // indices. Handles color type 3, 8-bit, all five PNG row filters, and a
  // single stored-deflate IDAT -- i.e. exactly what EncodeIndexed emits (the
  // encoder writes filter 0 only, but the decoder accepts 0..4). Chunk CRC-32
  // and the zlib Adler-32 are verified. Compressed (huffman) deflate and
  // multi-IDAT streams are *not* supported.
  //   png, png_len : the PNG byte stream.
  //   idx, idx_cap : output buffer, receives w*h row-major index bytes.
  //   w_out, h_out : receive the decoded dimensions (either may be null).
  // Returns the number of index bytes written (w*h, > 0), or 0 on malformed
  // input, an unsupported feature, a CRC/Adler mismatch, or idx_cap too small.
  // Does not allocate.
  int DecodeIndexed(const uint8_t* png, int png_len,
                    uint8_t* idx, int idx_cap,
                    int* w_out, int* h_out);

}
