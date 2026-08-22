#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "bgprint.h"
#include <b8/assert.h>
#include <b8/misc.h>
#include <submath.h>
#include <crt/crt.h>
#include <stdlib.h>
#include <string>
#include <fontdata.h>
#include <sublibc.h>
#include <esc_decoder.h>
#include <sys/errno.h>

using namespace std;
struct  DriverPriv{
  int _idx_slot = 0;
  CEscapeSeqDecoder _esc_decoder;
  bgprint::Context _ctx;
  s16 _x_locate = 0;
  s16 _y_locate = 0;
  u16 _u_bg = 0;
  u16 _v_bg = 0;
  bool _opened = false;
  EscapePAL _EscapePAL = PAL_0;
};
static  DriverPriv _dpriv[ bgprint::CHMAX ];

int bgprint_open( File* filep ){
  DriverPriv* dp = (DriverPriv*)filep->d_priv;

  // already opened ?
  if( dp->_opened ) return -1;

  dp->_opened = true;
  return 0;
}

int bgprint_close( File* filep ){
  DriverPriv* dp = (DriverPriv*)filep->d_priv;

  // not opened ?
  if( ! dp->_opened ) return -1;

  dp->_opened = false;
  return 0;
}

static void bgprint_clear_line( DriverPriv* dp , u16 yt ){
  const s16 wt = 1<< dp->_ctx._w_pow2;
  const s16 ht = 1<< dp->_ctx._h_pow2;
  const u16 ymask = ht-1;
  b8PpuBgTile tc = fontdata::gettc();
  b8PpuBgTile* cpuaddr = &dp->_ctx.cpuaddr[ wt * (yt & ymask) ];
  for( u16 xt=0 ; xt<wt ; ++xt ){
    *cpuaddr++ = tc;
  }
}

static void bgprint_clear_all( DriverPriv* dp ){
  const size_t words = (1<< dp->_ctx._w_pow2) * (1<<dp->_ctx._h_pow2);
  b8PpuBgTile tc = fontdata::gettc();
  b8PpuBgTile* cpuaddr = dp->_ctx.cpuaddr;
  for( u16 ii=0 ; ii<words ; ++ii ){
    *cpuaddr++ = tc;
  }
}

static ssize_t bgprint_write(File* filep,const char *buffer, size_t len) {
  DriverPriv* dp = (DriverPriv*)filep->d_priv;
  if( dp->_idx_slot < 0 ){
    set_errno( EINVAL );
    return -1;
  }
  if( dp->_idx_slot >= bgprint::CHMAX ){
    set_errno( EINVAL );
    return -1;
  }
  if( dp->_ctx.cpuaddr == nullptr ){
    set_errno( EINVAL );
    return -1;
  }

  size_t nn=0;
  const s16 wt = 1<< dp->_ctx._w_pow2;
  const s16 ht = 1<< dp->_ctx._h_pow2;
  const u16 xmask = wt-1;
  const u16 ymask = ht-1;

  // Loop-invariant, and these are out-of-line calls -- hoisting them saves two
  // calls per character printed.
  const u8 fnt_xtile = fontdata::dstxtile();
  const u8 fnt_ytile = fontdata::dstytile();

  for( ; nn<len ; ++nn ){
    const char character = buffer[ nn ];
    EscapeOut eout = dp->_esc_decoder.Stream( (s32)character );
    switch( eout._Ope) {
      case  ESO_NONE: break;
      case  ESO_ONE_CHAR:{
        if( 0xa == eout._code ){
          dp->_x_locate = 0;
          dp->_y_locate++;
          if( dp->_ctx._scroll && dp->_y_locate > dp->_ctx._h_disp ){ 
            dp->_v_bg += 8;
          }

          bgprint_clear_line( dp , dp->_y_locate );
          break;
        }

        const u16 ascii = (u16)eout._code - 0x20;
        if( ascii >= 0x60 ){
          set_errno( EIO );
          return -1;
        }

        const s16 xt = dp->_x_locate & xmask;
        const s16 yt = dp->_y_locate & ymask;

        if( dp->_x_locate < wt ){
          b8PpuBgTile* tile = &dp->_ctx.cpuaddr[ wt * yt + xt ];
          tile->PAL = dp->_EscapePAL;
          tile->XTILE = fnt_xtile + (ascii&15);
          tile->YTILE = fnt_ytile + (ascii>>4);
        }

        dp->_x_locate++;
      }break;
      case  ESO_MOVE_CURSOR:{
        dp->_x_locate = eout._x;
        dp->_y_locate = eout._y;
      }break;
      case  ESO_CLEAR_ENTIRE_SCREEN:{
        bgprint_clear_all( dp );
      }break;
      case  ESO_CLEAR_ENTIRE_LINE:{
        bgprint_clear_line( dp , dp->_y_locate );
      }break;
      case  ESO_SEL_PAL:{
        dp->_EscapePAL =  (EscapePAL)eout._EscapePAL;
      }break;
      default:
        _ASSERT(false,"[NOTIMPL]");
        break;
    }
  }
  return len;
}

int bgprint_ioctl( File* filep, unsigned int cmd, void* arg) {
  DriverPriv* dp = (DriverPriv*)filep->d_priv;
  switch( cmd ){
    case bgprint::SET_SLOT_CONTEXT:{
      bgprint::Context* pctx = (bgprint::Context*)arg;
      dp->_ctx = *pctx;
    }break;
    case bgprint::SET_UV_SCROLL:{
      bgprint::UvScroll* uv = (bgprint::UvScroll*)arg;
      dp->_u_bg = uv->_u;
      dp->_v_bg = uv->_v;
    }break;
    case bgprint::EXPORT_PPU_CMD:{
      bgprint::ExportPpuCmd* pepc = (bgprint::ExportPpuCmd*)arg;
      b8PpuBg* pp = b8PpuBgAllocZ( pepc->_cmd , pepc->_otz );
      pp->upix = dp->_u_bg;
      pp->vpix = dp->_v_bg;
      pp->wtile = dp->_ctx._w_pow2;
      pp->htile = dp->_ctx._h_pow2;
      pp->cpuaddr = dp->_ctx.cpuaddr;
      pp->vwrap = B8_PPU_BG_WRAP_CLAMP;
      pp->vwrap = B8_PPU_BG_WRAP_REPEAT;
    }break;
    case bgprint::GET_INFO:{
      bgprint::Info* info = static_cast< bgprint::Info* >( arg );
      if (!info) {
        set_errno(EINVAL);
        return -1;
      }
      info->_x_locate = dp->_x_locate;
      info->_y_locate = dp->_y_locate;
      info->_pal = static_cast< u8 >( dp->_EscapePAL );
    }break;
    case bgprint::XCHG_CURSOR:{
      // Same state change an "\e[y;xH\e[Nq" sequence would produce, without
      // going through the ANSI encoder/decoder for it.
      bgprint::XchgCursorArg* xc = static_cast< bgprint::XchgCursorArg* >( arg );
      if (!xc) {
        set_errno(EINVAL);
        return -1;
      }
      xc->_prev._x_locate = dp->_x_locate;
      xc->_prev._y_locate = dp->_y_locate;
      xc->_prev._pal      = static_cast< u8 >( dp->_EscapePAL );
      dp->_x_locate = xc->_x_locate;
      dp->_y_locate = xc->_y_locate;
      if( xc->_pal < 4 ) dp->_EscapePAL = (EscapePAL)xc->_pal;
    }break;
  }
  return 0;
}

static const file_operations bgprint_fops =
{
  bgprint_open,   /* open  */
  bgprint_close,  /* close */
  NULL,           /* read  */
  bgprint_write,  /* write */
  NULL,           /* seek  */
  bgprint_ioctl   /* ioctl */
};
namespace bgprint {
void  Reset(){
  static bool _is_reset = false;
  if( false == _is_reset ){
    fontdata::load();

    char name[32];
    for( int slot=0 ; slot<bgprint::CHMAX ; ++slot ){
      _dpriv[ slot ]._idx_slot = slot;
      memsetz(name,sizeof(name));
      snprintf(name, sizeof(name), "/bgprint/con%d", slot);
      fs_register_driver(
        name,
        &bgprint_fops,
        0666,
        &_dpriv[ slot ]
      );
    }
    _is_reset = true;
  }
}

FILE* Open(
  EnCh ch_,
  char* buff,
  size_t buffsize,
  bgprint::Context& ctx
){
  if( ch_ >= CHMAX )  return NULL;
  if( buffsize < 1)   return NULL;

  static const char tbl[] = {'0','1','2','3','4','5','6','7'};
  static_assert( sizeof(tbl)/sizeof(tbl[0]) == CHMAX , "numof(tbl) must be equal to CHMAX" );

  FILE* fp_bgprint = fopen(
    ( string( "/bgprint/con" ) + tbl[ ch_ ] ).c_str(),
    "wt"
  );
  if( fp_bgprint == NULL )  return NULL;

  static char* _buff[ bgprint::CHMAX ] = {nullptr}; 

  if( buff == nullptr ) buff = (char*)malloc( buffsize );
  _buff[ ch_ ] = buff;
  setvbuf( fp_bgprint, _buff[ ch_ ] , _IOFBF, buffsize );

  const size_t words = (1<<ctx._w_pow2) * (1<<ctx._h_pow2);
  if( nullptr == ctx.cpuaddr ){
    ctx.cpuaddr = new b8PpuBgTile[ words ];
  }
  for( size_t ii=0 ; ii<words ; ++ii ){
    ctx.cpuaddr[ ii ] = fontdata::gettc();
  }

  const int fd = fileno( fp_bgprint );
  ioctl(fd, bgprint::SET_SLOT_CONTEXT , &ctx );

  return fp_bgprint;
}

void  SetUvScroll(
  FILE* fp_,
  const bgprint::UvScroll& uv_
){
  const int fd = fileno( fp_ );
  ioctl(fd, bgprint::SET_UV_SCROLL , &uv_ );
}

void  Export(
  FILE* fp_,
  const bgprint::ExportPpuCmd& epc
){
  fflush( fp_ );
  const int fd = fileno( fp_ );
  ioctl(fd, bgprint::EXPORT_PPU_CMD , &epc );
}

// The escape sequences below are built by hand instead of with fprintf().
// newlib's formatted output costs ~1900 CPU cycles per call on the 4 MHz core,
// so a screenful of pico8::cursor() calls (one Locate each) alone burns a whole
// frame budget. Emitting the digits directly is roughly ten times cheaper, and
// these sequences only ever need a plain signed decimal.
//
// The finished sequence goes out through putc()-style inline stores rather than fwrite(): this
// stream is always fully buffered (see Open()), so that is a couple of
// instructions per character, where fwrite() pays newlib's whole vector-write
// setup -- ~500 cycles -- for a dozen bytes.
static void put_buf( FILE* fp_, const char* src, size_t len ){
  for( size_t ii=0 ; ii<len ; ++ii ) __sputc_r( _REENT, src[ ii ], fp_ );
}

static char* put_dec( char* dst, int val ){
  if( val < 0 ){ *dst++ = '-'; val = -val; }
  char rev[ 12 ];
  int nn = 0;
  do { rev[ nn++ ] = (char)( '0' + ( val % 10 ) ); val /= 10; } while( val );
  while( nn ) *dst++ = rev[ --nn ];
  return dst;
}

void  Locate(FILE* fp_ ,s16 lx_,s16 ly_ ){
  char buf[ 24 ];
  char* pp = buf;
  *pp++ = '\e'; *pp++ = '[';
  pp = put_dec( pp, ly_ );
  *pp++ = ';';
  pp = put_dec( pp, lx_ );
  *pp++ = 'H';
  put_buf( fp_, buf, (size_t)( pp - buf ) );
}

void Pal(FILE* fp_, u8 pal_ ){
  if( pal_ >= 4 ) return;
  static const char tbl[4][4] = { {'\e','[','0','q'}, {'\e','[','1','q'}, {'\e','[','2','q'}, {'\e','[','3','q'} };
  put_buf( fp_, tbl[ pal_ ], 4 );
}

int GetInfo(FILE* fp_ ,Info& dest ){
  if( ! fp_ ) return -1;
  int fd = fileno( fp_ );
  if (fd == -1) return -2;

  fflush( fp_ );
  int ret = ioctl(fd, bgprint::GET_INFO, &dest );
  if (ret == -1)  return -3;

  return 0;
}

int XchgCursor(FILE* fp_, s16 lx_, s16 ly_, u8 pal_, Info& prev_ ){
  if( ! fp_ ) return -1;
  int fd = fileno( fp_ );
  if (fd == -1) return -2;

  XchgCursorArg xc;
  xc._x_locate = lx_;
  xc._y_locate = ly_;
  xc._pal      = pal_;

  // Everything already printed has to reach the tilemap before the cursor
  // moves out from under it.
  fflush( fp_ );
  int ret = ioctl(fd, bgprint::XCHG_CURSOR, &xc );
  if (ret == -1)  return -3;

  prev_ = xc._prev;
  return 0;
}

} // namespace bgprint