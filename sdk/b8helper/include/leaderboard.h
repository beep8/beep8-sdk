/**
 * @file leaderboard.h
 * @brief Minimal client for the BEEP-8 score server, over the /http file driver.
 *
 * Fetches a daily/weekly/all-time leaderboard for display. Blocking (uses
 * httpdrv): call from a worker thread if the game must keep running during
 * the request, or at a screen where a brief stall is acceptable (e.g. boot /
 * title).
 *
 * @code
 *   leaderboard::Entry top[5];
 *   int n = leaderboard::board("1d-pacman", leaderboard::WEEKLY, top, 5);
 *   for(int i=0;i<n;++i) printf("%s %d\n", top[i].name, top[i].score);
 * @endcode
 */
#pragma once

namespace leaderboard {

  enum Window { DAILY = 0, ALLTIME = 1, WEEKLY = 2 };

  struct Entry {
    char name[8];
    int  score;
  };

  // Fetch a board into out[0..max). Returns the entry count (>=0), or -1 on
  // a transport/parse error. Blocking.
  int board( const char* game, int window, Entry* out, int max );

  // Begin a run: fetch a single-use submit token (and optionally the daily
  // seed). Blocking. Returns true on success and writes a NUL-terminated token
  // to tokenOut (needs ~160 bytes); seedOut may be null.
  bool start( const char* game, char* tokenOut, int tokenCap, unsigned* seedOut );

  // Submit a score with a token from start(). `window` (DAILY or WEEKLY)
  // picks which periodic board the entry is added to; ALLTIME is always
  // updated regardless. Blocking. Returns the new board entry count (>=0)
  // and fills out[0..max) with the updated Top board, or -1 on a
  // transport/parse error. The 1-based rank is written to *rankOut (>0 if
  // placed) when rankOut is non-null.
  int submit( const char* game, const char* name, int score, const char* token,
              int window, Entry* out, int max, int* rankOut );

}
