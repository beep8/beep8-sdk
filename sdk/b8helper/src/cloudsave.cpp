// Minimal cloudsave client over the /http file driver. See cloudsave.h.
#include <cloudsave.h>
#include <httpdrv.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Production cloudsave server. The dbg server is :9085.
#define CS_BASE "https://beep8.org:8085"

// Stay well under the 8KB SCI FIFO that carries both the request URL and the
// response body, in both directions (see js.b8/Sci.js: FIFO length = 8*1024).
#define CHUNK (4000)

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

// The integer following `"field":` in a JSON response (e.g. field="\"total\":").
static long json_int( const char* json, const char* field, long defval ){
  const char* p = strstr( json, field );
  return p ? strtol( p + strlen(field), nullptr, 10 ) : defval;
}

// Copy the string value following `field` (e.g. field="\"data\":\"") into dst
// up to dstcap-1 bytes. No escaping is expected (values are base64url/ASCII).
// Returns the copied length, or -1 if the field is absent.
static int json_str( const char* json, const char* field, char* dst, int dstcap ){
  const char* p = strstr( json, field );
  if( !p ) return -1;
  p += strlen( field );
  int i = 0;
  while( *p && *p != '"' && i < dstcap - 1 ) dst[i++] = *p++;
  dst[i] = 0;
  return i;
}

static bool valid_key( const char* key ){
  int n = 0;
  for( ; key[n] ; ++n ){
    char c = key[n];
    bool alnum = ( c>='0'&&c<='9' ) || ( c>='a'&&c<='z' ) || ( c>='A'&&c<='Z' );
    if( !alnum ) return false;
  }
  return n == 16;
}

int cloudsave::Get( const char* key, char* buf, int bufcap ){
  if( !valid_key(key) || bufcap <= 0 ) return -1;

  static char url[160];
  static char resp[ CHUNK + 256 ];
  static char chunk[ CHUNK + 1 ];

  int off = 0;                         // server-side read cursor
  int got = 0;                         // bytes written into buf
  for(;;){
    snprintf( url, sizeof(url), "%s/get?key=%s&off=%d&len=%d", CS_BASE, key, off, CHUNK );
    int n = http_get( url, resp, sizeof(resp) );
    if( n <= 0 || !strstr( resp, "\"ok\":true" ) ) return got > 0 ? got : -1;

    long total = json_int( resp, "\"total\":", 0 );
    int  clen  = json_str( resp, "\"data\":\"", chunk, sizeof(chunk) );
    if( clen < 0 ) clen = 0;

    int room = bufcap - 1 - got;
    int copy = clen < room ? clen : room;
    memcpy( buf + got, chunk, copy );
    got += copy;
    off += clen;

    if( off >= (int)total || clen < CHUNK || got >= bufcap - 1 ) break;
  }
  buf[ got ] = 0;
  return got;
}

bool cloudsave::Set( const char* key, const char* value, int len ){
  if( !valid_key(key) || len < 0 ) return false;

  static char url[ CHUNK + 160 ];
  static char resp[256];

  int off = 0;
  do {
    int n = len - off;
    if( n > CHUNK ) n = CHUNK;

    // value is caller-guaranteed URL-safe (base64url, or plain alnum/JSON
    // with no reserved characters) -- see cloudsave.h. No percent-encoding.
    int wlen = snprintf( url, sizeof(url), "%s/set?key=%s&off=%d&total=%d&data=",
                          CS_BASE, key, off, len );
    memcpy( url + wlen, value + off, n );
    url[ wlen + n ] = 0;

    int rn = http_get( url, resp, sizeof(resp) );
    if( rn <= 0 || !strstr( resp, "\"ok\":true" ) ) return false;

    off += n;
  } while( off < len );

  return true;
}
