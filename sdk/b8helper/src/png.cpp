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
                  uint8_t* idx, int idx_cap, int* w_out, int* h_out) {
  if (!png || !idx || png_len < 8) return 0;
  static const uint8_t sig[8] = {0x89,'P','N','G',0x0d,0x0a,0x1a,0x0a};
  for (int i = 0; i < 8; ++i) if (png[i] != sig[i]) return 0;

  // Walk the chunk list: capture IHDR geometry and the single IDAT payload,
  // verifying every chunk CRC as we go.
  int w = 0, h = 0; bool have_ihdr = false;
  const uint8_t* idat = nullptr; int idat_len = 0;
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
      if (w <= 0 || h <= 0) return 0;
      // bit depth 8, colour type 3, and the three fixed zeros (see encoder).
      if (data[8] != 8 || data[9] != 3 || data[10] || data[11] || data[12]) return 0;
      if (w > idx_cap / h) return 0;                     // w*h must fit idx_cap
      have_ihdr = true;
    } else if (type_is(type, "IDAT")) {
      if (idat) return 0;                                // single IDAT only
      idat = data; idat_len = (int)clen;
    } else if (type_is(type, "IEND")) {
      break;
    }
    p += 12 + (int)clen;
  }
  if (!have_ihdr || !idat || idat_len < 2 + 4) return 0;

  // zlib wrapper: 2-byte header, stored-deflate body, 4-byte Adler-32 trailer.
  const uint8_t* z = idat + 2;
  const int      zlen = idat_len - 2 - 4;
  const uint32_t adler_expect = rd_be32(idat + idat_len - 4);

  int zp = 0, rem = 0; bool final_seen = false;   // stored-block byte cursor
  uint32_t sa = 1, sb = 0;                         // Adler-32 over raw bytes
  const int stride = w;

  for (int y = 0; y < h; ++y) {
    int filt = -1;
    for (int col = 0; col < w + 1; ++col) {
      if (rem == 0) {                              // advance to the next block
        if (final_seen) return 0;
        if (zp + 5 > zlen) return 0;
        if (((z[zp] >> 1) & 3) != 0) return 0;     // BTYPE must be 00 (stored)
        if (z[zp] & 1) final_seen = true;          // BFINAL
        rem = z[zp + 1] | (z[zp + 2] << 8);        // LEN (NLEN not validated)
        zp += 5;
        if (rem == 0) return 0;                    // empty block mid-image
      }
      if (zp >= zlen) return 0;
      const uint8_t rb = z[zp++]; --rem;
      sa = (sa + rb) % 65521u; sb = (sb + sa) % 65521u;

      if (col == 0) { filt = rb; if (filt > 4) return 0; continue; }
      const int x    = col - 1;
      const int left = x > 0 ? idx[y * stride + x - 1] : 0;
      const int up   = y > 0 ? idx[(y - 1) * stride + x] : 0;
      const int ul   = (x > 0 && y > 0) ? idx[(y - 1) * stride + x - 1] : 0;
      int v = rb;
      switch (filt) {
        case 1: v = rb + left;                 break;
        case 2: v = rb + up;                   break;
        case 3: v = rb + ((left + up) >> 1);   break;
        case 4: v = rb + paeth(left, up, ul);  break;
        default: /* 0 = None */                break;
      }
      idx[y * stride + x] = (uint8_t)v;
    }
  }
  if (((sb << 16) | sa) != adler_expect) return 0;

  if (w_out) *w_out = w;
  if (h_out) *h_out = h;
  return w * h;
}

} // namespace png
