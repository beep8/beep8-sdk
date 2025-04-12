#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "sprprint.h"
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

#define PALSEL  (15)

using namespace std;

struct  DriverPriv{
  int _idx_slot = 0;
  CEscapeSeqDecoder _esc_decoder;
  sprprint::Context _ctx;
  s16 _xpix_locate = 0;
  s16 _ypix_locate = 0;
  u8  _wpix = 8;
  u8  _hpix = 8;
  bool _opened = false;
  bool _shadow = true;
  u8  _attr : 4;  // Holds values 0-15 (commonly used range is 0-9 for terminal attributes)
  b8PpuColor  _fg = B8_WHITE;
  b8PpuColor  _bg = B8_TRANSPARENT;
  u16 _otz = 0;
  s16 xreso;
  s16 yreso;
};
static  DriverPriv _dpriv[ sprprint::CHMAX ];

// ANSI to BEEP-8 PPU Color conversion table
static const b8PpuColor ansiToB8PpuColorTable[] = {
    B8_DARK_BLUE,   // ANSI_COLOR_BLACK -> B8_DARK_BLUE
                    // Reason: ANSI_COLOR_BLACK (pure black) is treated as a transparent color in BEEP-8.
                    //         To avoid transparency, we map it to B8_DARK_BLUE, which is the closest 
                    //         available color in the BEEP-8 palette.
    B8_RED,         // ANSI_COLOR_RED -> B8_RED
    B8_GREEN,       // ANSI_COLOR_GREEN -> B8_GREEN
    B8_YELLOW,      // ANSI_COLOR_YELLOW -> B8_YELLOW
    B8_BLUE,        // ANSI_COLOR_BLUE -> B8_BLUE
    B8_LAVENDER,    // ANSI_COLOR_MAGENTA -> B8_LAVENDER
    B8_LIGHT_PEACH, // ANSI_COLOR_CYAN -> B8_LIGHT_PEACH
    B8_WHITE,       // ANSI_COLOR_WHITE -> B8_WHITE

    // Bright colors
    B8_DARK_GREY,   // ANSI_COLOR_BRIGHT_BLACK -> B8_DARK_GREY
    B8_PINK,        // ANSI_COLOR_BRIGHT_RED -> B8_PINK
    B8_GREEN,       // ANSI_COLOR_BRIGHT_GREEN -> B8_GREEN
    B8_ORANGE,      // ANSI_COLOR_BRIGHT_YELLOW -> B8_ORANGE
    B8_BLUE,        // ANSI_COLOR_BRIGHT_BLUE -> B8_BLUE
    B8_LAVENDER,    // ANSI_COLOR_BRIGHT_MAGENTA -> B8_LAVENDER
    B8_LIGHT_GREY,  // ANSI_COLOR_BRIGHT_CYAN -> B8_LIGHT_GREY
    B8_WHITE        // ANSI_COLOR_BRIGHT_WHITE -> B8_WHITE
};

// Function to convert AnsiColor to b8PpuColor
static  b8PpuColor ansiToB8PpuColor(AnsiColor ansiColor) {
  if( ansiColor == ANSI_NULL )  return  B8_TRANSPARENT;

    // Map ANSI color range (30-37, 90-97) to the table indices (0-15)
    if (ansiColor >= ANSI_COLOR_BLACK && ansiColor <= ANSI_COLOR_WHITE) {
        return ansiToB8PpuColorTable[ansiColor - ANSI_COLOR_BLACK];

    // Handle BEEP-8 specific colors (50-65)
    } else if (ansiColor >= ANSI_COLOR_B8_BLACK && ansiColor <= ANSI_COLOR_B8_LIGHT_PEACH) {
        return static_cast< b8PpuColor >( (int)ansiColor - (int)ANSI_COLOR_B8_BLACK );

    } else if (ansiColor >= ANSI_COLOR_BRIGHT_BLACK && ansiColor <= ANSI_COLOR_BRIGHT_WHITE) {
        return ansiToB8PpuColorTable[ansiColor - ANSI_COLOR_BRIGHT_BLACK + 8]; // Offset by 8 for bright colors

    } else {
        return B8_WHITE;  // Default to white if out of range
    }
}

int sprprint_open( File* filep ){
  DriverPriv* dp = (DriverPriv*)filep->d_priv;

  // already opened ?
  if( dp->_opened ) return -1;

  dp->_opened = true;
  u32 xr,yr;
  b8PpuGetResolution(&xr,&yr);
  dp->xreso = static_cast< s16 >( xr );
  dp->yreso = static_cast< s16 >( yr );

  return 0;
}

int sprprint_close( File* filep ){
  DriverPriv* dp = (DriverPriv*)filep->d_priv;

  // not opened ?
  if( ! dp->_opened ) return -1;

  dp->_opened = false;
  return 0;
}

static ssize_t sprprint_write(File* filep,const char *buffer, size_t len) {
  // Check if buffer is NULL or length is zero
  if (buffer == NULL || len == 0) {
    set_errno(EINVAL);
    return -1;
  }

  DriverPriv* dp = (DriverPriv*)filep->d_priv;
  // Check if slot is valid
  if( dp->_idx_slot < 0 || dp->_idx_slot >= sprprint::CHMAX ){
    set_errno( EINVAL );
    return -1;
  }

  size_t nn=0;
  for( ; nn<len && buffer[nn] != '\0' ; ++nn ){
    const EscapeOut& eout = dp->_esc_decoder.Stream( (s32)buffer[ nn ] );
    switch( eout._Ope) {
      case  ESO_NONE: break;
      case  ESO_ONE_CHAR:{
        if( 0xa == eout._code ){
          dp->_xpix_locate = 0;
          dp->_ypix_locate += 8;
          break;
        }

        const u16 ascii = (u16)eout._code - 0x20;
        if( ascii >= 0x60 ) return len;

        if( 
          dp->_xpix_locate > -8     &&
          dp->_xpix_locate < dp->xreso  && 
          dp->_ypix_locate > -8     &&
          dp->_ypix_locate < dp->yreso
        ){
          if( dp->_bg != B8_TRANSPARENT ){
            b8PpuRect* pr = b8PpuRectAllocZPB( 
              dp->_ctx._cmd,
              dp->_otz
            );
            pr->pal = dp->_bg;
            pr->x = dp->_xpix_locate;
            pr->y = dp->_ypix_locate;
            pr->w = pr->h = 8;
          }

          b8PpuSprite* pp = b8PpuSpriteAllocZPB( 
            dp->_ctx._cmd,
            dp->_otz
          );
          pp->pal = PALSEL;
          pp->x = dp->_xpix_locate;
          pp->y = dp->_ypix_locate;
          pp->srcwtile = pp->srchtile = 1;
          pp->vfp = pp->hfp = 0;
          pp->srcxtile = fontdata::dstxtile() + (ascii&15);
          pp->srcytile = fontdata::dstytile() + (ascii>>4);
        }
        dp->_xpix_locate += 8;
      }break;
      case  ESO_MOVE_CURSOR:{
        dp->_xpix_locate = eout._x;
        dp->_ypix_locate = eout._y;
      }break;
      case  ESO_SET_Z:{
        dp->_otz = eout._otz;
      }break;
      case  ESO_SET_COLOR:{
        dp->_attr = eout._attr;
        dp->_fg = ansiToB8PpuColor( eout._fg );
        if( dp->_fg == B8_TRANSPARENT ) dp->_fg = B8_WHITE;
        dp->_bg = ansiToB8PpuColor(static_cast<AnsiColor>(eout._bg));

        b8PpuSetpal* pal = b8PpuSetpalAllocZPB(dp->_ctx._cmd, dp->_otz, 1);
        pal->palsel = PALSEL;
        pal->pidx7 = dp->_fg;
        pal->pidx1 = dp->_shadow ? 1:0;
      }break;
      case  ESO_UP:   dp->_ypix_locate -= dp->_hpix; break;
      case  ESO_DOWN: dp->_ypix_locate += dp->_hpix; break;
      case  ESO_RIGHT:dp->_xpix_locate += dp->_wpix; break;
      case  ESO_LEFT: dp->_xpix_locate -= dp->_wpix; break;

      case  ESO_ENABLE_SHADOW:{
        dp->_shadow = true;
      }break;

      case  ESO_DISABLE_SHADOW:{
        dp->_shadow = false;
      }break;

      // These cases intentionally do nothing
      case  ESO_CLEAR_SCREEN_FROM_CURSOR_DOWN:
      case  ESO_CLEAR_SCREEN_FROM_CURSOR_UP:
      case  ESO_CLEAR_ENTIRE_SCREEN:
      case  ESO_CLEAR_LINE_FROM_CURSOR_RIGHT:
      case  ESO_CLEAR_LINE_FROM_CURSOR_LEFT:
      case  ESO_CLEAR_ENTIRE_LINE:
      case  ESO_SEL_PAL:
      case  ESO_DEL:
        break;
    }
  }

  return nn;
}

int sprprint_ioctl( File* filep, unsigned int cmd, void* arg) {
  if (!filep || !arg) {
    set_errno(EINVAL);
    return -1;
  }

  DriverPriv* dp = (DriverPriv*)filep->d_priv;
  switch( cmd ){
    case sprprint::SET_SLOT_CONTEXT:{
      sprprint::Context* pctx = (sprprint::Context*)arg;
      if (!pctx) {
        set_errno(EINVAL);
        return -1;
      }
      dp->_ctx = *pctx;
    }break;
    case sprprint::GET_INFO:{
      sprprint::Info* info = static_cast< sprprint::Info* >( arg );
      if (!info) {
        set_errno(EINVAL);
        return -1;
      }
      info->_xpix_locate = dp->_xpix_locate;
      info->_ypix_locate = dp->_ypix_locate;
      info->_fg = dp->_fg;
      info->_bg = dp->_bg;
      info->_otz = dp->_otz;
    }break;
    default:{
      set_errno(ENOTTY);  // 無効なIOCTLコマンド
      return -1;
    }
  }
  return  0;
}

static const file_operations sprprint_fops =
{
  sprprint_open,   /* open  */
  sprprint_close,  /* close */
  NULL,           /* read  */
  sprprint_write,  /* write */
  NULL,           /* seek  */
  sprprint_ioctl   /* ioctl */
};

namespace sprprint {

void  Reset(){
  static bool _is_reset = false;
  if( false == _is_reset ){
    fontdata::load();

    char name[32];
    for( int slot=0 ; slot<sprprint::CHMAX ; ++slot ){
      _dpriv[ slot ]._idx_slot = slot;
      memsetz(name,sizeof(name));
      snprintf(name, sizeof(name), "/sprprint/con%d", slot);
      fs_register_driver(
        name,
        &sprprint_fops,
        0666,
        &_dpriv[ slot ]
      );
    }
    _is_reset = true;
  }
}

FILE* Open( EnCh ch_, sprprint::Context& ctx ){
  if( ch_ >= CHMAX )  return NULL;

  static const char tbl[] = {'0','1' };
  static_assert( sizeof(tbl)/sizeof(tbl[0]) == CHMAX , "numof(tbl) must be equal to CHMAX" );

  FILE* fp = fopen(
    ( string( "/sprprint/con" ) + tbl[ ch_ ] ).c_str(),
    "wt"
  );
  if( fp == NULL )  return NULL;

  setvbuf(fp, NULL, _IONBF, 0);
  fflush(fp);

  const int fd = fileno( fp );
  ioctl(fd, sprprint::SET_SLOT_CONTEXT , &ctx );
  return fp;
}

void  Locate(FILE* fp_ ,s16 lx_,s16 ly_,u16 otz_ ){
  if( ! fp_ ) return;
  fprintf( fp_, "\e[%d;%dH\e[%dz",ly_,lx_,otz_);
}

void  LocateZ(FILE* fp_ ,u16 otz_ ){
  if( ! fp_ ) return;
  fprintf( fp_, "\e[%dz" , otz_ );
}

void Color(FILE* fp_, b8PpuColor b8col_ ){
  if( ! fp_ ) return;
  fprintf(fp_, "\e[%dm",50+b8col_);
}

int GetInfo(FILE* fp_ ,Info& dest ){
  if( ! fp_ ) return -1;
  int fd = fileno( fp_ );
  if (fd == -1) return -2;

  fflush( fp_ );
  int ret = ioctl(fd, GET_INFO, &dest );
  if (ret == -1)  return -3;

  return 0;
}

} // namespace sprprint