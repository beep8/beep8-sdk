// httptest : thin vertical slice for the leaderboard HTTP path.
//
// Sends one HTTPS GET to the standalone score server through the VHttp SCI
// bridge (emulator host side) and prints the response body on screen. This
// is the deliberately-minimal end-to-end proof ("a server-origin string on a
// BEEP-8 screen") before the reusable /http file driver is built.
//
// Wire protocol (must match js.b8/vhttp.js):
//   ROM -> host : URL bytes, then 0x0a ('\n')
//   host -> ROM : response body bytes, then 0xff (EOF)
#include <beep8.h>
#include <bgprint.h>
#include <stdio.h>

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

// SCI channel for HTTP. Must match idx_vhttp in js.b8/beep8.js.
#define SCI_CH_HTTP   (2)
#define HTTP_REQ_EOL  (0x0a)
#define HTTP_RES_EOF  (0xff)

// dbg score server (prod would be :8083). 9083 must be reachable from the browser.
static const char* HTTP_URL = "https://beep8.org:9083/score?game=1d-pacman";

static void http_get_begin(const char* url){
  for( const char* p = url ; *p ; ++p ){
    B8_FIFO_SCI_TX( SCI_CH_HTTP ) = (u8)*p;
  }
  B8_FIFO_SCI_TX( SCI_CH_HTTP ) = HTTP_REQ_EOL;
}

int main(int argc,char* argv[]){
  (void)argc; (void)argv;
  printf("httptest @thin-slice\n");
  b8PpuGetResolution((u32*)&_PpuResoW ,(u32*)&_PpuResoH );

  bgprint::Reset();
  bgprint::Context ctx;
  FILE* fp = bgprint::Open( bgprint::CH3, nullptr, 256, ctx );
  _ASSERT( fp , "failed bgprint::Open()" );
  fprintf( fp, "GET %s\n", HTTP_URL );

  // Kick the request once; the host fetch()es it across the next few frames.
  http_get_begin( HTTP_URL );

  static char resp[256];
  int  rlen = 0;
  bool done = false;
  bool shown = false;

  for( int frm=0 ; ; ++frm ){
    // Drain whatever the host pushed back since the previous frame.
    while( !done && B8_FIFO_SCI_RX_LEN( SCI_CH_HTTP ) > 0 ){
      u8 b = B8_FIFO_SCI_RX( SCI_CH_HTTP );
      if( b == HTTP_RES_EOF ){ done = true; break; }
      if( rlen < (int)sizeof(resp)-1 ) resp[ rlen++ ] = (char)b;
    }
    resp[ rlen ] = 0;

    if( done && !shown ){
      fprintf( fp, "RESP: %s\n", rlen > 0 ? resp : "(empty/error)" );
      shown = true;
    }

    b8PpuVsyncWait();   // completes the frame -> host runs -> fetch progresses
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
    bgprint::Export( fp, epc );

    b8PpuExec( &_ppu_cmd );
  }
  return 0;
}
