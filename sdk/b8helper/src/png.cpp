// Minimal indexed-color PNG encoder. See png.h.
//
// Output is a color-type-3 (palette) 8-bit PNG whose IDAT is a zlib stream of
// *stored* (uncompressed) deflate blocks -- no compression, just framing. This
// keeps the whole codec tiny and self-contained (no zlib, no host bridge) at
// the cost of ~1:1 size, which is fine for BEEP-8's small sprites.
#include <png.h>

namespace png {

const uint8_t kPico8Palette[16 * 3] = {
  0x00,0x00,0x00, 0x1D,0x2B,0x53, 0x7E,0x25,0x53, 0x00,0x87,0x51,
  0xAB,0x52,0x36, 0x5F,0x57,0x4F, 0xC2,0xC3,0xC7, 0xFF,0xF1,0xE8,
  0xFF,0x00,0x4D, 0xFF,0xA3,0x00, 0xFF,0xEC,0x27, 0x00,0xE4,0x36,
  0x29,0xAD,0xFF, 0x83,0x76,0x9C, 0xFF,0x77,0xA8, 0xFF,0xCC,0xAA,
};

namespace {

// CRC-32 (reflected, poly 0xEDB88320), computed a byte at a time (no table).
inline uint32_t crc32_byte(uint32_t c, uint8_t b) {
  c ^= b;
  for (int k = 0; k < 8; ++k) {
    const uint32_t m = (uint32_t)0 - (c & 1u);   // 0xFFFFFFFF if LSB set, else 0
    c = (c >> 1) ^ (0xEDB88320u & m);
  }
  return c;
}

// Sequential PNG byte writer. Bounds-checked; `ok` latches false on overflow.
// While `crc_on`, each emitted byte folds into the running chunk CRC.
struct Writer {
  uint8_t* p;
  int      cap;
  int      pos = 0;
  bool     ok  = true;
  uint32_t crc = 0;
  bool     crc_on = false;

  Writer(uint8_t* buf, int buf_cap) : p(buf), cap(buf_cap) {}

  void put(uint8_t b) {
    if (pos < cap) {
      p[pos++] = b;
      if (crc_on) crc = crc32_byte(crc, b);
    } else {
      ok = false;
    }
  }
  void be32(uint32_t v) {                    // 32-bit big-endian
    put((uint8_t)(v >> 24)); put((uint8_t)(v >> 16));
    put((uint8_t)(v >> 8));  put((uint8_t)(v));
  }
  // A chunk is LENGTH(4) TYPE(4) DATA CRC(4); CRC covers TYPE+DATA only.
  void beginChunk(uint32_t len, const char* type) {
    be32(len);                               // crc_on still false: length excluded
    crc = 0xFFFFFFFFu; crc_on = true;
    for (int i = 0; i < 4; ++i) put((uint8_t)type[i]);
  }
  void endChunk() {
    crc_on = false;                          // the CRC field itself isn't CRC'd
    be32(crc ^ 0xFFFFFFFFu);
  }
};

// ---- decode helpers --------------------------------------------------------

inline uint32_t rd_be32(const uint8_t* p) {
  return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
         ((uint32_t)p[2] << 8)  |  (uint32_t)p[3];
}

inline bool type_is(const uint8_t* t, const char* s) {
  return t[0] == (uint8_t)s[0] && t[1] == (uint8_t)s[1] &&
         t[2] == (uint8_t)s[2] && t[3] == (uint8_t)s[3];
}

// CRC-32 over TYPE+DATA (len bytes) compared against the chunk's trailing CRC.
inline bool crc_ok(const uint8_t* type_and_data, int len, uint32_t expect) {
  uint32_t c = 0xFFFFFFFFu;
  for (int i = 0; i < len; ++i) c = crc32_byte(c, type_and_data[i]);
  return (c ^ 0xFFFFFFFFu) == expect;
}

// PNG Paeth predictor on the three neighbour bytes (left, up, up-left).
inline int paeth(int a, int b, int c) {
  int p = a + b - c;
  int pa = p - a; if (pa < 0) pa = -pa;
  int pb = p - b; if (pb < 0) pb = -pb;
  int pc = p - c; if (pc < 0) pc = -pc;
  if (pa <= pb && pa <= pc) return a;
  return (pb <= pc) ? b : c;
}

// ---- DEFLATE (RFC 1951) inflate --------------------------------------------
//
// A compact inflate supporting stored, fixed- and dynamic-Huffman blocks and
// decoding across several consecutive IDAT chunks (PNG splits its zlib stream
// over as many IDATs as it likes). The canonical-Huffman build/decode follows
// the public-domain puff.c (Mark Adler). The inflated bytes go straight into
// the caller's output buffer, which also serves as the LZ77 window -- a PNG's
// back-references never reach past the image data already produced.

// A bit source that walks the IDAT chunk sequence of a PNG, LSB-first.
struct BitReader {
  const uint8_t* file;      // whole PNG
  int            file_len;
  const uint8_t* cur;       // cursor within the current IDAT payload
  int            cur_rem;   // bytes left in the current IDAT payload
  int            next_p;    // offset of the chunk header after the current IDAT
  uint32_t       bitbuf;    // buffered bits (LSB = next bit out)
  int            bitcnt;    // number of valid bits in bitbuf
  bool           err;
};

// `first_idat_p` is the offset of the first IDAT chunk's LENGTH field.
void br_init(BitReader* br, const uint8_t* file, int file_len, int first_idat_p) {
  br->file = file; br->file_len = file_len;
  br->bitbuf = 0; br->bitcnt = 0; br->err = false;
  const uint32_t clen = rd_be32(file + first_idat_p);
  br->cur     = file + first_idat_p + 8;
  br->cur_rem = (int)clen;
  br->next_p  = first_idat_p + 12 + (int)clen;   // skip this chunk incl. its CRC
}

// Next raw byte of the concatenated IDAT payloads, or -1 at the end of the
// IDAT run (a non-IDAT chunk or EOF).
int br_next_byte(BitReader* br) {
  while (br->cur_rem == 0) {
    const int p = br->next_p;
    if (p + 12 > br->file_len) return -1;
    const uint32_t clen = rd_be32(br->file + p);
    if (clen > (uint32_t)(br->file_len - p - 12)) { br->err = true; return -1; }
    if (!type_is(br->file + p + 4, "IDAT")) return -1;   // end of IDAT sequence
    br->cur     = br->file + p + 8;
    br->cur_rem = (int)clen;
    br->next_p  = p + 12 + (int)clen;
  }
  br->cur_rem--;
  return *br->cur++;
}

// Read `n` bits (n <= 24), LSB-first. Latches err on underflow.
int br_bits(BitReader* br, int n) {
  while (br->bitcnt < n) {
    const int b = br_next_byte(br);
    if (b < 0) { br->err = true; return 0; }
    br->bitbuf |= (uint32_t)b << br->bitcnt;
    br->bitcnt += 8;
  }
  const int v = (int)(br->bitbuf & (((uint32_t)1 << n) - 1));
  br->bitbuf >>= n;
  br->bitcnt -= n;
  return v;
}

// Discard bits up to the next byte boundary (keeps whole buffered bytes).
void br_align(BitReader* br) {
  const int drop = br->bitcnt & 7;
  br->bitbuf >>= drop;
  br->bitcnt -= drop;
}

// Canonical Huffman table: count[len] = #codes of that length, symbol[] holds
// the symbols sorted by (length, symbol). Enough room for the literal/length
// alphabet (288). Returns 0 (complete), >0 (incomplete/empty), <0 (invalid).
struct Huff { short count[16]; short symbol[288]; };

int huff_build(Huff* h, const uint8_t* lengths, int n) {
  for (int i = 0; i < 16; ++i) h->count[i] = 0;
  for (int i = 0; i < n; ++i) h->count[lengths[i]]++;
  h->count[0] = 0;                                  // length 0 = unused symbol
  int left = 1;                                     // over/under-subscription
  for (int len = 1; len < 16; ++len) { left <<= 1; left -= h->count[len]; if (left < 0) return -1; }
  short offs[16]; offs[1] = 0;
  for (int len = 1; len < 15; ++len) offs[len + 1] = offs[len] + h->count[len];
  for (int i = 0; i < n; ++i) if (lengths[i]) h->symbol[offs[lengths[i]]++] = (short)i;
  return left;                                      // >0 => incomplete code
}

int huff_decode(BitReader* br, const Huff* h) {
  int code = 0, first = 0, index = 0;
  for (int len = 1; len <= 15; ++len) {
    code |= br_bits(br, 1);
    if (br->err) return -1;
    const int count = h->count[len];
    if (code - count < first) return h->symbol[index + (code - first)];
    index += count;
    first  = (first + count) << 1;
    code <<= 1;
  }
  return -1;
}

// RFC 1951 length / distance base + extra-bit tables.
const short kLenBase[29]  = {3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258};
const short kLenExtra[29] = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};
const short kDistBase[30] = {1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577};
const short kDistExtra[30]= {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

// Decode one Huffman-coded block's symbols into out[*pos..]. Returns 0/-1.
int inflate_codes(BitReader* br, const Huff* ll, const Huff* dh,
                  uint8_t* out, int out_cap, int* pos_io) {
  int pos = *pos_io;
  for (;;) {
    const int sym = huff_decode(br, ll);
    if (sym < 0) return -1;
    if (sym == 256) break;                          // end of block
    if (sym < 256) {
      if (pos >= out_cap) return -1;
      out[pos++] = (uint8_t)sym;
    } else {
      const int ls = sym - 257;
      if (ls >= 29) return -1;
      const int length = kLenBase[ls] + br_bits(br, kLenExtra[ls]);
      const int ds = huff_decode(br, dh);
      if (ds < 0 || ds >= 30) return -1;
      const int dist = kDistBase[ds] + br_bits(br, kDistExtra[ds]);
      if (br->err || dist > pos || pos + length > out_cap) return -1;
      for (int i = 0; i < length; ++i) { out[pos] = out[pos - dist]; ++pos; }
    }
    if (br->err) return -1;
  }
  *pos_io = pos;
  return 0;
}

// Inflate the DEFLATE stream into out (<= out_cap). Returns bytes produced, or
// -1. The caller has already consumed the 2-byte zlib header.
int inflate(BitReader* br, uint8_t* out, int out_cap) {
  int pos = 0, final = 0;
  do {
    final = br_bits(br, 1);
    const int type = br_bits(br, 2);
    if (br->err) return -1;
    if (type == 0) {                                // stored (uncompressed)
      br_align(br);
      const int len  = br_bits(br, 8) | (br_bits(br, 8) << 8);
      const int nlen = br_bits(br, 8) | (br_bits(br, 8) << 8);
      if (br->err || (len ^ 0xffff) != nlen || pos + len > out_cap) return -1;
      for (int i = 0; i < len; ++i) {
        const int b = br_bits(br, 8);
        if (br->err) return -1;
        out[pos++] = (uint8_t)b;
      }
    } else if (type == 1) {                         // fixed Huffman
      uint8_t ll_len[288], d_len[30];
      for (int i = 0;   i < 144; ++i) ll_len[i] = 8;
      for (int i = 144; i < 256; ++i) ll_len[i] = 9;
      for (int i = 256; i < 280; ++i) ll_len[i] = 7;
      for (int i = 280; i < 288; ++i) ll_len[i] = 8;
      for (int i = 0;   i < 30;  ++i) d_len[i]  = 5;
      Huff ll, dh;
      huff_build(&ll, ll_len, 288);
      huff_build(&dh, d_len, 30);
      if (inflate_codes(br, &ll, &dh, out, out_cap, &pos)) return -1;
    } else if (type == 2) {                         // dynamic Huffman
      const int hlit  = br_bits(br, 5) + 257;
      const int hdist = br_bits(br, 5) + 1;
      const int hclen = br_bits(br, 4) + 4;
      if (br->err || hlit > 286 || hdist > 30) return -1;
      static const uint8_t ord[19] =
        {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
      uint8_t cl_len[19] = {0};
      for (int i = 0; i < hclen; ++i) cl_len[ord[i]] = (uint8_t)br_bits(br, 3);
      Huff clh;
      if (br->err || huff_build(&clh, cl_len, 19) < 0) return -1;
      uint8_t lens[286 + 30] = {0};
      const int total = hlit + hdist;
      int n = 0;
      while (n < total) {
        const int sym = huff_decode(br, &clh);
        if (sym < 0) return -1;
        if (sym < 16) {
          lens[n++] = (uint8_t)sym;
        } else if (sym == 16) {                     // copy previous 3..6 times
          if (n == 0) return -1;
          int r = br_bits(br, 2) + 3;
          while (r-- && n < total) { lens[n] = lens[n - 1]; ++n; }
        } else if (sym == 17) {                     // repeat zero 3..10
          int r = br_bits(br, 3) + 3;
          while (r-- && n < total) lens[n++] = 0;
        } else {                                    // 18: repeat zero 11..138
          int r = br_bits(br, 7) + 11;
          while (r-- && n < total) lens[n++] = 0;
        }
        if (br->err) return -1;
      }
      Huff ll, dh;
      if (huff_build(&ll, lens, hlit) < 0) return -1;
      huff_build(&dh, lens + hlit, hdist);          // dist code may be incomplete
      if (inflate_codes(br, &ll, &dh, out, out_cap, &pos)) return -1;
    } else {
      return -1;                                    // type 3 is reserved
    }
    if (br->err) return -1;
  } while (!final);
  return pos;
}

// Unpack a reconstructed scanline of `w` samples at `bitdepth` (1/2/4/8,
// MSB-first packing) into one index byte per pixel.
void unpack_row(const uint8_t* src, int w, int bitdepth, uint8_t* dst) {
  if (bitdepth == 8) { for (int x = 0; x < w; ++x) dst[x] = src[x]; return; }
  const int per  = 8 / bitdepth;
  const int mask = (1 << bitdepth) - 1;
  for (int x = 0; x < w; ++x) {
    const int shift = 8 - bitdepth - (x % per) * bitdepth;
    dst[x] = (uint8_t)((src[x / per] >> shift) & mask);
  }
}

} // namespace

int EncodeIndexed(const uint8_t* idx, int w, int h,
                  const uint8_t* palette, int palette_count,
                  uint8_t* out, int out_cap) {
  if (!idx || !palette || !out) return 0;
  if (w <= 0 || h <= 0) return 0;
  if (palette_count < 1 || palette_count > 256) return 0;

  // Raw filtered image = h rows of (1 filter byte + w pixel bytes).
  const int raw_len = h * (w + 1);
  const int nblocks = (raw_len + 65534) / 65535;   // stored blocks, <=65535 each
  const int deflate_len = raw_len + 5 * nblocks;   // +5-byte header per block
  const int zlib_len = 2 + deflate_len + 4;        // 2 hdr + deflate + 4 adler
  const int needed = 8 + (12 + 13) + (12 + 3 * palette_count) +
                     (12 + zlib_len) + 12;
  if (needed > out_cap) return 0;

  Writer W(out, out_cap);

  // Signature.
  static const uint8_t sig[8] = {0x89,'P','N','G',0x0d,0x0a,0x1a,0x0a};
  for (int i = 0; i < 8; ++i) W.put(sig[i]);

  // IHDR: width, height, bit depth (8), color type (3=indexed), the three
  // fixed zeros (deflate / no-filter-adaptive / no-interlace).
  W.beginChunk(13, "IHDR");
  W.be32((uint32_t)w);
  W.be32((uint32_t)h);
  W.put(8); W.put(3); W.put(0); W.put(0); W.put(0);
  W.endChunk();

  // PLTE: the RGB triples, verbatim.
  W.beginChunk((uint32_t)(3 * palette_count), "PLTE");
  for (int i = 0; i < 3 * palette_count; ++i) W.put(palette[i]);
  W.endChunk();

  // IDAT: zlib stream = header(0x78 0x01) + stored deflate blocks + Adler-32.
  W.beginChunk((uint32_t)zlib_len, "IDAT");
  W.put(0x78); W.put(0x01);
  uint32_t a = 1, b = 0;                            // Adler-32 accumulators
  int off = 0, r = 0, col = 0;
  while (off < raw_len) {
    int blen = raw_len - off;
    if (blen > 65535) blen = 65535;
    const uint8_t bfinal = (off + blen == raw_len) ? 1 : 0;   // BTYPE=00
    W.put(bfinal);
    W.put((uint8_t)(blen & 0xff));  W.put((uint8_t)((blen >> 8) & 0xff));
    const uint16_t nlen = (uint16_t)~blen;
    W.put((uint8_t)(nlen & 0xff));  W.put((uint8_t)((nlen >> 8) & 0xff));
    for (int j = 0; j < blen; ++j) {
      const uint8_t v = (col == 0) ? 0 : idx[r * w + (col - 1)];  // filter byte / pixel
      W.put(v);
      a = (a + v) % 65521u;
      b = (b + a) % 65521u;
      if (++col == w + 1) { col = 0; ++r; }
    }
    off += blen;
  }
  W.be32((b << 16) | a);                           // Adler-32, big-endian
  W.endChunk();

  // IEND: empty.
  W.beginChunk(0, "IEND");
  W.endChunk();

  return W.ok ? W.pos : 0;
}

int DecodeIndexed(const uint8_t* png, int png_len,
                  uint8_t* idx, int idx_cap, int* w_out, int* h_out,
                  uint8_t* scratch, int scratch_cap) {
  if (!png || !idx || !scratch || png_len < 8) return 0;
  static const uint8_t sig[8] = {0x89,'P','N','G',0x0d,0x0a,0x1a,0x0a};
  for (int i = 0; i < 8; ++i) if (png[i] != sig[i]) return 0;

  // Walk the chunk list: capture IHDR geometry and the offset of the first
  // IDAT chunk (the zlib stream may span several consecutive IDATs), verifying
  // every chunk CRC as we go.
  int w = 0, h = 0, bitdepth = 0; bool have_ihdr = false;
  int first_idat_p = 0;
  int p = 8;
  while (p + 12 <= png_len) {
    const uint32_t clen = rd_be32(png + p);
    if (clen > (uint32_t)(png_len - p - 12)) return 0;   // truncated chunk
    const uint8_t* type = png + p + 4;
    const uint8_t* data = png + p + 8;
    if (!crc_ok(type, 4 + (int)clen, rd_be32(data + clen))) return 0;

    if (type_is(type, "IHDR")) {
      if (clen != 13) return 0;
      w = (int)rd_be32(data); h = (int)rd_be32(data + 4);
      bitdepth = data[8];
      if (w <= 0 || h <= 0) return 0;
      // indexed colour (type 3), bit depth 1/2/4/8, no interlace, and the two
      // fixed zeros (deflate compression / adaptive filtering).
      if (data[9] != 3) return 0;
      if (!(bitdepth == 1 || bitdepth == 2 || bitdepth == 4 || bitdepth == 8)) return 0;
      if (data[10] || data[11] || data[12]) return 0;
      if (w > idx_cap / h) return 0;                     // w*h must fit idx_cap
      have_ihdr = true;
    } else if (type_is(type, "IDAT")) {
      if (!first_idat_p) first_idat_p = p;               // remember the first
    } else if (type_is(type, "IEND")) {
      break;
    }
    p += 12 + (int)clen;
  }
  if (!have_ihdr || !first_idat_p) return 0;

  // The unfiltered image (one filter byte + packed samples per row) is the
  // inflate output; it must fit `scratch`.
  const int rowbytes = (w * bitdepth + 7) / 8;
  const int raw_len  = h * (1 + rowbytes);
  if (raw_len > scratch_cap) return 0;

  // Inflate the zlib stream (2-byte header, DEFLATE body, 4-byte Adler-32).
  BitReader br;
  br_init(&br, png, png_len, first_idat_p);
  const int cmf = br_bits(&br, 8);
  const int flg = br_bits(&br, 8);
  if (br.err) return 0;
  if ((cmf & 0x0f) != 8) return 0;                       // CM must be deflate
  if (((cmf << 8) | flg) % 31 != 0) return 0;            // FCHECK
  if (flg & 0x20) return 0;                              // no preset dictionary
  if (inflate(&br, scratch, raw_len) != raw_len) return 0;

  // Verify the byte-aligned Adler-32 trailer over the inflated bytes.
  br_align(&br);
  uint32_t adler = 0;
  for (int i = 0; i < 4; ++i) adler = (adler << 8) | (uint32_t)br_bits(&br, 8);
  if (br.err) return 0;
  uint32_t sa = 1, sb = 0;
  for (int i = 0; i < raw_len; ++i) { sa = (sa + scratch[i]) % 65521u; sb = (sb + sa) % 65521u; }
  if (((sb << 16) | sa) != adler) return 0;

  // Reverse the row filters in place (indexed => 1-byte filter unit), then
  // unpack each reconstructed scanline into one index byte per pixel.
  uint8_t* prev = nullptr;
  uint8_t* row  = scratch;
  for (int y = 0; y < h; ++y) {
    const int filt = row[0];
    if (filt > 4) return 0;
    uint8_t* cur = row + 1;
    for (int i = 0; i < rowbytes; ++i) {
      const int a = (i >= 1)             ? cur[i - 1]  : 0;
      const int b = prev                 ? prev[i]     : 0;
      const int c = (prev && i >= 1)     ? prev[i - 1] : 0;
      int v = cur[i];
      switch (filt) {
        case 1: v += a;                 break;
        case 2: v += b;                 break;
        case 3: v += (a + b) >> 1;      break;
        case 4: v += paeth(a, b, c);    break;
        default: /* 0 = None */         break;
      }
      cur[i] = (uint8_t)v;
    }
    unpack_row(cur, w, bitdepth, idx + y * w);
    prev = cur;
    row += 1 + rowbytes;
  }

  if (w_out) *w_out = w;
  if (h_out) *h_out = h;
  return w * h;
}

} // namespace png
