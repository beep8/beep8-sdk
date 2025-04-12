#include <beep8.h>
#include <bgprint.h>

static  s32 _PpuResoW = 128;
static  s32 _PpuResoH = 128;

#define PPU_CMD_BUFF_WORDS (8*1024)
static  u32       _ppu_cmd_buff[ PPU_CMD_BUFF_WORDS ];
static  b8PpuCmd  _ppu_cmd;

#define MAX_OTZ (16)
enum EnOtz {
  OTZ_CLEAR   = MAX_OTZ - 1,
  OTZ_RECT    = MAX_OTZ - 2,
  OTZ_BG_TEXT = MAX_OTZ - 3,
};
static  u32   _ot[ MAX_OTZ ];
static  u32   _ot_prev[ MAX_OTZ ];

int main(int argc,char* argv[]){
  (void)argc; (void)argv;
  printf( "bgprint @2023_0616a\n" );
  b8PpuGetResolution((u32*)&_PpuResoW ,(u32*)&_PpuResoH );

  bgprint::Reset();

  // open bgprint
  bgprint::Context ctx;
  FILE* fp_bgprint = bgprint::Open(
    bgprint::CH3,
    nullptr, 256,
    ctx
  );
  _ASSERT( fp_bgprint , "failed bgprint::Open()" );
  fprintf(fp_bgprint, "hello bgprint\n" );

  for( int frm=0 ; ; ++frm ){
    if( !(frm&7) ){
      static int nn = 0;
	  fprintf(fp_bgprint, "n=%d n^2=%d\n",nn,nn*nn);
	  ++nn;
	  printf("n=%d n^2=%d\n",nn,nn*nn);
    }

    b8PpuVsyncWait();
    b8PpuCmdSetBuff( &_ppu_cmd , _ppu_cmd_buff , sizeof( _ppu_cmd_buff ) );
    b8PpuClearOT( &_ppu_cmd , &_ot[0] , &_ot_prev[0], MAX_OTZ );

    {
      b8PpuRect* pp = b8PpuRectAlloc( &_ppu_cmd );
      pp->pal = B8_PPU_COLOR_BLUE;
      pp->x = 0; pp->y = 0;
      pp->w = _PpuResoW; pp->h = _PpuResoH;
      b8PpuPushFrontOT( &_ppu_cmd  , OTZ_CLEAR, pp );
    }

    bgprint::ExportPpuCmd epc;
    epc._cmd = &_ppu_cmd;
    epc._otz = OTZ_BG_TEXT;
    bgprint::Export(fp_bgprint, epc);

    b8PpuExec( &_ppu_cmd );
  }
  return 0;
}
