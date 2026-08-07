/**
 * @file download.h
 * @brief Trigger a browser file download from a BEEP-8 ROM.
 *
 * A one-way byte sink reached over the SCI link (host handler
 * js.b8/vdownload.js). Bytes written to /download/con0 are streamed to the
 * host, which saves them to the user's device as a file (via an <a download>
 * Blob). There is no BEEP-8 "local save" API otherwise; this is the ROM-side
 * counterpart of the browser Save-As.
 *
 * Wire framing (defined by the host vdownload.js): a payload is an ASCII
 * decimal byte-count terminated by '\n', immediately followed by exactly that
 * many raw bytes. The host counts the bytes, and once it has them all, offers
 * the file for download. Because the length is known up front, the payload may
 * contain any byte (including 0x00 / 0xff) -- unlike clipboard.h, there is no
 * in-band terminator.
 *
 * Flow control is the CALLER's responsibility: the SCI FIFO is 8 KB and is
 * drained by the host once per frame, so write at most a few KB per frame and
 * spread a larger payload (a 16 KB PNG) across several frames -- see PixArt's
 * download state machine. Writing a whole 16 KB blob in one shot overflows the
 * FIFO (oldest bytes are dropped) and corrupts the file.
 *
 * @code
 *   #include <download.h>
 *   download::Reset();                       // once, before first use
 *   // then, paced across frames:
 *   FILE* fp = fopen("/download/con0", "wb");
 *   fwrite(chunk, 1, chunk_len, fp);         // header "<n>\n" first, then bytes
 *   fclose(fp);
 * @endcode
 */
#pragma once
namespace download {
  void  Reset();
};
