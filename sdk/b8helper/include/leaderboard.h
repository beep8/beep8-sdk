/**
 * @file leaderboard.h
 * @brief Minimal client for the BEEP-8 score server, over the /http file driver.
 *
 * Fetches a daily/all-time leaderboard for display. Blocking (uses httpdrv):
 * call from a worker thread if the game must keep running during the request,
 * or at a screen where a brief stall is acceptable (e.g. boot / title).
 *
 * @code
 *   leaderboard::Entry top[5];
 *   int n = leaderboard::board("1d-pacman", leaderboard::DAILY, top, 5);
 *   for(int i=0;i<n;++i) printf("%s %d\n", top[i].name, top[i].score);
 * @endcode
 */
#pragma once

namespace leaderboard {

  enum Window { DAILY = 0, ALLTIME = 1 };

  struct Entry {
    char name[8];
    int  score;
  };

  // Fetch a board into out[0..max). Returns the entry count (>=0), or -1 on
  // a transport/parse error. Blocking.
  int board( const char* game, int window, Entry* out, int max );

}
