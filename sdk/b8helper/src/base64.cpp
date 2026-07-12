// Base64url encode/decode. See base64.h for the usage contract.
#include <base64.h>
#include <b8/type.h>

static const char ENC_TABLE[] =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

// -1 for invalid chars; built lazily on first use.
static  s8    DEC_TABLE[256];
static  bool  s_dec_ready = false;

static  void  buildDecodeTable(){
  for( int i = 0 ; i < 256 ; ++i ) DEC_TABLE[i] = -1;
  for( int i = 0 ; i < 64  ; ++i ) DEC_TABLE[ (u8)ENC_TABLE[i] ] = (s8)i;
  s_dec_ready = true;
}

int base64::encodedLen( int len ){
  return ( len + 2 ) / 3 * 4 - ( (3 - len % 3) % 3 );
}

int base64::decodedLen( int len ){
  return len * 3 / 4;
}

int base64::encode( const void* src, int srclen, char* dst, int dstcap ){
  const u8* s = (const u8*)src;
  int       need = encodedLen( srclen );
  if( need + 1 > dstcap ) return -1;

  int di = 0;
  int i  = 0;
  for( ; i + 3 <= srclen ; i += 3 ){
    u32 v = ( (u32)s[i] << 16 ) | ( (u32)s[i+1] << 8 ) | (u32)s[i+2];
    dst[di++] = ENC_TABLE[ (v >> 18) & 0x3f ];
    dst[di++] = ENC_TABLE[ (v >> 12) & 0x3f ];
    dst[di++] = ENC_TABLE[ (v >> 6)  & 0x3f ];
    dst[di++] = ENC_TABLE[ v & 0x3f ];
  }
  int rem = srclen - i;
  if( rem == 1 ){
    u32 v = (u32)s[i] << 16;
    dst[di++] = ENC_TABLE[ (v >> 18) & 0x3f ];
    dst[di++] = ENC_TABLE[ (v >> 12) & 0x3f ];
  } else if( rem == 2 ){
    u32 v = ( (u32)s[i] << 16 ) | ( (u32)s[i+1] << 8 );
    dst[di++] = ENC_TABLE[ (v >> 18) & 0x3f ];
    dst[di++] = ENC_TABLE[ (v >> 12) & 0x3f ];
    dst[di++] = ENC_TABLE[ (v >> 6)  & 0x3f ];
  }
  dst[di] = 0;
  return di;
}

int base64::decode( const char* src, int srclen, void* dst, int dstcap ){
  if( !s_dec_ready ) buildDecodeTable();
  if( srclen % 4 == 1 ) return -1;           // never a valid base64 length

  u8* d    = (u8*)dst;
  int need = decodedLen( srclen );
  if( need > dstcap ) return -1;

  int di = 0;
  int i  = 0;
  for( ; i + 4 <= srclen ; i += 4 ){
    s8 a = DEC_TABLE[ (u8)src[i]   ];
    s8 b = DEC_TABLE[ (u8)src[i+1] ];
    s8 c = DEC_TABLE[ (u8)src[i+2] ];
    s8 e = DEC_TABLE[ (u8)src[i+3] ];
    if( a < 0 || b < 0 || c < 0 || e < 0 ) return -1;
    u32 v = ( (u32)a << 18 ) | ( (u32)b << 12 ) | ( (u32)c << 6 ) | (u32)e;
    d[di++] = (u8)( v >> 16 );
    d[di++] = (u8)( v >> 8 );
    d[di++] = (u8)v;
  }
  int rem = srclen - i;
  if( rem == 2 ){
    s8 a = DEC_TABLE[ (u8)src[i]   ];
    s8 b = DEC_TABLE[ (u8)src[i+1] ];
    if( a < 0 || b < 0 ) return -1;
    u32 v = ( (u32)a << 18 ) | ( (u32)b << 12 );
    d[di++] = (u8)( v >> 16 );
  } else if( rem == 3 ){
    s8 a = DEC_TABLE[ (u8)src[i]   ];
    s8 b = DEC_TABLE[ (u8)src[i+1] ];
    s8 c = DEC_TABLE[ (u8)src[i+2] ];
    if( a < 0 || b < 0 || c < 0 ) return -1;
    u32 v = ( (u32)a << 18 ) | ( (u32)b << 12 ) | ( (u32)c << 6 );
    d[di++] = (u8)( v >> 16 );
    d[di++] = (u8)( v >> 8 );
  }
  return di;
}
