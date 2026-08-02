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

} // namespace png
