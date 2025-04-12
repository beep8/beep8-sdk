#include <stdio.h>
#include <unistd.h>
#include <beep8.h>
#include <time.h>
#include <string.h>
#include <bgprint.h>
#include <fstream>
#include <b8/misc.h>
#include <hif_decoder.h>
#include <trace.h>

using namespace std;

static  s32 _PpuResoW = 128;
static  s32 _PpuResoH = 128;

#define PPU_CMD_BUFF_WORDS (8*1024)
static  u32 _ppu_cmd_buff[ PPU_CMD_BUFF_WORDS ];
b8PpuCmd  _ppu_cmd;

#define MAX_OTZ (16)
enum EnOtz {
  OTZ_CLEAR         = MAX_OTZ - 1,
  OTZ_MOUSE_CURSOR  = MAX_OTZ - 2,
  OTZ_RECT          = MAX_OTZ - 3,
  OTZ_BG_TEXT       = MAX_OTZ - 4,
};
u32 _ot[ MAX_OTZ ];
u32 _ot_prev[ MAX_OTZ ];
CHifDecoder decoder;

int main(int argc,char* argv[]){
  (void)argc; (void)argv;
  static const char* caption = "touch\n@2024_1112a\nTry touching\nthe screen with\nseveral fingers.\n";
  printf( caption );

  b8PpuGetResolution((u32*)&_PpuResoW ,(u32*)&_PpuResoH );

  bgprint::Reset();
  bgprint::Context ctx;
  //ctx._h_pow2 = 4;
  FILE* fp_bgprint = bgprint::Open(
    bgprint::CH3,
    nullptr, 256,
    ctx
  );
  fprintf(fp_bgprint, caption );

  for( int frm=0 ; ; ++frm ){
    b8PpuVsyncWait();
    b8PpuCmdSetBuff( &_ppu_cmd , _ppu_cmd_buff , sizeof( _ppu_cmd_buff ) );
    b8PpuClearOT( &_ppu_cmd , &_ot[0], &_ot_prev[0], MAX_OTZ );

    {
      b8PpuRect* pp = b8PpuRectAlloc( &_ppu_cmd );
      pp->pal = B8_PPU_COLOR_BLUE;
      pp->x = 0;
      pp->y = 0;
      pp->w = _PpuResoW;
      pp->h = _PpuResoH;
      b8PpuPushFrontOT( &_ppu_cmd  , OTZ_CLEAR, pp );
    }

    const auto& status = decoder.GetStatus();
    for (const auto& [key, value] : status) {
      b8PpuRect* pp = b8PpuRectAllocZ( &_ppu_cmd , OTZ_RECT );
      pp->pal = value->hdl & 15;
      const int RR =16;
      pp->x = (value->ev.xp>>4)-RR;
      pp->y = (value->ev.yp>>4)-RR;
      pp->w = (RR*2);
      pp->h = (RR*2);

      switch(value->ev.type){
        case  B8_HIF_EV_TOUCH_START:
          fprintf( fp_bgprint, "%d t start\n" , (int)value->hdl );
          break;
        case  B8_HIF_EV_TOUCH_MOVE:
          fprintf( fp_bgprint, "%d t move\n" , (int)value->hdl );
          break;
        case  B8_HIF_EV_TOUCH_CANCEL:
          fprintf( fp_bgprint, "%d t cancel\n", (int)value->hdl );
          break;
        case  B8_HIF_EV_TOUCH_END:
          fprintf( fp_bgprint, "%d t end\n", (int)value->hdl );
          break;
        case  B8_HIF_EV_MOUSE_DOWN:
          fprintf( fp_bgprint, "%d m down %d %d\n" , (int)value->hdl , value->ev.xp>>4 , value->ev.yp>>4 );
          break;
        case  B8_HIF_EV_MOUSE_MOVE:
          fprintf( fp_bgprint, "%d m move %d %d\n" , (int)value->hdl , value->ev.xp>>4 , value->ev.yp>>4 );
          break;
        case  B8_HIF_EV_MOUSE_HOVER_MOVE:
          fprintf( fp_bgprint, "%d m hover %d %d\n" , (int)value->hdl , value->ev.xp>>4 , value->ev.yp>>4 );
          break;
        case  B8_HIF_EV_MOUSE_UP:
          fprintf( fp_bgprint, "%d m up %d %d\n" , (int)value->hdl , value->ev.xp>>4 , value->ev.yp>>4 );
          break;
      }
    }

    // mouse cursor
    {
      const b8HifMouseStatus* ms = b8HifGetMouseStatus();

      b8PpuRect* pp = b8PpuRectAllocZ( &_ppu_cmd , OTZ_MOUSE_CURSOR );
      pp->pal = ms->is_dragging ? B8_RED : B8_WHITE;
      const int RR = 32;
      pp->x = (ms->mouse_x>>4)-RR;
      pp->y = (ms->mouse_y>>4)-RR;
      pp->w = (RR*2);
      pp->h = (RR*2);
    }

    bgprint::ExportPpuCmd epc;
    epc._cmd = &_ppu_cmd;
    epc._otz = OTZ_BG_TEXT;
    bgprint::Export(fp_bgprint, epc);

    b8PpuExec( &_ppu_cmd );
  }
  return 0;
}