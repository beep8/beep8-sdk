#include <stdio.h>
#include <unistd.h>
#include <beep8.h>
#include <time.h>
#include <string.h>
#include <nesctrl.h>
#include <submath.h>

s32 _PpuResoW = 128;
s32 _PpuResoH = 128;

#define PPU_CMD_BUFF_WORDS (8*1024)
u32 _ppu_cmd_buff[ PPU_CMD_BUFF_WORDS ];
b8PpuCmd  _ppu_cmd;

#define MAX_OTZ (16)
enum EnOtz {
  OTZ_CLEAR = MAX_OTZ - 1,
  OTZ_RECT  = MAX_OTZ - 2,
  OTZ_CTRL  = MAX_OTZ - 3,
  OTZ_OBJ   = MAX_OTZ - 4,
};
u32 _ot[ MAX_OTZ ];
u32 _ot_prev[ MAX_OTZ ];

CfgNesCtrl cfg_nes_ctrl;

int main(int argc,char* argv[]){
  (void)argc; (void)argv;
  printf( "helper_nesctrl @2022_0418a\n" );

  b8PpuGetResolution((u32*)&_PpuResoW ,(u32*)&_PpuResoH );
  printf( "PpuReso=%d,%d\n",_PpuResoW, _PpuResoH);

  cfg_nes_ctrl.debug_visual = true;
  CNesCtrl* nes_ctrl = new  CNesCtrl( cfg_nes_ctrl );

  nes_ctrl->Reset();

  s32 xx = _PpuResoW>>1;
  s32 yy = 100;
  u32 hold_a = 0;
  u32 release_a = 0;
  u32 hold_b = 0;
  u32 release_b = 0;

  for( int frm=0 ; ; ++frm ){
    b8PpuVsyncWait();

    nes_ctrl->Step();
    if( nes_ctrl->GetCntHold( ENesCtrl::LEFT ) > 0 ){
      xx -= 1;
    } else if( nes_ctrl->GetCntHold( ENesCtrl::RIGHT ) > 0 ){
      xx += 1;
    }

    if( nes_ctrl->GetCntHold( ENesCtrl::UP ) > 0 ){
      yy -= 1;
    } else if( nes_ctrl->GetCntHold( ENesCtrl::DOWN ) > 0 ){
      yy += 1;
    }

    hold_a    = nes_ctrl->GetCntHold(    ENesCtrl::BTN_A );
    release_a = nes_ctrl->GetCntRelease( ENesCtrl::BTN_A );
    hold_b    = nes_ctrl->GetCntHold(    ENesCtrl::BTN_B );
    release_b = nes_ctrl->GetCntRelease( ENesCtrl::BTN_B );

    xx = lim(xx,4,_PpuResoW-4);
    yy = lim(yy,4,128-4);

    b8PpuCmdSetBuff( &_ppu_cmd , _ppu_cmd_buff , sizeof( _ppu_cmd_buff ) );
    b8PpuClearOT( &_ppu_cmd , &_ot[0] , &_ot_prev[0], MAX_OTZ );

    {
      b8PpuRect* pp = b8PpuRectAllocZ( &_ppu_cmd , OTZ_OBJ );
      pp->w = 8;
      pp->h = 8;
      pp->x = xx - (pp->w>>1);
      pp->y = yy - (pp->h>>1);
      pp->pal = B8_PPU_COLOR_WHITE;
    }

    if( nes_ctrl->GetCntHold( ENesCtrl::BTN_B ) > 0 ){
      b8PpuRect* pp = b8PpuRectAllocZ( &_ppu_cmd , OTZ_OBJ );
      pp->w = 4;
      pp->h = 4;
      pp->x = xx - (pp->w>>1) - 10;
      pp->y = yy - (pp->h>>1);
      pp->pal = B8_PPU_COLOR_GREEN;
    }

    if( nes_ctrl->GetCntHold( ENesCtrl::BTN_A ) > 0 ){
      b8PpuRect* pp = b8PpuRectAllocZ( &_ppu_cmd , OTZ_OBJ );
      pp->w = 4;
      pp->h = 4;
      pp->x = xx - (pp->w>>1) + 10;
      pp->y = yy - (pp->h>>1);
      pp->pal = B8_PPU_COLOR_GREEN;
    }


    const u32 Y_A = 120;

    {
      b8PpuRect* pp = b8PpuRectAllocZ( &_ppu_cmd , OTZ_RECT );
      pp->x = (_PpuResoW>>1) - release_a;
      pp->y = Y_A;
      pp->w = release_a;
      pp->h = 1;
      pp->pal = B8_PPU_COLOR_RED;
    }
    {
      b8PpuRect* pp = b8PpuRectAllocZ( &_ppu_cmd , OTZ_RECT );
      pp->x = (_PpuResoW>>1);
      pp->y = Y_A;
      pp->w = hold_a;
      pp->h = 1;
      pp->pal = B8_PPU_COLOR_RED;
    }

    const u32 Y_B = Y_A+3;
    {
      b8PpuRect* pp = b8PpuRectAllocZ( &_ppu_cmd , OTZ_RECT );
      pp->x = (_PpuResoW>>1) - release_b;
      pp->y = Y_B;
      pp->w = release_b;
      pp->h = 1;
      pp->pal = B8_PPU_COLOR_GREEN;
    }
    {
      b8PpuRect* pp = b8PpuRectAllocZ( &_ppu_cmd , OTZ_RECT );
      pp->x = (_PpuResoW>>1);
      pp->y = Y_B;
      pp->w = hold_b;
      pp->h = 1;
      pp->pal = B8_PPU_COLOR_GREEN;
    }

    {
      b8PpuRect* pp = b8PpuRectAllocZ( &_ppu_cmd , OTZ_CLEAR );
      pp->pal = B8_PPU_COLOR_BLUE;
      pp->x = 0;
      pp->y = 0;
      pp->w = 128;
      pp->h = 128;
    }

    nes_ctrl->Draw( &_ppu_cmd, OTZ_CTRL );
    b8PpuExec( &_ppu_cmd );
  }
  return 0;
}