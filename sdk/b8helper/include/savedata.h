/**
 * @file savedata.h
 * @brief Persistent key-value save storage for a BEEP-8 app.
 *
 * A non-volatile storage device reached over the SCI link (host handler
 * js.b8/vsavedata.js). Backed by the browser's localStorage on the web host,
 * scoped per ROM so games don't clobber each other. This is the
 * cookie-equivalent for ROMs (which can't touch cookies directly). Use it to
 * remember the player's name, a local best-score copy, settings, etc.
 *
 * The VFS open() does not receive the path, so the command (and key/value) are
 * written to the stream, not encoded in the path:
 *
 * @code
 *   #include <savedata.h>
 *   savedata::Reset();                               // once, before first use
 *
 *   // save:
 *   FILE* fp = fopen("/savedata/con0", "r+");
 *   fwrite("set player_name=ZEP", 1, 19, fp);
 *   fclose(fp);
 *
 *   // load:
 *   char name[8] = {0};
 *   fp = fopen("/savedata/con0", "r+");
 *   fwrite("get player_name", 1, 15, fp);
 *   fflush(fp);                                       // r+ sync: send command
 *   int n = fread(name, 1, sizeof(name)-1, fp);       // value, or 0 bytes if absent
 *   fclose(fp);
 * @endcode
 *
 * localStorage is synchronous on the host, so the round-trip is ~1 frame;
 * calling this from the main thread is fine (unlike httpdrv, no worker needed).
 */
#pragma once
namespace savedata {
  void  Reset();
};
