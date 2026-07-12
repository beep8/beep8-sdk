// cloudsavetest : exercises cloudsave::Set/Get (global KV over the /http
// file driver) with both a small value and one bigger than one 4000-byte
// chunk, to prove the internal chunking round-trips correctly.
#include <beep8.h>
#include <bgprint.h>
#include <cloudsave.h>
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

#define KEY_SMALL "CLOUDSAVETESTKEY"   // 16 chars
#define KEY_BIG   "CLOUDSAVEBIGKEY0"   // 16 chars

static  char  _bigbuf[ 9000 ];

int main(int argc,char* argv[]){
  (void)argc; (void)argv;
  printf("cloudsavetest\n");
  b8PpuGetResolution((u32*)&_PpuResoW ,(u32*)&_PpuResoH );

  bgprint::Reset();
  bgprint::Context ctx;
  FILE* bg = bgprint::Open( bgprint::CH3, nullptr, 512, ctx );
  _ASSERT( bg , "failed bgprint::Open()" );

  // --- small value round-trip ---
  bool set_ok = cloudsave::Set( KEY_SMALL, "hello cloudsave", 15 );   // 15 = strlen, no NUL
  char small_buf[32];
  int  small_n = cloudsave::Get( KEY_SMALL, small_buf, sizeof(small_buf) );
  bool small_ok = set_ok && small_n == 15 && memcmp( small_buf, "hello cloudsave", 15 ) == 0;

  fprintf( bg, "small set=%d get_n=%d\n%s\n", set_ok, small_n,
           small_ok ? "SMALL OK" : "SMALL FAIL" );

  // --- big value round-trip: exercises internal chunking (>4000 bytes) ---
  static char bigval[8500];
  for( int i = 0 ; i < (int)sizeof(bigval) ; ++i ) bigval[i] = 'A' + (i % 26);

  bool big_set_ok = cloudsave::Set( KEY_BIG, bigval, sizeof(bigval) );
  int  big_n = cloudsave::Get( KEY_BIG, _bigbuf, sizeof(_bigbuf) );
  bool big_ok = big_set_ok && big_n == (int)sizeof(bigval) &&
                memcmp( _bigbuf, bigval, sizeof(bigval) ) == 0;

  fprintf( bg, "big set=%d get_n=%d (want %d)\n%s\n", big_set_ok, big_n, (int)sizeof(bigval),
           big_ok ? "BIG OK" : "BIG FAIL" );

  printf( "small_ok=%d big_ok=%d\n", small_ok, big_ok );

  for( int frm=0 ; ; ++frm ){
    b8PpuVsyncWait();
    b8PpuCmdSetBuff( &_ppu_cmd , _ppu_cmd_buff , sizeof( _ppu_cmd_buff ) );
    b8PpuClearOT( &_ppu_cmd , &_ot[0] , &_ot_prev[0], MAX_OTZ );

    {
      b8PpuRect* pp = b8PpuRectAlloc( &_ppu_cmd );
      pp->pal = ( small_ok && big_ok ) ? B8_PPU_COLOR_GREEN : B8_PPU_COLOR_RED;
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
