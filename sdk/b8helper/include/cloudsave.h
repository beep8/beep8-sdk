/**
 * @file cloudsave.h
 * @brief Global (server-backed) key-value storage shared across all clients
 * of an app -- the networked counterpart to the browser-local SaveData.
 *
 * Values persist on the beep8.org VPS (service: beep8.cloudsave), not the
 * browser, so multiple players/devices can read and write the same data
 * (leaderboards, shared galleries, etc.). A "key" is a 16-character alnum
 * string the app developer picks and hardcodes in source: it doubles as both
 * the storage slot's name and its de facto access credential -- keep it
 * secret for private data, or publish/share it to let multiple apps read the
 * same slot. There is no per-key size limit, only a small service-wide cap;
 * the server serializes writes to the same key so concurrent submitters
 * don't lose updates.
 *
 * Values are always opaque strings server-side. To store binary (e.g. a PNG),
 * base64url-encode it first (see base64.h) -- Set()/Get() do not percent-
 * escape the payload, so it must already be URL-safe (no '&', '=', '?', '#',
 * '%', space, or raw control bytes; base64url output already qualifies).
 * The payload also rides as a C string internally (the request URL is sent
 * via strlen()), so it must not contain an embedded NUL byte -- again,
 * base64url output never does.
 *
 * Transport rides the existing /http file driver (see httpdrv.h), chunked
 * internally in ~4000-byte pieces to stay under the 8KB SCI FIFO that carries
 * requests and responses -- callers never see the chunking.
 *
 * @code
 *   #include <cloudsave.h>
 *   cloudsave::Set( "PIXARTGALLERY001", "hello", 5 );
 *
 *   char buf[64];
 *   int  n = cloudsave::Get( "PIXARTGALLERY001", buf, sizeof(buf) );  // n==5
 * @endcode
 */
#pragma once

namespace cloudsave {

  // Fetch the value stored under `key` (must match ^[0-9A-Za-z]{16}$) into
  // buf (NUL-terminated, up to bufcap-1 bytes; longer values are truncated).
  // Returns the byte length copied (>=0; 0 if the key was never set), or -1
  // on a bad key or transport error. Blocking; may issue multiple HTTP
  // round-trips for values larger than ~4000 bytes.
  int   Get( const char* key, char* buf, int bufcap );

  // Store `value` (len bytes, URL-safe -- see file comment) under `key`
  // (must match ^[0-9A-Za-z]{16}$), replacing any previous value. Returns
  // true on success, false on a bad key, transport error, or the service's
  // storage cap being full. Blocking; chunks the upload internally.
  bool  Set( const char* key, const char* value, int len );

}
