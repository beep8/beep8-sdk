#include <stdio.h>
#include <nesctrl.h>
#include <vector>
#include <string.h>
#include <sublibc.h>
#include <hif_decoder.h>
#include <nesctrl.h>

using namespace std;
struct TouchPos{
  s16 _tx = 0;
  s16 _ty = 0;
  inline  void  Set( b8HifEvent* ev ){
    _tx = ev->xp;
    _ty = ev->yp;
  }
};

#define N_BTN  (6)
static_assert( (int)ENesCtrl::MAX == (int)N_BTN , "need to adjust N_BTN" );
class ImplCNesCtrl{
  friend  class CNesCtrl;
  u32   _holding    [ N_BTN ] = {0};
  u32   _cnt_hold   [ N_BTN ] = {0};
  u32   _cnt_release[ N_BTN ] = {0};
  CHifDecoder decoder;
};

extern "C" const uint8_t* b8_image_helper_nes_ctrl_get( uint16_t* width, uint16_t* height );

void  CNesCtrl::Reset(){
  static  u32       _ppu_cmd_buff[ 32 ];
  static  b8PpuCmd  _ppu_cmd;

  if( impl )  delete impl;
  impl = new ImplCNesCtrl;

  b8PpuCmdSetBuff( &_ppu_cmd , _ppu_cmd_buff , sizeof( _ppu_cmd_buff ) );
  {
    uint16_t ww,hh;
    b8PpuLoadimg* pp = b8PpuLoadimgAlloc( &_ppu_cmd );
    pp->cpuaddr = b8_image_helper_nes_ctrl_get(&ww,&hh);

    // src
    pp->srcxtile = 0;
    pp->srcytile = 0;
    pp->srcwtile = ww >>3;

    // dst
    pp->dstxtile = _cfg.img_utile;
    pp->dstytile = _cfg.img_vtile;
    pp->trnwtile = ww >>3;
    pp->trnhtile = hh >>3;
  }

  {
    b8PpuFlush* pp = b8PpuFlushAlloc( &_ppu_cmd );
    pp->img = 1;
  }
  b8PpuHaltAlloc( &_ppu_cmd );
  b8PpuExec( &_ppu_cmd );
}


struct Sensor {
  u8 xt;
  u8 yt;
  ENesCtrl ctrl_0;
  ENesCtrl ctrl_1;
  u8 wt = 2;
  u8 ht = 2;
};

static const Sensor sensor[] = {
  {0,0,ENesCtrl::UP,    ENesCtrl::LEFT  },
  {2,0,ENesCtrl::UP,    ENesCtrl::MAX   },
  {4,0,ENesCtrl::UP,    ENesCtrl::RIGHT },

  {0,2,ENesCtrl::LEFT,  ENesCtrl::MAX   },
  {4,2,ENesCtrl::RIGHT, ENesCtrl::MAX   },

  {0,4,ENesCtrl::DOWN,  ENesCtrl::LEFT  },
  {2,4,ENesCtrl::DOWN,  ENesCtrl::MAX   },
  {4,4,ENesCtrl::DOWN,  ENesCtrl::RIGHT },

  { 7,3,ENesCtrl::BTN_B, ENesCtrl::MAX ,3,3 },
  {11,3,ENesCtrl::BTN_A, ENesCtrl::MAX ,3,3 },
};

struct CtrlLayout {
  u8 xt;      // offset xtile
  u8 yt;      // offset ytile
  u8 ut;      // src u tile
  u8 vt;      // src v tile
  ENesCtrl ctrl = ENesCtrl::MAX;
  u8 wt = 2;  // width  of tile
  u8 ht = 2;  // height of tile
};

static const CtrlLayout layout[] = {
                          {2,0,2,1,ENesCtrl::UP},
{0,2,0,3,ENesCtrl::LEFT}, {2,2,2,3},               {4,2,4,3,ENesCtrl::RIGHT},
                          {2,4,2,5,ENesCtrl::DOWN},

                          /* B BUTTON */
                          { 7, 3,5,5,ENesCtrl::BTN_B,3,3},

                          /* A BUTTON */
                          {11, 3,5,0,ENesCtrl::BTN_A,3,3}
};

static  void  ChkCollide( const TouchPos& tp , const CfgNesCtrl& cfg, u8* results ){
  const s32 xmin_tp = tp._tx - (cfg.radius_pix<<4);
  const s32 xmax_tp = tp._tx + (cfg.radius_pix<<4);
  const s32 ymin_tp = tp._ty - (cfg.radius_pix<<4);
  const s32 ymax_tp = tp._ty + (cfg.radius_pix<<4);

  for( size_t nn=0 ; nn<sizeof(sensor)/sizeof(sensor[0]) ; ++nn ){
    const Sensor& ss = sensor[nn];
    const s32 xmin_ui = (cfg.left_pix + (ss.xt<<3))<<4;
    const s32 ymin_ui = (cfg.top_pix  + (ss.yt<<3))<<4;
    const s32 xmax_ui = xmin_ui + (ss.wt<<(3+4));
    const s32 ymax_ui = ymin_ui + (ss.ht<<(3+4));

    if( xmin_tp > xmax_ui ) continue;
    if( xmax_tp < xmin_ui ) continue;
    if( ymin_tp > ymax_ui ) continue;
    if( ymax_tp < ymin_ui ) continue;

    if( ss.ctrl_0 == ENesCtrl::MAX )  continue;
    results[ int( ss.ctrl_0 ) ] = 1;

    if( ss.ctrl_1 == ENesCtrl::MAX )  continue;
    results[ int( ss.ctrl_1 ) ] = 1;
  }
}

void  CNesCtrl::Step(){
  u8 results[ N_BTN ];
  memset(&results[0],0x00,sizeof(results));
  const auto& status = impl->decoder.GetStatus();
  for (const auto& [key, value] : status) {
    switch(value->ev.type){
      case  B8_HIF_EV_MOUSE_DOWN:
      case  B8_HIF_EV_MOUSE_MOVE:
      case  B8_HIF_EV_TOUCH_START:
      case  B8_HIF_EV_TOUCH_MOVE:{
        TouchPos tp;
        tp._tx = value->ev.xp;
        tp._ty = value->ev.yp;
        ChkCollide( tp , _cfg, results );
      }break;
      default:break;
    }
  }

  for( size_t nn=0 ; nn< numof( results ) ; ++nn ){
    if( results[nn] ){
      impl->_cnt_hold   [ nn ]++;
      impl->_cnt_release[ nn ] = 0;
    } else {
      impl->_cnt_hold   [ nn ] = 0;
      impl->_cnt_release[ nn ]++;
    }
  }
}

void  CNesCtrl::Draw( b8PpuCmd* pcmd , u32 otz ){
  (void)pcmd; (void)otz;

  u32 wpix,hpix;
  b8PpuGetResolution(&wpix ,&hpix );

  {
    b8PpuScissor* ps = b8PpuScissorAllocZ( pcmd , otz );
    ps->x = 0;
    ps->y = 0;
    ps->w = wpix;
    ps->h = hpix;
  }

  for( size_t nn=0 ; nn < numof(layout) ; ++nn ){
    b8PpuSprite* pp = b8PpuSpriteAllocZ( pcmd , otz );
    pp->pal = _cfg.img_pal;
    pp->vfp = pp->hfp = 0;
    pp->srcxtile = layout[nn].ut + _cfg.img_utile;
    pp->srcytile = layout[nn].vt + _cfg.img_vtile;

    if( layout[nn].ctrl != ENesCtrl::MAX ){
      if( impl->_cnt_hold[ int( layout[nn].ctrl ) ] > 0 ) pp->srcytile += 8;
    }

    pp->srcwtile = layout[nn].wt;
    pp->srchtile = layout[nn].ht;

    pp->x = _cfg.left_pix + (layout[nn].xt<<3);
    pp->y = _cfg.top_pix  + (layout[nn].yt<<3);
  }

  if( _cfg.debug_visual ){
    const auto& status = impl->decoder.GetStatus();
    for (const auto& [key, value] : status) {
      b8PpuRect* pp = b8PpuRectAllocZ( pcmd , 1 );
      pp->pal = value->hdl & 15;
      const int RR =16;
      pp->x = (value->ev.xp>>4)-RR;
      pp->y = (value->ev.yp>>4)-RR;
      pp->w = (RR*2);
      pp->h = (RR*2);
    }
  }

  {
    b8PpuRect* pp = b8PpuRectAllocZ( pcmd , otz );
    pp->pal = _cfg.clear_color;
    pp->x = 0;
    pp->y = 127;
    pp->w = wpix;
    pp->h = hpix-128;
    {
      b8PpuScissor* ps = b8PpuScissorAllocZ( pcmd , otz );
      ps->x = 0;
      ps->y = 128;
      ps->w = wpix;
      ps->h = hpix-128;
    }
  }
}

u32   CNesCtrl::GetCntHold( ENesCtrl enc ){
  if( !impl ) return 0;
  return impl->_cnt_hold[ static_cast< u32 >( enc ) ];
}

u32   CNesCtrl::GetCntRelease( ENesCtrl enc ){
  if( !impl ) return 0;
  return impl->_cnt_release[ static_cast< u32 >( enc ) ];
}

CNesCtrl::CNesCtrl( const CfgNesCtrl& cfg )
  : _cfg( cfg )
{
  impl = 0;
}

CNesCtrl::~CNesCtrl(){
  if( impl )  delete impl;
}
