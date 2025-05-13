#include <pico8.h>
#include <sprprint.h>
#include <b8/syscall.h>
#include <b8/sys.h>
#include <b8/hif.h>
#include <hif_decoder.h>
#include <sublibc.h>
#include <submath.h>
#include <exception>
#include <vector>
#include <cstring>
#include <esc_decoder.h>
#include <assert.h>
#include <bit>
#include <map>
#include <bgprint.h>

using namespace std;
using namespace pico8;

namespace pico8 {

#define MAX_SPR_BANK  (14)

static  bool  _is_colliding( const Line& line , const Rect& rc );
static  bool  _is_colliding( const Poly& pol , const Rect& rc );
static  bool  _is_colliding( const Vec& pos , const Rect& rc );
static  bool  _is_colliding( const Rect& rc1, const Rect& rc2);
static  Xorshift32 xors;


// TODO: remove tester
static  void test_esc(FILE *fp) {
return;
  static int tt=0;
  pico8::sprint(11, tt, RED, "Red(x=50 y=%d)\n",tt);
  pico8::sprint(tt, 50, BLUE, "Blue(x=%d, y=50)\n",tt);
  ++tt;
  return;
   
  fprintf( fp, "\e[%d;%dH",32,22);
  fprintf(fp, "\e[0mreset\n");

  static int cnt = 0;
  ++cnt;
  //if( cnt > 45) return;

  fprintf(fp, "\e[1;37;41mA_WHITE / A_RED\n");    // 白い文字、赤い背景
  fprintf(fp, "\e[1;30;47mA_BLACK / A_WHITE\n");  // 黒い文字、白い背景
  fprintf(fp, "\e[1;31;40mA_RED / A_BLACK\n");    // 赤い文字、黒い背景
  fprintf(fp, "\e[1;32;44mA_GREEN / A_BLUE\n");   // 緑の文字、青い背景
  fprintf(fp, "\e[1;33;45mA_YELLOW / A_MAGENTA\n"); // 黄色の文字、マゼンタ背景
  fprintf(fp, "\e[1;34;43mA_BLUE / A_YELLOW\n");  // 青い文字、黄色の背景
  fprintf(fp, "\e[1;35;42mA_MAGENTA / A_GREEN\n"); // マゼンタ文字、緑の背景
  fprintf(fp, "\e[1;36;46mA_CYAN / A_CYAN\n");    // シアンの文字、シアンの背景
  fprintf(fp, "\e[1;90;41mA_BRIGHT_BLACK / A_RED\n"); // 明るい黒文字、赤背景
  fprintf(fp, "\e[1;97;100mA_BRIGHT_WHITE / A_BRIGHT_BLACK\n"); // 明るい白文字、明るい黒背景

  // 前景色と背景色の組み合わせを短縮表記でテスト
  fprintf(fp, "\e[57;72mWhite/Purple\n"); // 前景: White, 背景: Dark Purple
  fprintf(fp, "\e[59;73mOrange/Green\n"); // 前景: Orange, 背景: Dark Green
  fprintf(fp, "\e[60;74mYellow/Brown\n"); // 前景: Yellow, 背景: Brown
  fprintf(fp, "\e[61;75mGreen/DarkGrey\n"); // 前景: Green, 背景: Dark Grey
  fprintf(fp, "\e[62;76mBlue/LightGrey\n"); // 前景: Blue, 背景: Light Grey
  fprintf(fp, "\e[63;77mLavender/White\n"); // 前景: Lavender, 背景: White
  fprintf(fp, "\e[64;78mPink/Red\n"); // 前景: Pink, 背景: Red
  fprintf(fp, "\e[65;79mPeach/Orange\n"); // 前景: Light Peach, 背景: Orange
  fprintf(fp, "\e[50;71mBlack/Blue\n"); // 前景: Black, 背景: Dark Blue
  fprintf(fp, "\e[51;70mDarkBlue/Black\n"); // 前景: Dark Blue, 背景: Black
}

static  void _test_fgetset(){  // TODO: remove
  printf("Starting fset/fget tests...\n");

  // テストケース1: 初期状態確認
  printf("Test Case 1: Initial state\n");
  assert(fget(10) == 0);  // スプライト10はフラグがすべて0
  assert(fget(20) == 0);  // スプライト20もフラグがすべて0
  assert(fget(10, 0) == 0);
  assert(fget(10, 1) == 0);
  
  // テストケース2: フラグを個別に設定して取得
  printf("Test Case 2: Set and get individual flags\n");
  fset(10, 0, 1);  // スプライト10、フラグ0を1に
  assert(fget(10, 0) == 1);
  assert(fget(10) == 1);  // 全体のビットフィールドは 00000001

  fset(10, 1, 1);  // フラグ1を1に
  assert(fget(10, 1) == 1);
  assert(fget(10) == 3);  // 全体のビットフィールドは 00000011

  fset(10, 2, 1);  // フラグ2を1に
  assert(fget(10, 2) == 1);
  assert(fget(10) == 7);  // 全体のビットフィールドは 00000111

  // テストケース3: フラグをクリアして取得
  printf("Test Case 3: Clear individual flags\n");
  fset(10, 1, 0);  // フラグ1をクリア
  assert(fget(10, 1) == 0);
  assert(fget(10) == 5);  // 全体のビットフィールドは 00000101

  // テストケース4: 全フラグを一度に設定して取得
  printf("Test Case 4: Set all flags at once\n");
  fset(10, 0xff, 0b10101010);  // スプライト10の全フラグを10101010に設定
  assert(fget(10) == 0b10101010);
  assert(fget(10, 0) == 0);
  assert(fget(10, 1) == 1);
  assert(fget(10, 2) == 0);
  assert(fget(10, 3) == 1);

  // テストケース5: 異なるスプライトバンクでのテスト
  printf("Test Case 5: Test with different sprite banks\n");
  fset(10, 0, 1, 1);  // スプライトバンク1のスプライト10、フラグ0を1に設定
  assert(fget(10, 0, 1) == 1);
  assert(fget(10, 0, 0) == 0);  // デフォルトバンクでは影響なし

  fset(10, 0xff, 0b11110000, 1);  // スプライトバンク1で全フラグを設定
  assert(fget(10, 0, 1) == 0);
  assert(fget(10, 4, 1) == 1);  // フラグ4が1

  // テストケース6: エッジケースのテスト（フラグ範囲外を設定）
  printf("Test Case 6: Out of range flag tests\n");
  // この部分はassertでエラーハンドリングされるので実際のコード上ではエラーを防止
  //fset(10, 8, 1);  // フラグ8は存在しない -> アサートエラー

  // テストケース7: 無効なスプライトバンクを指定
  printf("Test Case 7: Invalid sprite bank tests\n");
  printf("fset/fget tests passed!\n");
}
}

namespace pico8 {

struct  ButtonStatus{
  u32 frm_pressed [ BUTTON_MAX ] = {};
  u32 frm_released[ BUTTON_MAX ] = {};
};

struct  MouseStatus{
  s32 x = 0;
  s32 y = 0;
  u16 btn_status = 0x0000;
  void  ClearStatus(){
    btn_status = 0x0000;
  }
};

struct BgConfig {
  bool ready = false;
  BgTiles wtile;
  BgTiles htile;
  u8      ppu_wtile;
  u8      ppu_htile;
  BgTilesPtr tiles;
  u8      uwrap;
  u8      vwrap;
};

enum Status {
  IDLE,
  RUNNING,
  ERROR
};

#define MAX_OTZ     (16)
#define OTZ_BG_TEXT (1)
static  u32  _ot        [ MAX_OTZ ];
static  u32  _ot_prev   [ MAX_OTZ ];
static  u32* _jmp_prev  [ MAX_OTZ ];

static  void  clear_jmp_prev( b8PpuCmd* ppu_cmd ){
  _jmp_prev [ 0 ] = ppu_cmd->addr_halt;
  for( size_t nn=1 ; nn < MAX_OTZ ; ++nn ){
    _jmp_prev [ nn ] = ppu_cmd->ot + (nn-1);
  }
}

#define PLAYER_MAX  (2)
#define PPU_CMD_BUFF_WORDS (16*1024)
static  u32       _cnt_update;
static  u32       _ppu_cmd_buff[ PPU_CMD_BUFF_WORDS ];
static  b8PpuCmd  _ppu_cmd;
static  s32       _reso_w     = 0;
static  s32       _reso_h     = 0;
static  Color     _color        = BLACK;
static  bool      _during_draw  = false;
static  Error     _error        = NO_ERROR;
static  Status    _status       = IDLE;
static  Vec       _camera_cur;
static  Vec       _camera_prev;
static  s16       _otz = MAX_OTZ>>1;  
static  Rect      _clip_cur;
static  Rect      _clip_prev;
static  SprCursor _spr_cursor_prev;
static  BgCursor  _bg_cursor_prev;
static  FILE*     _fp_sprprint;
static  FILE*     _fp_bgprint;
static  FILE*     _fp_bgprint_debug = 0;
static  BgConfig  _bg_config[ BG_MAX ];
static  ButtonStatus  _button_status[ PLAYER_MAX ];
static  MouseStatus   _mouse_status;
static  shared_ptr< CHifDecoder > _hif_decoder;
static  bool  _init_dprint;
static  bool  _dprint_enabled;

#define SPRITE_PATTERN_BANK_NUM (16)
static  u8        _sprite_flags[ SPRITE_PATTERN_BANK_NUM ][256];
static  const uint8_t* sprite_sheets[ MAX_SPR_BANK ] = {0};

enum EnOtz {
  OTZ_CLEAR     = MAX_OTZ - 1,
};

struct CameraStack{
  Vec _save;
  CameraStack(){  _save = _camera_cur; }
  ~CameraStack(){ _camera_cur = _save; } 
};

static  void  set_seed_from_time(){
  const u64 current_time_msec = (static_cast< u64 >( B8_INF_CAL_H ) << 32) | static_cast< u64 >( B8_INF_CAL_L );
  u32 seed = static_cast< u32 >(current_time_msec ^ (current_time_msec >> 33));
  seed ^= seed >> 16;
  seed *= 0x85ebca6b;
  seed ^= seed >> 13;
  seed *= 0xc2b2ae35;
  seed ^= seed >> 16;
  pico8::srand( seed );
}

static  void  _reset(){
  _cnt_update = 0;
  _status = IDLE;
  _color  = BLACK;
  _during_draw = false;
  _error  = NO_ERROR;
  _camera_cur.set();
  _camera_prev = _camera_cur;
  _otz = MAX_OTZ>>1;  
  set_seed_from_time();

  b8PpuGetResolution((u32*)&_reso_w ,(u32*)&_reso_h );
  _clip_cur.SetXYWH(0,0,_reso_w,_reso_h);
  _clip_prev = _clip_cur;
  _spr_cursor_prev.Reset();
  _bg_cursor_prev.Reset();
  memset(_sprite_flags, 0, sizeof(_sprite_flags));

  for( size_t nn=0 ; nn < numof( _button_status ) ; ++nn ){
    _button_status[ nn ] = ButtonStatus();
  }
  _hif_decoder = make_shared< CHifDecoder >(); 
  _mouse_status = MouseStatus();
  _init_dprint = false;

  {
    sprprint::Reset();
    sprprint::Context ctx;
    ctx._cmd = &_ppu_cmd;
    _fp_sprprint = sprprint::Open( sprprint::CH1, ctx );
    _ASSERT(_fp_sprprint,"sprprint::Open");
  }

  {
    bgprint::Reset();
    bgprint::Context ctx;
    ctx._scroll = false;
    _fp_bgprint = bgprint::Open(
      bgprint::CH6,
      nullptr, 256,
      ctx
    );
    _ASSERT( _fp_bgprint , "failed bgprint::Open()" );
  }

  _dprint_enabled = true;
}

void  seterr( Error error ){
  if( _error != NO_ERROR )  return;
  _error = error;
  _status = ERROR;
}

static  bool  has_error(){
  return  _error != NO_ERROR;
}

static  void  hif_update(){
  ButtonStatus& bs = _button_status[ 0 ];
  for (Button it = BUTTON_LEFT; it <= BUTTON_X ; it = static_cast<Button>(it + 1)) {
    if( btn( it ) ){
      bs.frm_pressed[ it ]++;
      bs.frm_released[ it ] = 0;
    } else {
      bs.frm_pressed[ it ] = 0;
      bs.frm_released[ it ]++;
    } 
  }

  _mouse_status.ClearStatus();
  const b8HifMouseStatus* ms = _hif_decoder->GetMouseStatus();
  if( ms->is_dragging ){
    _mouse_status.btn_status |= LEFT;
    bs.frm_pressed[ BUTTON_MOUSE_LEFT ]++;
    bs.frm_released[ BUTTON_MOUSE_LEFT ] = 0;
  } else {
    bs.frm_pressed[ BUTTON_MOUSE_LEFT ] = 0;
    bs.frm_released[ BUTTON_MOUSE_LEFT ]++;
  }

  const auto& status = _hif_decoder->GetStatus();
  for (const auto& [key, value] : status) {
    switch(value->ev.type){
      case  B8_HIF_EV_TOUCH_START:
      case  B8_HIF_EV_TOUCH_MOVE:
      case  B8_HIF_EV_MOUSE_MOVE:
      case  B8_HIF_EV_MOUSE_HOVER_MOVE:
      case  B8_HIF_EV_MOUSE_DOWN:
      case  B8_HIF_EV_MOUSE_UP:
        _mouse_status.x = value->ev.xp;
        _mouse_status.y = value->ev.yp;
        break;

      case  B8_HIF_EV_TOUCH_CANCEL:
      case  B8_HIF_EV_TOUCH_END:
        _mouse_status.ClearStatus();
        break;

    }
  }
}

void  Pico8::run(){
  _reset();
  _test_fgetset();
  _init();

  _status = RUNNING;

  while(1){
    hif_update();
    _update();
    ++_cnt_update;
    if( has_error() ) break;

    b8PpuCmdSetBuff( &_ppu_cmd , _ppu_cmd_buff , sizeof( _ppu_cmd_buff ) );
    b8PpuClearOT( &_ppu_cmd , &_ot[0], &_ot_prev[0], MAX_OTZ );
    clear_jmp_prev( &_ppu_cmd );
    _during_draw = true;
    _draw();

    {
      bgprint::ExportPpuCmd epc;
      epc._cmd = &_ppu_cmd;
      epc._otz = OTZ_BG_TEXT;
      bgprint::Export(_fp_bgprint, epc);
    }

    if( _init_dprint && _dprint_enabled ){
      bgprint::ExportPpuCmd epc;
      epc._cmd = &_ppu_cmd;
      epc._otz = OTZ_BG_TEXT;
      bgprint::Export(_fp_bgprint_debug, epc);
    }

    test_esc( _fp_sprprint );

    _during_draw = false;
    if( has_error() ) break;
    fflush(_fp_sprprint);
    b8PpuHaltAlloc( &_ppu_cmd );
    b8PpuExec( &_ppu_cmd );
    b8PpuVsyncWait();
  }

  _status = ERROR; 
  fprintf( stderr, "PICO-8 API ERROR : %d\n" , _error );
  fprintf( stdout,
          "An error with code %d (_error) occurred while calling the PICO-8 API.\n"
          "Please refer to the 'enum Error' definitions in pico8.h to identify the error.\n",
          _error);
  while( true ){
    usleep(16*1000);
  }
}

void lsp(u8 bank,const uint8_t* srcimg){
  _ASSERT( bank < MAX_SPR_BANK , "invalid bank" );
  _ASSERT( sprite_sheets[ bank ] == 0 , "sprite_sheets is already used" );

  b8PpuCmdSetBuff( &_ppu_cmd , _ppu_cmd_buff , sizeof( _ppu_cmd_buff ) );

  {
    b8PpuLoadimg* pp = b8PpuLoadimgAlloc( &_ppu_cmd );
    pp->cpuaddr = srcimg;

    // src
    pp->srcxtile = 0;
    pp->srcytile = 0;
    pp->srcwtile = 128 >>3;

    // dst
    pp->dstxtile = (bank&3)<<4;
    pp->dstytile = bank>>2;
    pp->trnwtile = 128 >>3;
    pp->trnhtile = 128 >>3;
  }

  {
    b8PpuFlush* pp = b8PpuFlushAlloc( &_ppu_cmd );
    pp->img = 1;
  }

  sprite_sheets[ bank ] = srcimg;

  b8PpuHaltAlloc( &_ppu_cmd );
  b8PpuExec( &_ppu_cmd );
  b8PpuVsyncWait();
}

void  cls( Color color ){
  MUST( _during_draw , NOT_DURING_DRAWING );

  scursor();

  {
    b8PpuScissor* pp = b8PpuScissorAllocZPB( &_ppu_cmd, OTZ_CLEAR );
    pp->x = pp->y = 0;
    pp->w = _reso_w;
    pp->h = _reso_h;
  }

  {
    b8PpuRect* pp = b8PpuRectAllocZPB( &_ppu_cmd , OTZ_CLEAR );
    pp->pal = (color == CURRENT) ? _color : color;
    pp->x = 0;
    pp->y = 0;
    pp->w = _reso_w;
    pp->h = _reso_h;
  }

  {
    b8PpuScissor* pp = b8PpuScissorAllocZPB( &_ppu_cmd, OTZ_CLEAR );
    pp->x = _clip_cur.x;
    pp->y = _clip_cur.y;
    pp->w = _clip_cur.w;
    pp->h = _clip_cur.h;
  }
}

void rectfill(fx8 x0, fx8 y0, fx8 x1, fx8 y1, Color color) {
  MUST(_during_draw, NOT_DURING_DRAWING);

  x0 -= _camera_cur.x;
  x1 -= _camera_cur.x;
  y0 -= _camera_cur.y;
  y1 -= _camera_cur.y;

  if(x0 > x1) std::swap(x0, x1);
  if(y0 > y1) std::swap(y0, y1);

  const Rect rc(x0, y0, x1 - x0, y1 - y0);
  if (false == _is_colliding(rc, _clip_cur)) return;

  b8PpuRect* pp = b8PpuRectAllocZPB(&_ppu_cmd, _otz);
  pp->pal = (color == CURRENT) ? _color : color;
  pp->x = x0;
  pp->y = y0;
  pp->w = x1 - x0;
  pp->h = y1 - y0;
}

void  pset(fx8 x0, fx8 y0, Color col ){
  rectfill(x0, y0, x0+fx8(1), y0+fx8(1), col );
}

void  rect(fx8 x0, fx8 y0, fx8 x1, fx8 y1, Color color ){
  rectfill(x0, y0, x1,   y0+1, color );
  rectfill(x0, y1, x1+1, y1+1, color );
  rectfill(x0, y0, x0+1, y1,   color );
  rectfill(x1, y0, x1+1, y1,   color );
}

void  line(fx8 x0,fx8 y0,fx8 x1,fx8 y1, Color color ){
  line( Line( Vec(x0,y0), Vec(x1,y1) ) , color );
}

void  line(const Line& ln, Color color ){
  MUST( _during_draw , NOT_DURING_DRAWING );

  Line lln(ln);
  lln.pos0 -= _camera_cur;
  lln.pos1 -= _camera_cur;

  if( false == _is_colliding( lln, _clip_cur ) ) return;

  b8PpuLine* pp = b8PpuLineAllocZPB( &_ppu_cmd , _otz );
  pp->pal = (color == CURRENT) ? _color : color;
  pp->width = 2;
  pp->x0 = lln.pos0.x;
  pp->y0 = lln.pos0.y;
  pp->x1 = lln.pos1.x;
  pp->y1 = lln.pos1.y;
}

void  poly(const Poly& pol , Color color ){
  MUST( _during_draw , NOT_DURING_DRAWING );

  Poly lpol(pol);
  lpol.pos0 -= _camera_cur;
  lpol.pos1 -= _camera_cur;
  lpol.pos2 -= _camera_cur;

  if( false == _is_colliding( lpol , _clip_cur ) ) return;

  b8PpuPoly* pp = b8PpuPolyAllocZPB( &_ppu_cmd, _otz);
  pp->pal = (color == CURRENT) ? _color : color;
  pp->x0 = lpol.pos0.x;
  pp->y0 = lpol.pos0.y;
  pp->x1 = lpol.pos1.x;
  pp->y1 = lpol.pos1.y;
  pp->x2 = lpol.pos2.x;
  pp->y2 = lpol.pos2.y;
}

void  poly(fx8 x0,fx8 y0,fx8 x1,fx8 y1,fx8 x2,fx8 y2, Color color ){
  poly( Poly(x0,y0,x1,y1,x2,y2),color);
}

void  spr(int n, fx8 x, fx8 y, u8 w, u8 h, bool flip_x, bool flip_y,u8 selpal){
  MUST( _during_draw, NOT_DURING_DRAWING );
  MUST( selpal < 16 , INVALID_PARAM );
  MUST( n < 256 , INVALID_PARAM );
  if( 0 == w || 0 == h )  return;

  const Rect rc(x - _camera_cur.x, y - _camera_cur.y, w<<3,h<<3);
  if( false == _is_colliding(rc, _clip_cur ) )  return;

  b8PpuSprite* pp = b8PpuSpriteAllocZPB( &_ppu_cmd , _otz );
  pp->pal = selpal;
  pp->x = rc.x;
  pp->y = rc.y;
  pp->srcwtile = w;
  pp->srchtile = h;
  pp->vfp = flip_y ? 1:0;
  pp->hfp = flip_x ? 1:0;
  pp->srcxtile = (n&0xf);
  pp->srcytile = (n>>4);
}

void sprb(u8 bank , int n, fx8 x , fx8 y , u8 w , u8 h , bool flip_x , bool flip_y , u8 selpal ){
  MUST( _during_draw, NOT_DURING_DRAWING );
  MUST( selpal < 16 , INVALID_PARAM );
  MUST( n < 256  , INVALID_PARAM );
  MUST( bank < 16, INVALID_PARAM );
  if( 0 == w || 0 == h )  return;

  const Rect rc(x - _camera_cur.x, y - _camera_cur.y, w<<3,h<<3);
  if( false == _is_colliding(rc, _clip_cur ) )  return;

  b8PpuSprite* pp = b8PpuSpriteAllocZPB( &_ppu_cmd , _otz );
  pp->pal = selpal;
  pp->x = rc.x;
  pp->y = rc.y;
  pp->srcwtile = w;
  pp->srchtile = h;
  pp->vfp = flip_y ? 1:0;
  pp->hfp = flip_x ? 1:0;

  const u8 lx = n&0xf;
  const u8 ly = n>>4;

  const u8 bx = ((bank&3)<<4);
  const u8 by = ((bank>>2)<<4);

  pp->srcxtile = bx + lx;
  pp->srcytile = by + ly;
}

void setpal(int palsel, const std::array<unsigned char, 16>& pidx ){
  MUST( _during_draw , NOT_DURING_DRAWING );

  b8PpuSetpal* pp = b8PpuSetpalAllocZPB( &_ppu_cmd, _otz, 1 );
  pp->palsel = palsel;

  pp->pidx0 = pidx[0];
  pp->pidx1 = pidx[1];
  pp->pidx2 = pidx[2];
  pp->pidx3 = pidx[3];
  pp->pidx4 = pidx[4];
  pp->pidx5 = pidx[5];
  pp->pidx6 = pidx[6];
  pp->pidx7 = pidx[7];

  pp->pidx8 = pidx[8];
  pp->pidx9 = pidx[9];
  pp->pidx10= pidx[10];
  pp->pidx11= pidx[11];
  pp->pidx12= pidx[12];
  pp->pidx13= pidx[13];
  pp->pidx14= pidx[14];
  pp->pidx15= pidx[15];
}

void pal( Color c0 , Color c1 , u8 palsel ){
  MUST( _during_draw , NOT_DURING_DRAWING );
  MUST( c0 < 16 && c1 < 16 && palsel < 16, INVALID_PARAM ); 

  b8PpuSetpal* pp = b8PpuSetpalAllocZPB( &_ppu_cmd, _otz, 1 );
  pp->palsel = palsel;
  pp->wmask = 1<<c0;

  switch( c0 ){
    case  0: pp->pidx0 = c1; break;
    case  1: pp->pidx1 = c1; break;
    case  2: pp->pidx2 = c1; break;
    case  3: pp->pidx3 = c1; break;
    case  4: pp->pidx4 = c1; break;
    case  5: pp->pidx5 = c1; break;
    case  6: pp->pidx6 = c1; break;
    case  7: pp->pidx7 = c1; break;
    case  8: pp->pidx8 = c1; break;
    case  9: pp->pidx9 = c1; break;
    case 10: pp->pidx10= c1; break;
    case 11: pp->pidx11= c1; break;
    case 12: pp->pidx12= c1; break;
    case 13: pp->pidx13= c1; break;
    case 14: pp->pidx14= c1; break;
    case 15: pp->pidx15= c1; break;
    default:  break;
  }
}

Color color(Color color ){
  const Color prev = _color;
  _color = color;
  return prev;
}

const Vec& camera( fx8 camera_x , fx8 camera_y ){
  _camera_prev = _camera_cur;
  _camera_cur.x = camera_x; 
  _camera_cur.y = camera_y;
  return _camera_prev;
}

int   setz(int otz ){
  int save_otz = otz;
  _otz = (otz < 0) ? 0 : (otz > maxz() ? maxz() : otz);
  return save_otz;
}

int   getz(){
  return _otz;
}

const Rect& clip(const Rect& rc ){
  _clip_prev = _clip_cur;

  b8PpuScissor* pp = b8PpuScissorAllocZPB( &_ppu_cmd, _otz);
  pp->x = rc.x;
  pp->y = rc.y;
  pp->w = rc.w;
  pp->h = rc.h;

  _clip_cur = rc;
  return _clip_prev;
}

const Rect& clip(fx8 x, fx8 y, fx8 w, fx8 h){
  return clip( Rect(x,y,w,h) );
}

const Rect& clip(){
  const Rect rc_whole(0,0,_reso_w,_reso_h);
  return clip( rc_whole );
}

int maxz(){
  return  MAX_OTZ-1;
}

#define AFR (4096<<18)

static const u32 vangle_table[] = {
  AFR / 16, AFR / 17, AFR / 18, AFR / 19, 
  AFR / 20, AFR / 21, AFR / 22, AFR / 23, 
  AFR / 24, AFR / 25, AFR / 26, AFR / 27, 
  AFR / 28, AFR / 29, AFR / 30, AFR / 31, 
  AFR / 32, AFR / 33, AFR / 34, AFR / 35, 
  AFR / 36, AFR / 37, AFR / 38, AFR / 39, 
  AFR / 40, AFR / 41, AFR / 42, AFR / 43, 
  AFR / 44, AFR / 45, AFR / 46, AFR / 47, 
  AFR / 48, AFR / 49, AFR / 50
};

static  void  _circ_r1(fx8 x,fx8 y,Color col){
  x -= 1;
  y -= 1;
  const fx8 one(1);
  pset(x-one, y     ,col);
  pset(x+one, y     ,col);
  pset(x,     y-one ,col);
  pset(x,     y+one ,col);
}

static  void  _calc_segments_vangle( fx8 r , int* segments, u32* vangle ){
  const fx8 circumference = fx8(6) * r + (r >> 2);  // 6*r + r/4 ≈ 6.28*r
  *segments = std::min(50, std::max(16, circumference>>3 ) );
  *vangle = vangle_table[ *segments - 16 ];
}

static bool _is_colliding(const Line& line, const Rect& rc) {
  if (line.pos0.x < rc.x && line.pos1.x < rc.x) return false;

  const fx8 right = rc.x + rc.w;
  if (line.pos0.x > right && line.pos1.x > right) return false;

  if (line.pos0.y < rc.y && line.pos1.y < rc.y) return false;

  const fx8 bottom = rc.y + rc.h;
  if (line.pos0.y > bottom && line.pos1.y > bottom) return false;

  return true;
}

static  bool  _is_colliding( const Poly& pol , const Rect& rc ){
  if( pol.pos0.x < rc.x && pol.pos1.x < rc.x && pol.pos2.x < rc.x )     return false;

  const fx8 right = rc.x + rc.w;
  if( pol.pos0.x > right && pol.pos1.x > right && pol.pos2.x > right )  return false;

  if( pol.pos0.y < rc.y && pol.pos1.y < rc.y && pol.pos2.y < rc.y )     return false;

  const fx8 bottom = rc.y + rc.h;
  if( pol.pos0.y > bottom && pol.pos1.y > bottom && pol.pos2.y > bottom ) return false;

  return true;
}

static  bool  _is_colliding( const Vec& pos , const Rect& rc ){
  if( pos.x < rc.x )          return  false;  
  if( pos.x > rc.x + rc.w )   return  false;  
  if( pos.y < rc.y )          return  false;  
  if( pos.y > rc.y + rc.h )   return  false;  
  return  true;
}

static bool _is_colliding(const Rect& rc1, const Rect& rc2) {
  if (rc1.x + rc1.w < rc2.x) return false;
  if (rc1.x > rc2.x + rc2.w) return false;
  if (rc1.y + rc1.h < rc2.y) return false;
  if (rc1.y > rc2.y + rc2.h) return false;

  // If none of the above conditions are met, the rectangles collide
  return true;
}

void circ(fx8 x, fx8 y, fx8 r, Color col) {
  MUST(_during_draw, NOT_DURING_DRAWING);
  if( r <= 0 )  return;
  if( r <= 1 ){
    Vec cpos(x,y);
    if( false == _is_colliding( cpos - _camera_cur , _clip_cur ) ) return;
    _circ_r1(x,y,col);
    return;
  }

  const Vec   center(x - _camera_cur.x, y - _camera_cur.y);
  const Rect  rc( center.x - r, center.y - r, r * 2, r * 2 );
  if( false == _is_colliding(rc,_clip_cur) )  return;

  CameraStack cstk;
  int segments;
  u32 vangle;
  _calc_segments_vangle( r , &segments, &vangle );

  _camera_cur.set();
  Line ln;
  ln.pos0.set( center.x + r , center.y );
  const Vec start( ln.pos0 );
  u32 angle = vangle;

  const Color color = (col == CURRENT) ? _color : col;
  for (int ii = 0; ii < segments; ++ii, angle += vangle , ln.pos0 = ln.pos1 ) {
    if( ii < segments-1 ){
      ln.pos1.x = center.x + rad_cos_12(r, angle>>18);
      ln.pos1.y = center.y + rad_sin_12(r, angle>>18);
    } else {
      ln.pos1 = start;
    }
    if( ! _is_colliding( ln , _clip_cur ) )  continue;
    line(ln, color );
  }
}

void circfill(fx8 x, fx8 y, fx8 r , Color col ){
  MUST(_during_draw, NOT_DURING_DRAWING);
  if( r <= 0 )  return;
  if( r <= 1 ){
    Vec cpos(x,y);
    if( false == _is_colliding( cpos - _camera_cur , _clip_cur ) ) return;
    _circ_r1(x,y,col);
    pset(x,y,col);
    return;
  }

  const Vec   center(x - _camera_cur.x, y - _camera_cur.y);
  const Rect  rc( center.x - r, center.y - r, r * 2, r * 2 );
  if( false == _is_colliding(rc,_clip_cur) )  return;

  CameraStack cstk;
  int segments;
  u32 vangle;
  _calc_segments_vangle( r , &segments, &vangle );

  _camera_cur.set();
  Line ln;
  ln.pos0.set( center.x + r , center.y );
  const Vec start( ln.pos0 );
  u32 angle = vangle;

  col = (col == CURRENT) ? _color : col;
  Poly pol;
  for (int ii = 0; ii < segments; ++ii, angle += vangle , ln.pos0 = ln.pos1 ) {
    if( ii < segments-1 ){
      ln.pos1.x = center.x + rad_cos_12(r, angle>>18);
      ln.pos1.y = center.y + rad_sin_12(r, angle>>18);
    } else {
      ln.pos1 = start;
    }

    pol.pos0 = ln.pos0;
    pol.pos1 = ln.pos1;
    pol.pos2 = center;

    if( !_is_colliding( pol , _clip_cur ) ) continue;
    poly(pol,col);
  }
}

static  void  get_scursor_info( SprCursor& dest ){
  sprprint::Info info;
  if( sprprint::GetInfo(_fp_sprprint,info) < 0 ) return;

  dest.x = info._xpix_locate;
  dest.y = info._ypix_locate;
  dest.z = info._otz;
  dest.color = static_cast< Color >( info._fg );
}

const SprCursor& scursor(int x,int y,Color color,int z) {
  if( !_fp_sprprint ) return  _spr_cursor_prev; 

  get_scursor_info( _spr_cursor_prev );
  sprprint::Locate(_fp_sprprint,x,y,z);
  if( color != CURRENT ) sprprint::Color( _fp_sprprint , static_cast< b8PpuColor >( color ) );
  return  _spr_cursor_prev; 
}

void sprint(std::string_view format, ...){
  va_list args;
  va_start(args, format);
  vfprintf(_fp_sprprint, format.data(), args);
  va_end(args);
}

void sprint(int x, int y, Color color, std::string_view format, ...){
  scursor(x,y,color);

  va_list args;
  va_start(args, format);
  vfprintf(_fp_sprprint, format.data(), args);
  va_end(args);
}

static  void  get_bgcursor_info( BgCursor& dest ){
  bgprint::Info info;
  if( bgprint::GetInfo(_fp_bgprint,info) < 0 ) return;

  dest.x = info._x_locate;
  dest.y = info._y_locate;
  dest.pal = static_cast< BgPal >( info._pal );
}

const BgCursor& cursor(int x, int y, BgPal pal ){
  if( !_fp_bgprint ) return  _bg_cursor_prev;

  get_bgcursor_info( _bg_cursor_prev );
  bgprint::Locate(_fp_bgprint,x,y);
  if( pal != BG_PAL_CURRENT ) bgprint::Pal( _fp_bgprint , static_cast<u8>( pal ) );
  return  _bg_cursor_prev; 
}

void  dprintenable(bool enable){
  _dprint_enabled = enable;
}

void dprint(std::string_view format, ...){
  if( !_init_dprint ){
    bgprint::Context ctx;
    ctx._scroll = true;
    _fp_bgprint_debug = bgprint::Open(
      bgprint::CH7,
      nullptr, 256,
      ctx
    );
    _ASSERT( _fp_bgprint_debug , "failed bgprint::Open()" );
    _init_dprint = true;
  }
  va_list args;
  va_start(args, format);
  vfprintf(_fp_bgprint_debug, format.data(), args);
  va_end(args);
}

void print(std::string_view format, ...){
  va_list args;
  va_start(args, format);
  vfprintf(_fp_bgprint, format.data(), args);
  va_end(args);
}

void print(int x, int y, BgPal pal, std::string_view format, ...){
  cursor(x,y,pal);

  va_list args;
  va_start(args, format);
  vfprintf(_fp_bgprint, format.data(), args);
  va_end(args);
}

void  fset(u8 sprite_index,u8 flag_index,u8 value,u8 sprite_pattern_bank ){
  MUST( sprite_pattern_bank < SPRITE_PATTERN_BANK_NUM, INVALID_PARAM );
  u8* pflag = &_sprite_flags[ sprite_pattern_bank ][ sprite_index ];
  if( flag_index == 0xff ){
    *pflag = value;
  } else {
    MUST( flag_index < 8 , INVALID_PARAM );
    MUST( value == 0 || value == 1, INVALID_PARAM );
    if (value == 1) {
      *pflag |= (1 << flag_index);
    } else {
      *pflag &= ~(1 << flag_index);
    }
  }
}

u8  fget(u8 sprite_index,u8 flag_index,u8 sprite_pattern_bank ){
  MUST_RETURN( sprite_pattern_bank < SPRITE_PATTERN_BANK_NUM, INVALID_PARAM , 0 );
  u8* pflag = &_sprite_flags[ sprite_pattern_bank ][ sprite_index ];
  if( flag_index == 0xff ){
    return  *pflag;
  } else {
    MUST_RETURN( flag_index < 8 , INVALID_PARAM , 0 );
    return  (*pflag & (1<<flag_index)) != 0 ? 1:0;
  }
}

void  mapsetup(BgTiles wtile, BgTiles htile, std::optional<BgTilesPtr> tiles , u8 uwrap, u8 vwrap, BgIndex index ){
  MUST(index < BG_MAX, INVALID_PARAM);
  BgConfig& cfg = _bg_config[ index ];
  if( !tiles.has_value() ) {
    // Memory allocation for tiles (will cause immediate abort on failure; no exception handling)
    cfg.tiles = std::make_shared<std::vector<b8PpuBgTile>>(wtile * htile , b8PpuBgTile{} );
  } else {
    cfg.tiles = *tiles;
  }
  cfg.wtile = wtile;
  cfg.htile = htile;
  cfg.ppu_wtile = std::countr_zero( static_cast< uint32_t >( wtile ) );
  cfg.ppu_htile = std::countr_zero( static_cast< uint32_t >( htile ) );
  cfg.uwrap = uwrap;
  cfg.vwrap = vwrap;
  cfg.ready = true;
}

void  map(s16 upix,s16 vpix, BgIndex index ){
  MUST( _during_draw , NOT_DURING_DRAWING );
  const BgConfig& cfg = _bg_config[ index ];
  MUST( cfg.ready , NOT_INITIALIZED );

  b8PpuBg* pp = b8PpuBgAllocZPB( &_ppu_cmd, _otz );
  pp->cpuaddr = cfg.tiles->data();
  pp->upix = upix;
  pp->vpix = vpix;
  pp->wtile = cfg.ppu_wtile;
  pp->htile = cfg.ppu_htile;
  pp->uwrap = cfg.uwrap;
  pp->vwrap = cfg.vwrap;
}

static  const b8PpuBgTile zerotile = {};
b8PpuBgTile mgett(u32 x,u32 y,BgIndex index){
  const BgConfig& cfg = _bg_config[ index ];
  MUST_RETURN( cfg.ready , NOT_INITIALIZED , zerotile );

  if( x >= cfg.wtile )  return zerotile;
  if( y >= cfg.htile )  return zerotile;
  return  cfg.tiles->at( cfg.wtile * y + x );
}

u16 mget(u32 x,u32 y,BgIndex index ){
  b8PpuBgTile tile = mgett(x,y,index);
  return  (tile.YTILE<<4) + tile.XTILE;
}

void  msett(u32 x,u32 y,b8PpuBgTile tile,BgIndex index){
  const BgConfig& cfg = _bg_config[ index ];
  MUST( cfg.ready , NOT_INITIALIZED );

  if( x >= cfg.wtile )  return;
  if( y >= cfg.htile )  return;
  cfg.tiles->at( cfg.wtile * y + x ) = tile;
}

void  mset(u32 x,u32 y,u8 v,u8 bank,BgIndex index,uint8_t pal ){
  MUST( bank < 16 , INVALID_PARAM );
  MUST( v < 256   , INVALID_PARAM );

  b8PpuBgTile tile;
  tile.XTILE = ((bank &3)<<4) + (v & 0xf);
  tile.YTILE = ((bank>>2)<<4) + (v>>4);
  tile.PAL = pal;
  pico8::msett(x,y,tile,index);
}

void  mcls( b8PpuBgTile tile , BgIndex index){
  const BgConfig& cfg = _bg_config[ index ];
  MUST( cfg.ready , NOT_INITIALIZED );
  fill(cfg.tiles->begin(), cfg.tiles->end(), tile);
}

u32 btn( Button button , u8 player ){
  if( player >= 1 ) return 0;

  const uint32_t pad0 = B8_HIF_PAD(0);

  if( button == BUTTON_ANY ){
    u32 retval = 0x0000;

    if( (pad0 & B8_HIF_PAD_STATUS_LEFT) )   retval |= 1<<BUTTON_LEFT;
    if( (pad0 & B8_HIF_PAD_STATUS_RIGHT))   retval |= 1<<BUTTON_RIGHT;
    if( (pad0 & B8_HIF_PAD_STATUS_UP)   )   retval |= 1<<BUTTON_UP;
    if( (pad0 & B8_HIF_PAD_STATUS_DOWN) )   retval |= 1<<BUTTON_DOWN;
    if( (pad0 & B8_HIF_PAD_STATUS_BTN_Z ) ) retval |= 1<<BUTTON_O;
    if( (pad0 & B8_HIF_PAD_STATUS_BTN_X)  ) retval |= 1<<BUTTON_X;
    if( _mouse_status.btn_status & LEFT )   retval |= 1<<BUTTON_MOUSE_LEFT;

    return retval;
  }

  switch( button ){
    case  BUTTON_LEFT:  return  pad0 & B8_HIF_PAD_STATUS_LEFT   ? true : false;
    case  BUTTON_RIGHT: return  pad0 & B8_HIF_PAD_STATUS_RIGHT  ? true : false;
    case  BUTTON_UP:    return  pad0 & B8_HIF_PAD_STATUS_UP     ? true : false;
    case  BUTTON_DOWN:  return  pad0 & B8_HIF_PAD_STATUS_DOWN   ? true : false;
    case  BUTTON_O:     return  pad0 & B8_HIF_PAD_STATUS_BTN_Z  ? true : false;
    case  BUTTON_X:     return  pad0 & B8_HIF_PAD_STATUS_BTN_X  ? true : false;
    default:break;
  }

  return false;
}

static  bool  btnpany( u8 player ){
  if( btnp( BUTTON_LEFT , player ) ) return true;
  if( btnp( BUTTON_RIGHT, player ) ) return true;
  if( btnp( BUTTON_UP,    player ) ) return true;
  if( btnp( BUTTON_DOWN,  player ) ) return true;
  if( btnp( BUTTON_O,     player ) ) return true;
  if( btnp( BUTTON_X,     player ) ) return true;
  if( btnp( BUTTON_MOUSE_LEFT , player ) ) return true;
  return false;
}

bool  btnp( Button button , u8 player ){
  if( player >= 1 ) return false;

  if( button == BUTTON_ANY )  return btnpany( player );

  const ButtonStatus& bs = _button_status[ 0 ];
  const u32 frm = bs.frm_pressed[ button ];
  if( 1 == frm )  return true;
  if( frm < 15)   return false;
  if( !(frm & 3) )  return  true;
  return  false;
}

u32  btnr(Button button, u8 player){
  if( player >= 1 ) return false;

  const ButtonStatus& bs = _button_status[ 0 ];
  return  bs.frm_released[ button ];
}

s32 stat( int index ){
  switch( index ){
    case  32: return mousex();
    case  33: return mousey();
    case  34: return mousestatus();

    case 1000:{
      return  _ppu_cmd.sp - _ppu_cmd.buff;
    }break;
  }
  return 0;
}

fx8 mousex(){
  fx8 ret;
  ret.set_raw_value( _mouse_status.x << 4 );
  return ret;
}

fx8 mousey(){
  fx8 ret;
  ret.set_raw_value( _mouse_status.y << 4 );
  return ret;
}

u32 mousestatus(){
  return _mouse_status.btn_status;
}

fx8 cos( fx8 rad ){
  return  fpm::cos( rad );
}

fx8 sin( fx8 rad ){
  return  fpm::sin( rad );
}

fx8 atan2(fx8 y,fx8 x){
  return  fpm::atan2(y,x);
}

fx8 abs(fx8 x){
  return  _abs( x );
}

fx8 flr(fx8 x){
  return  fpm::floor(x);
}

fx8 cel(fx8 x){
  return  fpm::ceil(x);
}

fx8 max(fx8 x,fx8 y){
  return  _max(x,y);
}

fx8 min(fx8 x,fx8 y){
  return  _min(x,y);
}

fx8 mid( fx8 first, fx8 second, fx8 third ){
  return max(
    min(first, second),
    min(
      max(first, second),
      third
    )
  );
}

fx8 sgn(fx8 x) {
  return (x > fx8(0)) ? fx8(1) : (x < fx8(0)) ? fx8(-1) : fx8(1);
}

fx8 sqrt(fx8 x) {
  if( x < 0)  return 0;
  return fpm::sqrt(x);
}

u32 rndu(){
  return  xors.next();
}

fx8 rnd(fx8 x){
  int32_t rv =  x.raw_value();
  if( rv <= 1 )  return  fx8(0);
  fx8 result;
  result.set_raw_value( qmod(xors.next() , rv) );
  return result;
}

fx8 rndi(fx8 x) {
  if (x <= fx8(1)) return fx8(0);
  const int32_t max_value = static_cast<int>(x);
  const int32_t random_int = xors.next_range(0, max_value - 1);
  return fx8(random_int);
}

fx8 rndf(fx8 x0, fx8 x1){
  if (x0 > x1) {
    const fx8 temp(x0);
    x0 = x1;
    x1 = temp;
  }

  const int32_t min_raw = x0.raw_value();
  const int32_t max_raw = x1.raw_value();
  const int32_t random_raw = xors.next_range(min_raw, max_raw);

  fx8 result;
  result.set_raw_value(random_raw);
  return result;
}

void  srand(u32 seed ){
  xors.state = seed;
}

fx8 resw(){
  return  _reso_w;
}

fx8 resh(){
  return  _reso_h;
}

Color sget(u8 x, u8 y , u8 bank ){
  MUST_RETURN( bank < MAX_SPR_BANK , INVALID_PARAM , BLACK );
  MUST_RETURN( x < 128 && y < 128  , INVALID_PARAM , BLACK );

  if( sprite_sheets[ bank ] == 0 )  return  BLACK;

  const u8* pix = sprite_sheets[ bank ];
  const u8 dot = *( pix + ((y<<6)+(x>>1)) );
  return x&1 ? static_cast< Color >( dot&0xf ) : static_cast< Color >( dot>>4 );
}
struct LargeStruct {
  int data[256];
  int x=100;
};

// TODO: remove tester
class Tester{
  public:
    Tester(){
      for ( int i = 0 ; i < 100 ; ++i) {
        static  const int tbl[] = {10, 20, 30, 40, 50};
        int val = rndt(tbl);
        WATCH( val );
      }

      for ( int i = 0 ; i < 100 ; ++i) {
        static  const LargeStruct tbl[] ={
          LargeStruct{.x=1},
          LargeStruct{.x=2},
          LargeStruct{.x=3}
        };
        const LargeStruct& ls = rndt(tbl);
        WATCH( ls.x );
      }

      assert(mid(fx8(1), fx8(2), fx8(3)) == fx8(2));
      assert(mid(fx8(3), fx8(2), fx8(1)) == fx8(2));
      assert(mid(fx8(1), fx8(3), fx8(2)) == fx8(2));

      assert(mid(fx8(2), fx8(2), fx8(3)) == fx8(2));
      assert(mid(fx8(3), fx8(3), fx8(2)) == fx8(3));
      assert(mid(fx8(1), fx8(2), fx8(2)) == fx8(2));

      assert(mid(fx8(5), fx8(5), fx8(5)) == fx8(5));

      assert(mid(fx8(-1), fx8(0), fx8(1)) == fx8(0));
      assert(mid(fx8(-3), fx8(-2), fx8(-1)) == fx8(-2));
      assert(mid(fx8(-1), fx8(-2), fx8(0)) == fx8(-1));

      assert(mid(fx8(1.1), fx8(2.2), fx8(3.3)) == fx8(2.2));
      assert(mid(fx8(3.3), fx8(1.1), fx8(2.2)) == fx8(2.2));

      assert(sgn(fx8(5)) == fx8(1));
      assert(sgn(fx8(0.5)) == fx8(1));

      assert(sgn(fx8(0)) == fx8(1));

      assert(sgn(fx8(-3)) == fx8(-1));
      assert(sgn(fx8(-0.7)) == fx8(-1));

      assert(sqrt(fx8(4)) == fx8(2));
      assert(sqrt(fx8(9)) == fx8(3));
      assert(sqrt(fx8(2.25)) == fx8(1.50));

      assert(sqrt(fx8(0)) == fx8(0));

      assert(sqrt(fx8(-1)) == fx8(0));
      assert(sqrt(fx8(-2.25)) == fx8(0));

      assert(cel(fx8(2.1)) == fx8(3));
      assert(cel(fx8(2.9)) == fx8(3));
      assert(cel(fx8(0.5)) == fx8(1));

      assert(cel(fx8(-2.1)) == fx8(-2));
      assert(cel(fx8(-2.9)) == fx8(-2));
      assert(cel(fx8(-0.5)) == fx8(0));

      assert(cel(fx8(3)) == fx8(3));
      assert(cel(fx8(-4)) == fx8(-4));
      assert(cel(fx8(0)) == fx8(0));

      srand(42);

      fx8 max_val = fx8(10);
      for (int i = 0; i < 100; ++i) {
          fx8 result = rndi(max_val);
          assert(result >= fx8(0));
          assert(result < max_val);
      }

      fx8 min_val = fx8(1.5);
      max_val = fx8(5.75);
      for (int i = 0; i < 100; ++i) {
          fx8 result = rndf(min_val, max_val);
          assert(result >= min_val);
          assert(result <= max_val);
      }

      srand(42);
      fx8 result1 = rndi(max_val);
      fx8 result2 = rndi(max_val);

      srand(42);
      fx8 expected1 = rndi(max_val);
      fx8 expected2 = rndi(max_val);

      assert(result1 == expected1);
      assert(result2 == expected2);
    }
};
//static  Tester  _tester;

} // namespace pico8