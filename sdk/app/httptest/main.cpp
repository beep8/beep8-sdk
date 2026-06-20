// httptest : leaderboard HTTP path via the /http file driver (fopen/fread).
//
// Refactor of the inline-SCI thin slice: the SCI poking now lives in the
// reusable http driver (b8helper/src/httpdrv.cpp). The app just does a
// blocking fopen -> fwrite(url) -> fflush -> fread(body) -> fclose and prints
// the server response on screen.
#include <beep8.h>
#include <bgprint.h>
#include <httpdrv.h>
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

// dbg score server (prod would be :8083). 9083 must be reachable from the browser.
static const char* HTTP_URL = "https://beep8.org:9083/score?game=1d-pacman";

int main(int argc,char* argv[]){
  (void)argc; (void)argv;
  printf("httptest @file-driver\n");
  b8PpuGetResolution((u32*)&_PpuResoW ,(u32*)&_PpuResoH );

  bgprint::Reset();
  bgprint::Context ctx;
  FILE* bg = bgprint::Open( bgprint::CH3, nullptr, 256, ctx );
  _ASSERT( bg , "failed bgprint::Open()" );
  fprintf( bg, "GET %s\n", HTTP_URL );

  // --- HTTP GET through the /http file driver (blocks until response) ---
  http::Reset();
  static char resp[256];
  int  rlen = 0;
  FILE* fp = fopen( "/http/con0", "r+" );
  if( fp ){
    fwrite( HTTP_URL, 1, strlen(HTTP_URL), fp );
    fflush( fp );                              // r+ sync: send the request
    rlen = (int)fread( resp, 1, sizeof(resp)-1, fp );
    if( rlen < 0 ) rlen = 0;
    fclose( fp );
  }
  resp[ rlen ] = 0;

  // Full text -> browser DevTools console; wrapped at 16 cols -> screen.
  printf( "RESP[%d]: %s\n", rlen, rlen > 0 ? resp : "(empty/error)" );
  char wrapped[320];
  int  w = 0;
  for( int i = 0 ; i < rlen && w < (int)sizeof(wrapped) - 2 ; ++i ){
    wrapped[ w++ ] = resp[i];
    if( ((i + 1) % 16) == 0 ) wrapped[ w++ ] = '\n';
  }
  wrapped[ w ] = 0;
  fprintf( bg, "RESP:\n%s\n", rlen > 0 ? wrapped : "(empty/error)" );

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
