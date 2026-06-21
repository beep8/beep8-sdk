// Minimal score-server client over the /http file driver. See leaderboard.h.
#include <leaderboard.h>
#include <httpdrv.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// dbg score server. Production is :8083.
#define LB_BASE "https://beep8.org:9083"

static int http_get( const char* url, char* buf, int buflen ){
  http::Reset();                       // idempotent: register /http on first use
  FILE* fp = fopen( "/http/con0", "r+" );
  if( !fp ) return -1;
  fwrite( url, 1, strlen(url), fp );
  fflush( fp );
  int n = (int)fread( buf, 1, buflen - 1, fp );
  if( n < 0 ) n = 0;
  buf[ n ] = 0;
  fclose( fp );
  return n;
}

// Parse {"board":[{"name":"ZEP","score":9800},...]} into out[0..max).
static int parse_board( const char* json, leaderboard::Entry* out, int max ){
  const char* p = strstr( json, "\"board\"" );
  if( !p ) return 0;
  int n = 0;
  while( n < max ){
    p = strstr( p, "\"name\":\"" );
    if( !p ) break;
    p += 8;                            // past "name":"
    int i = 0;
    while( *p && *p != '"' && i < (int)sizeof(out[n].name) - 1 )
      out[n].name[ i++ ] = *p++;
    out[n].name[ i ] = 0;

    const char* s = strstr( p, "\"score\":" );
    if( !s ) break;
    s += 8;                            // past "score":
    out[n].score = atoi( s );
    p = s;
    ++n;
  }
  return n;
}

int leaderboard::board( const char* game, int window, Entry* out, int max ){
  char url[160];
  snprintf( url, sizeof(url), "%s/board?game=%s&window=%s",
            LB_BASE, game, window == ALLTIME ? "alltime" : "daily" );

  static char buf[2048];
  int n = http_get( url, buf, sizeof(buf) );
  if( n <= 0 ) return -1;
  if( !strstr( buf, "\"ok\":true" ) ) return -1;
  return parse_board( buf, out, max );
}
