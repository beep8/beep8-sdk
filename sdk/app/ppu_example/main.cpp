/*!
 * @file main.cpp
 * @brief Sample application demonstrating the basic usage of 
 *        the Pixel Processing Unit (PPU) in the BEEP-8 system.
 *
 * BEEP-8 includes a virtual graphics LSI called the Pixel Processing Unit (PPU). 
 * This sample provides a comprehensive overview of the fundamental 
 * features and usage of the PPU. 
 * Developers who wish to directly control the PPU can refer to this sample as a guide.
 *
 * Note:  If you are using the pico8 compatible library, 
 *        the PPU is abstracted away, so this sample may not be necessary.
 */
#include <stdio.h>
#include <unistd.h>
#include <beep8.h>
#include <time.h>
#include <string.h>
#include <exception>
#include <bgprint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <crt/crt.h>
#include <b8/misc.h>
#include <b8/ppu.h>
#include <fontdata.h>
#include <submath.h>
#include <sublibc.h>

/// PPU resolution
static  s32 _PpuResoW;
static  s32 _PpuResoH;

/// PPU command buffer
#define PPU_CMD_BUFF_WORDS (16*1024)
static  u32 _ppu_cmd_buff[ PPU_CMD_BUFF_WORDS ];
static  b8PpuCmd  _ppu_cmd;

/// Drawing priority
#define MAX_OTZ (16)
enum EnOtz {
  OTZ_CLEAR     = MAX_OTZ - 1,
  OTZ_SETPAL    = MAX_OTZ - 2,
  OTZ_RECT      = MAX_OTZ - 3,
  OTZ_POLY      = MAX_OTZ - 4,
  OTZ_LINE      = MAX_OTZ - 5,
  OTZ_BG_BEHIND = MAX_OTZ - 6,
  OTZ_BG_FRONT  = MAX_OTZ - 7,
  OTZ_SPRITE    = MAX_OTZ - 8,
  OTZ_BG_TEXT   = MAX_OTZ - 9,
  OTZ_PUSH_BACK = MAX_OTZ - 10,
};
static  u32 _ot[ MAX_OTZ ];
static  u32 _ot_prev[ MAX_OTZ ];

/// Background surface (for text)
#define TEXT_BG_WTILE_POW2 (4)
#define TEXT_BG_HTILE_POW2 (5)
#define TEXT_BG_WTILE (1<<TEXT_BG_WTILE_POW2)
#define TEXT_BG_HTILE (1<<TEXT_BG_HTILE_POW2)
static  b8PpuBgTile _bg_text[ TEXT_BG_WTILE * TEXT_BG_HTILE ];

/// Background surfaces with double scrolling sample. _bg_tiles[0] is in the back, _bg_tiles[1] is in the front.
#define BG_WTILE_POW2  (4) 
#define BG_HTILE_POW2  (5) 
#define BG_WTILE (1<<BG_WTILE_POW2)
#define BG_HTILE (1<<BG_HTILE_POW2)
static  b8PpuBgTile _bg_tiles[2][ BG_WTILE * BG_HTILE ];

/// Vertex structure
struct Point {
  fx8 _x;
  fx8 _y;
  fx8 _vx;
  fx8 _vy;
};

/// Use these parameters if you want to test with more elements
#define NUM_OF_TEST_LINES       (7) 
#define NUM_OF_TEST_TRIANGLES   (5) 
#define NUM_OF_TEST_RECT        (6) 
#define NUM_OF_TEST_LINE_POINTS (NUM_OF_TEST_LINES*2 + NUM_OF_TEST_TRIANGLES*3 + NUM_OF_TEST_RECT)

/// Array of vertices
Point LinePoints[ NUM_OF_TEST_LINE_POINTS ];

/// Initialize the vertex positions with random values
static  void  _points_reset(){
  for( size_t nn=0 ; nn < NUM_OF_TEST_LINE_POINTS ; ++nn ){
    LinePoints[ nn ]._x = genrand_min_max_fx8( fx8(0), fx8(_PpuResoW) );
    LinePoints[ nn ]._y = genrand_min_max_fx8( fx8(0), fx8(_PpuResoH) );
    LinePoints[ nn ]._vx = genrand_min_max_fx8( fx8(-2), fx8(+2) );
    LinePoints[ nn ]._vy = genrand_min_max_fx8( fx8(-2), fx8(+2) );
  }
}

/// Perform calculations for one frame of vertex movement
static  void  _points_step(){
  for( size_t nn=0 ; nn < NUM_OF_TEST_LINE_POINTS ; ++nn ){
    Point& point = LinePoints[ nn ];
    point._x += point._vx;
    if( point._x < fx8(0) ){
      point._x = fx8(0);
      point._vx = -point._vx;
    } else if ( point._x >= fx8( _PpuResoW ) ){
      point._x = fx8( _PpuResoW );
      point._vx = -point._vx;
    }

    point._y += point._vy;
    if( point._y < fx8(0) ){
      point._y = fx8(0);
      point._vy = -point._vy;
    } else if ( point._y >= fx8( _PpuResoH ) ){
      point._y = fx8( _PpuResoH );
      point._vy = -point._vy;
    }
  }
}

/// Load sprite data.
/// The data from ./data/import/sprite1.png is defined in ./data/export/sprite1/png.cpp as a C array variable.
/// The array's starting address, width, and height are defined by b8_image_sprite1_get().
/// sprite1.png can be created with any graphics tool, and running 'make' will automatically convert the content 
/// of sprite1.png into png.cpp and compile it.
extern "C" const uint8_t* b8_image_sprite1_get( uint16_t* width, uint16_t* height );
static  void  load_sprite_image(){
  b8PpuCmdSetBuff( &_ppu_cmd , _ppu_cmd_buff , sizeof( _ppu_cmd_buff ) );
  uint16_t ww,hh;
  const uint8_t* pix = b8_image_sprite1_get(&ww,&hh);

  {
    b8PpuLoadimg* pp = b8PpuLoadimgAlloc( &_ppu_cmd );
    pp->cpuaddr = pix;

    // src
    pp->srcxtile = 0;
    pp->srcytile = 0;
    pp->srcwtile = ww >>3;

    // dst
    pp->dstxtile = 0;
    pp->dstytile = 0;
    pp->trnwtile = ww >>3;
    pp->trnhtile = hh >>3;
  }

  {
    b8PpuFlush* pp = b8PpuFlushAlloc( &_ppu_cmd );
    pp->img = 1;
  }
  b8PpuHaltAlloc( &_ppu_cmd );
  b8PpuExec( &_ppu_cmd );
  b8PpuVsyncWait();
}

/// Initialize the background surface.
/// Prepares an array of b8PpuBgTile structures in the RAM space accessible from the CPU.
static  void  _bg_reset(){
  for( size_t nn=0 ; nn<numof(_bg_tiles[0]) ; ++nn ){
    b8PpuBgTile& tile = _bg_tiles[0][nn];
    tile.XTILE = tile.YTILE = 0;
    tile.VFP = 0;
    tile.HFP = 0;
    tile.PAL = 0;
  }

  for( u16 nc=0 ; nc<8 ; ++nc ){
    u16 sx = nc<<1;
    u16 sy = rand() % (TEXT_BG_HTILE-2);
    for( int yy=0 ; yy<2 ; ++yy){
      for( int xx=0 ; xx<2 ; ++xx){
        b8PpuBgTile& tile = _bg_tiles[0][ ((sy+yy)<<BG_WTILE_POW2) + (sx+xx) ];
        tile.XTILE = 0 + xx;
        tile.YTILE = 3 + yy;
      }
    }
  }

  for( size_t nn=0 ; nn<numof(_bg_tiles[1]) ; ++nn ){
    b8PpuBgTile& tile = _bg_tiles[1][nn];
    if( rand()&7 ){
      tile.XTILE = tile.YTILE = 0;
      continue;
    }

    tile.XTILE = rand()%5;
    if( tile.XTILE == 3 ){  // coin ?
      tile.PAL = 1;
    }
    tile.YTILE = 2;
  }
}

static  int open_bgprint(){
  /// You can output arbitrary strings on the background surface using standard libc functions like fprintf
  const char* fname = "/bgprint/con3";  // or "con1" or "con2" or "con3"
  FILE* fp_bgprint = fopen(fname,"wt" );
  _ASSERT( fp_bgprint , fname );

  /// Assign buffering to a stream
  setvbuf( fp_bgprint , NULL, _IONBF, 0);

  /// Get fileno
  const int fd = fileno( fp_bgprint );

  /// Set context
  bgprint::Context ctx;
  ctx.cpuaddr = _bg_text;
  ctx._h_pow2 = TEXT_BG_HTILE_POW2;
  ctx._w_pow2 = TEXT_BG_WTILE_POW2;
  ioctl(fd, bgprint::SET_SLOT_CONTEXT , &ctx );

  fprintf(fp_bgprint, "\e[3qppu_example\n" );
  return  fd;
}

int main(){
  printf( "BEEP-8 PPU Example\n" );
  b8PpuGetResolution((u32*)&_PpuResoW ,(u32*)&_PpuResoH );
  _points_reset();
  load_sprite_image();
  bgprint::Reset();
  _bg_reset();
  int fd = open_bgprint();

  /// fx8 is an 8-bit fixed-point type provided by the BEEP-8 SDK
  fx8 xx(0);
  fx8 yy( _PpuResoH - (_PpuResoH>>2) );
  fx8 vx(1);
  fx8 vy(0);

  u32 cnt = 0;
  for( int frm=0 ; ; ++frm ){
    u16 nn;

    b8PpuVsyncWait();
    _points_step();
    b8PpuCmdSetBuff( &_ppu_cmd , _ppu_cmd_buff , sizeof( _ppu_cmd_buff ) );
    b8PpuClearOT( &_ppu_cmd , &_ot[0] , &_ot_prev[0], MAX_OTZ );

    /// Clear the backmost layer with the specified color B8_PPU_COLOR_BLUE
    {
      b8PpuRect* pp = b8PpuRectAllocZ( &_ppu_cmd , OTZ_CLEAR );
      pp->pal = B8_PPU_COLOR_BLUE;
      pp->x = 0;
      pp->y = 0;
      pp->w = _PpuResoW;
      pp->h = _PpuResoH;
    }

    /// Output the contents of bgprint
    bgprint::ExportPpuCmd epc;
    epc._cmd = &_ppu_cmd;
    epc._otz = OTZ_BG_TEXT;
    ioctl(fd, bgprint::EXPORT_PPU_CMD , &epc );
    
    /// Calculate the coordinates for a bouncing sprite along a parabolic path and issue drawing commands
    {
      xx += vx;
      yy += vy;
      vy += fx8(31,300);
      if( xx > fx8( _PpuResoW )){
        xx -= fx8( _PpuResoW );
      } else if ( xx < fx8(0) ){
        xx += fx8( _PpuResoW );
      }

      const fx8 y_bottom(_PpuResoH - 7 );
      if( yy > y_bottom ){
        yy = y_bottom;
        vy = genrand_min_max_fx8( -fx8(5) , -fx8(1,5) );
        vx = genrand_min_max_fx8( -fx8(1) , +fx8(1) );
        cnt = 0;
      }

      b8PpuSprite* pp = b8PpuSpriteAllocZ( &_ppu_cmd , OTZ_SPRITE );
      pp->pal = 0;
      pp->x = int(xx);
      pp->y = int(yy);
      pp->srcwtile = pp->srchtile = 1;
      pp->vfp = 0;
      pp->hfp = vx > fx8(0) ? 0:1;
      static const u8 tbl[] = {1,1,1,1,2,2,2,2,2,2,2,2};
      if( cnt < numof(tbl) ){
        pp->srcxtile = tbl[cnt];
      } else {
        pp->srcxtile = 0;
      }
      pp->srcytile = 1;
    }

    /// Enable backface culling.
    /// When drawing triangles with Polygon (b8PpuPoly), set whether to cull when the 3 vertices are counterclockwise.
    {
      b8PpuEnable* pp = b8PpuEnableAllocZ( &_ppu_cmd, OTZ_CLEAR );
      pp->cul = (frm>>7)&1 ?  B8_PPU_CMD_ENABLE_CULLING_OF_POLYGONS : 0;
    }

    Point* lp = LinePoints;

    /// Line drawing sample
    for( nn=0 ; nn < NUM_OF_TEST_LINES ; ++nn ) {
      b8PpuLine* pp = b8PpuLineAllocZ( &_ppu_cmd , OTZ_LINE );
      pp->pal = (nn+8) & 15;
      pp->width = 2;
      pp->x0 = lp->_x;
      pp->y0 = lp->_y;
      ++lp;

      pp->x1 = lp->_x;
      pp->y1 = lp->_y;
      ++lp;
    }

    /// Triangle drawing sample
    for( nn=0 ; nn < NUM_OF_TEST_TRIANGLES ; ++nn ) {
      b8PpuPoly* pp = b8PpuPolyAllocZ( &_ppu_cmd, OTZ_POLY );
      pp->pal = (nn+11) & 15;

      pp->x0 = lp->_x;
      pp->y0 = lp->_y;
      ++lp;

      pp->x1 = lp->_x;
      pp->y1 = lp->_y;
      ++lp;

      pp->x2 = lp->_x;
      pp->y2 = lp->_y;
      ++lp;
    }

    /// Monochrome rectangle drawing sample
    for( nn=0 ; nn < NUM_OF_TEST_RECT ; ++nn ) {
      b8PpuRect* pp = b8PpuRectAllocZ( &_ppu_cmd, OTZ_RECT );
      pp->pal = 15-(nn & 15);
      pp->x = lp->_x;
      pp->y = lp->_y;
      pp->w = ((nn+1)*187)&63;
      pp->h = ((nn+1)*391)&63;

      ++lp;
    }

    /// Sample of changing the palette of the background surface.
    /// This sample changes the palette of the coin (yellow) on the background surface.
    {
      b8PpuSetpal* pp = b8PpuSetpalAllocZ( &_ppu_cmd, OTZ_BG_BEHIND, 1 );
      pp->palsel = 1;
      if(!( (frm>>3) & 7 ) ){
        pp->pidx10 = 1;
        pp->pidx4 = 2;
      }
    }

    /// Sample of background surface drawing.
    /// By preparing an array of b8PpuBgTile accessible by the CPU in advance, and generating a drawing command 
    /// with its starting address and tile size in X and Y directions, the background surface is drawn.
    /// Two surfaces _bg_tiles[0] and _bg_tiles[1] are combined.
    static u16 vpix[ 2 ] = {0,0};
    for( int layer = 0 ; layer < 2 ; ++layer ) {
      b8PpuBg* pp = b8PpuBgAllocZ( &_ppu_cmd , 0 == layer ? OTZ_BG_BEHIND : OTZ_BG_FRONT );
      pp->upix = 0;
      pp->vpix = vpix[ layer ];
      vpix[ layer ] -= (layer+1);
      pp->vwrap = pp->uwrap = B8_PPU_BG_WRAP_REPEAT;
      pp->wtile = BG_WTILE_POW2;
      pp->htile = BG_HTILE_POW2;  
      pp->cpuaddr = _bg_tiles[ layer ];
    }

    for( nn=0 ; nn<16 ; ++nn){
      b8PpuRect* pp = b8PpuRectAllocZPB( &_ppu_cmd , OTZ_PUSH_BACK );
      pp->pal = nn&15;
      pp->x = nn*8;
      pp->y = 100;
      pp->w = 20;
      pp->h = 12;
    }

    if( get_errno() < 0 ){
      printf( "get_errno() : %d\n" , get_errno() );
      _ASSERT(0,"get_errno() returns error");
    }

    /// Execute the command group created in _ppu_cmd to start drawing.
    b8PpuExec( &_ppu_cmd );
    ++cnt;
  }
  return 0;
}