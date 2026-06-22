// Minimal score-server client over the /http file driver. See leaderboard.h.
#include <leaderboard.h>
#include <httpdrv.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Production score server. The dbg server is :9083.
#define LB_BASE "https://beep8.org:8083"

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

bool leaderboard::start( const char* game, char* tokenOut, int tokenCap,
                         unsigned* seedOut ){
  char url[160];
  snprintf( url, sizeof(url), "%s/start?game=%s", LB_BASE, game );

  static char buf[1024];
  int n = http_get( url, buf, sizeof(buf) );
  if( n <= 0 ) return false;
  if( !strstr( buf, "\"ok\":true" ) ) return false;

  if( seedOut ){
    const char* s = strstr( buf, "\"seed\":" );
    *seedOut = s ? (unsigned)strtoul( s + 7, nullptr, 10 ) : 0;
  }

  const char* t = strstr( buf, "\"token\":\"" );
  if( !t ) return false;
  t += 9;                                // past "token":"
  int i = 0;
  while( *t && *t != '"' && i < tokenCap - 1 ) tokenOut[ i++ ] = *t++;
  tokenOut[ i ] = 0;
  return i > 0;
}

int leaderboard::submit( const char* game, const char* name, int score,
                         const char* token, Entry* out, int max, int* rankOut ){
  // name (A-Z only) and the base64url token are URL-safe, so no escaping needed.
  char url[320];
  snprintf( url, sizeof(url), "%s/submit?game=%s&name=%s&score=%d&token=%s",
            LB_BASE, game, name, score, token );

  static char buf[2048];
  int n = http_get( url, buf, sizeof(buf) );
  if( n <= 0 ) return -1;
  if( !strstr( buf, "\"ok\":true" ) ){
    if( rankOut ) *rankOut = -1;
    return -1;
  }

  if( rankOut ){
    const char* r = strstr( buf, "\"rank\":" );
    *rankOut = r ? atoi( r + 7 ) : -1;
  }
  return parse_board( buf, out, max );
}
