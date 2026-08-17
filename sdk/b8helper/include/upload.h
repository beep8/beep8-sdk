/**
 * @file upload.h
 * @brief Receive a local file (a PNG) from the browser into a BEEP-8 ROM.
 *
 * The host-side counterpart of a browser "Open File...": the ROM-facing half of
 * a one-shot upload over the SCI link (host handler js.b8/vupload.js). It is the
 * inverse of download.h -- bytes flow host -> ROM.
 *
 * Because a browser file picker must be opened from a real user gesture (which
 * the async ROM->host request is not), the flow is non-blocking and host-driven:
 *
 *   1. The ROM calls Begin(), which sends an "open" request to the host.
 *   2. The host shows a small "click to choose a PNG" overlay; the user's click
 *      (a genuine gesture) opens the file picker.
 *   3. The chosen file's bytes are streamed back, length-prefixed
 *      ("<len>\n" then exactly <len> raw bytes -- binary-safe, no sentinel),
 *      paced a few KB per frame to respect the 8 KB SCI FIFO.
 *   4. The ROM calls Poll() every frame until it returns DONE / CANCELLED /
 *      ERROR. On DONE the buffer holds the whole file (out_len bytes).
 *
 * A payload larger than the caller's buffer, or a cancelled picker (host sends
 * length 0), resolves as ERROR / CANCELLED rather than blocking. Poll() never
 * blocks: it only drains whatever bytes have arrived so far.
 *
 * @code
 *   #include <upload.h>
 *   // on an "Import" button press:
 *   upload::Begin();
 *   // then, each frame:
 *   int n = 0;
 *   switch (upload::Poll(buf, sizeof(buf), &n)) {
 *     case upload::DONE:      // buf[0..n) is the file (e.g. a PNG to decode)
 *     case upload::CANCELLED: // user dismissed the picker
 *     case upload::ERROR:     // too big / malformed
 *     default: break;         // WAITING: keep polling next frame
 *   }
 * @endcode
 *
 * When the player runs inside an embedding page that owns the file (the AI
 * Playground opening PixArt on its sprite0.png tab), there is no picker and no
 * disk: Begin() then means "give me the page's copy", and the page can ask for
 * the ROM's data back at any time. Stat() exposes that wiring to the ROM so it
 * can behave like an editor for the page's file instead of a file dialog app.
 */
#pragma once
namespace upload {
  enum State { IDLE = 0, WAITING, DONE, CANCELLED, ERROR };

  // Flag bits in the 1-byte Stat() reply (see below).
  enum { STAT_HOSTED = 1,     // an embedding page owns our file I/O
         STAT_FLUSH  = 2 };   // ...and it is asking for the current data now

  // Ask the host to open a file picker. Clears any stale bytes first.
  void  Begin();

  // Ask the host how file I/O is wired up right now. The reply is a 1-byte
  // payload of STAT_* bits, read back through the ordinary Poll() (DONE with
  // *out_len == 1). Hosts older than this request stay silent, so treat "no
  // answer within a second or so" as "not hosted" -- Poll() cannot time that
  // out for you, since it only watches for a transfer stalling mid-flight.
  void  Stat();

  // Drain whatever has arrived this frame. Returns WAITING until the whole
  // payload is in `buf` (DONE, *out_len set), or CANCELLED / ERROR. Call once
  // per frame after Begin().
  State Poll(unsigned char* buf, int cap, int* out_len);
}
