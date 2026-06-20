/**
 * @file httpdrv.h
 * @brief HTTP(S) GET as a file driver, layered on the VHttp SCI bridge.
 *
 * The emulator host (js.b8/vhttp.js) performs the actual fetch(); this driver
 * is the ROM-side endpoint. Because the VFS open() does not receive the path,
 * the URL is supplied by writing to the stream, not by the path:
 *
 * @code
 *   #include <httpdrv.h>
 *
 *   http::Reset();                               // once, before first use
 *   FILE* fp = fopen("/http/con0", "r+");
 *   fwrite(url, 1, strlen(url), fp);             // the request URL
 *   fflush(fp);                                  // r+ sync point: send request
 *   int n = fread(buf, 1, buflen, fp);           // blocks until response (EOF)
 *   fclose(fp);
 * @endcode
 *
 * GET only for now; the response body is returned verbatim. HTTP status and
 * headers are intentionally not exposed (signal ok/reason in the JSON body);
 * a hard transport failure surfaces as fopen()==NULL or a zero-length read.
 *
 * fread() blocks the calling thread until the response arrives, yielding each
 * poll so the host fetch can progress. Call it from a worker thread (b8 OS
 * pthread) if the game must keep running while the request is in flight.
 */
#pragma once
namespace http {
  void  Reset();
};
