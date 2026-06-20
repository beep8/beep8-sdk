// savetest : exercise the /savedata key-value store (cookie-equivalent).
//
// On each run it:
//   1. get player_name   -> shows what persisted from a previous session
//   2. set player_name=ZEP
//   3. get player_name   -> shows the round-trip within this session
// Reload the page and run 1 should now report ZEP (persistence proven).
#include <beep8.h>
#include <bgprint.h>
#include <savedata.h>
#include <stdio.h>
#include <string.h>

static  s32 _PpuResoW = 128;
static  s32 _PpuResoH = 128;

#define PPU_CMD_BUFF_WORDS (8*1024)
static  u32       _ppu_cmd_buff[ PPU_CMD_BUFF_WORDS ];
static  b8PpuCmd  _ppu_cmd;

#define MAX_OTZ (16)
enum EnOtz {
  OTZ_CLEAR   = MAX_OTZ - 1,
  OTZ_BG_TEXT = MAX_OTZ - 3,
};
static  u32   _ot[ MAX_OTZ ];
static  u32   _ot_prev[ MAX_OTZ ];

static int sd_get( const char* key, char* out, int out_cap ){
  FILE* fp = fopen( "/savedata/con0", "r+" );
  if( !fp ) return -1;
  char cmd[64];
  int  n = snprintf( cmd, sizeof(cmd), "get %s", key );
  fwrite( cmd, 1, n, fp );
  fflush( fp );
  int got = (int)fread( out, 1, out_cap - 1, fp );
  if( got < 0 ) got = 0;
  out[ got ] = 0;
  fclose( fp );
  return got;
}

static void sd_set( const char* key, const char* val ){
  FILE* fp = fopen( "/savedata/con0", "r+" );
  if( !fp ) return;
  char cmd[96];
  int  n = snprintf( cmd, sizeof(cmd), "set %s=%s", key, val );
  fwrite( cmd, 1, n, fp );
  fclose( fp );
}

int main(int argc,char* argv[]){
  (void)argc; (void)argv;
  printf("savetest @savedata\n");
  b8PpuGetResolution((u32*)&_PpuResoW ,(u32*)&_PpuResoH );

  bgprint::Reset();
  bgprint::Context ctx;
  FILE* bg = bgprint::Open( bgprint::CH3, nullptr, 256, ctx );
  _ASSERT( bg , "failed bgprint::Open()" );

  savedata::Reset();

  char v1[16] = {0};
  int  n1 = sd_get( "player_name", v1, sizeof(v1) );
  sd_set( "player_name", "ZEP" );
  char v2[16] = {0};
  int  n2 = sd_get( "player_name", v2, sizeof(v2) );

  printf( "get1[%d]=%s  get2[%d]=%s\n", n1, v1, n2, v2 );
  fprintf( bg, "persisted:\n %s\n", n1 > 0 ? v1 : "(none)" );
  fprintf( bg, "after set:\n %s\n", n2 > 0 ? v2 : "(none)" );
  fprintf( bg, "\nreload to see\npersisted=ZEP\n" );

  for( int frm=0 ; ; ++frm ){
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
    bgprint::Export( bg, epc );

    b8PpuExec( &_ppu_cmd );
  }
  return 0;
}
