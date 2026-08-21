// Sound helper audition tool: steps through every SFX preset and plays a
// 4-track MML piece, so the presets can be listened to and tuned by ear.
//
// TEXT LAYER CHOICE: everything here is drawn on the BACKGROUND text layer
// (cursor()/print(), TILE units, 16x30 tiles on a 128x240 screen) rather than
// the sprite layer (scursor()/sprint(), PIXEL units). Background text is
// written into a tilemap once and then costs a single PPU command per frame no
// matter how much of it there is, whereas sprite text re-issues one sprite per
// glyph every frame. A 22-row list is ~180 glyphs, which is real money on a
// 4 MHz budget -- see the FPS/WORK readout this app prints.
//
// The list is only re-written when something actually changes; the stats line
// is the only thing touched on a normal frame.
#include <pico8.h>
#include <sound.h>
#include <b8/dwt.h>
#include <b8/sys.h>

using namespace pico8;

// Same order as enum SndSfx.
static const char* SFX_NAME[ SFX_COUNT ] = {
  "JUMP", "LAND", "STEP", "SWIPE", "SHOOT", "CHARGE",
  "HIT", "BOUNCE", "BREAK", "BLOCK", "EXPLODE", "DAMAGE",
  "COIN", "HEAL", "POWERUP", "LEVELUP", "UNLOCK",
  "SELECT", "CONFIRM", "CANCEL", "DENY", "BLIP",
  "ALARM", "CLEAR", "GAMEOVER",
  "SPLASH", "WARP",
};

static const char* BGM_MELODY = "t130 o5 l8 v11 q6 @0 [ c e g e | d f a f ]2 [ >c< b g e | g e c4 ]";
static const char* BGM_BASS   = "t130 o3 l4 v11 q5 @1 [ c c g g | f f g g ]2";
static const char* BGM_HARM   = "t130 o4 l8 v7  q4 @2 [ r e r e | r f r f ]2";
static const char* BGM_DRUM   = "@n t130 o5 l8 v9 [ c r > c < c r c > c4 < ]4";

// Background text grid: 16 tiles across, 30 visible down.
static const int ROW_TITLE = 0;
static const int ROW_BGM   = 1;
static const int ROW_STATS = 2;
static const int LIST_TOP  = 4;
static const int ROWS      = 22;      // visible slice of the preset list
static const int ROW_HELP  = LIST_TOP + ROWS + 1;

static const int PAL_HEAD  = 1;       // WHITE -> YELLOW
static const int PAL_SEL   = 2;       // WHITE -> LIGHT_PEACH
static const int PAL_HELP  = 3;       // WHITE -> LAVENDER

static const int STAT_WIN  = 30;      // frames averaged per stats update

class App : public Pico8 {
public:
  int  cur   = 0;
  int  top   = 0;
  bool dirty = true;
  bool first_draw = true;

  // --- cycle accounting (see the note at the top of the file) ---
  u32 clk        = 4000000;
  u32 budget     = 66666;             // cycles available per 60Hz frame
  u32 t_enter    = 0;                 // CYCCNT at the top of this _update()
  u32 t_prev     = 0;                 // ... of the previous one
  u32 sum_work   = 0;                 // cycles spent in _update()+_draw()
  u32 sum_frame  = 0;                 // cycles per whole frame period
  int nsample    = 0;
  int fps        = 0;
  int workpct    = 0;

  void _init() override {
    B8_DWT_CTRL = 1;                  // the cycle counter does not run until this is set
    clk    = b8SysGetCpuClock();
    budget = clk / 60;

    // NOTE: pal() is not called here. It emits a PPU command, so like every
    // drawing call it is only legal inside _draw() -- calling it from _init()
    // raises NOT_DURING_DRAWING and the app dies before the first frame. The
    // palette itself persists once written, so _draw() sets it up exactly once
    // (see first_draw below), the same way pakupaku does it.
    sndBgmPlay( BGM_MELODY, BGM_BASS, BGM_HARM, BGM_DRUM );
    t_prev = B8_DWT_CYCCNT;
  }

  void play(){ sndSfx( (SndSfx)cur ); }

  void scrollToCur(){
    if( cur <  top )               top = cur;
    if( cur >= top + ROWS )        top = cur - ROWS + 1;
    if( top <  0 )                 top = 0;
    if( top >  SFX_COUNT - ROWS )  top = SFX_COUNT - ROWS;
  }

  void _update() override {
    t_enter    = B8_DWT_CYCCNT;
    sum_frame += t_enter - t_prev;
    t_prev     = t_enter;

    if( btnp(BUTTON_UP)   ){ cur = ( cur + SFX_COUNT - 1 ) % SFX_COUNT; scrollToCur(); dirty = true; }
    if( btnp(BUTTON_DOWN) ){ cur = ( cur + 1 ) % SFX_COUNT;             scrollToCur(); dirty = true; }
    if( btnp(BUTTON_O)    ) play();

    if( btnp(BUTTON_MOUSE_LEFT) ){
      // Background text is on the tile grid, so a tap maps to a row by /8.
      const int row = ( (int)mousey() ) / 8 - LIST_TOP;
      if( row >= 0 && row < ROWS && top + row < SFX_COUNT ){
        cur = top + row; dirty = true;
      }
      play();
    }

    if( btnp(BUTTON_X) ){
      if( sndBgmIsPlaying() ) sndBgmStop();
      else sndBgmPlay( BGM_MELODY, BGM_BASS, BGM_HARM, BGM_DRUM );
      dirty = true;
    }
  }

  void drawList(){
    print( "\e[2J" );                 // clear the whole background text plane

    cursor( 1, ROW_TITLE, (BgPal)PAL_HEAD );
    print( "SFX %d/%d", cur + 1, (int)SFX_COUNT );

    cursor( 1, ROW_BGM, (BgPal)PAL_HEAD );
    print( sndBgmIsPlaying() ? "BGM ON " : "BGM OFF" );

    for( int r = 0 ; r < ROWS ; ++r ){
      const int i = top + r;
      if( i >= SFX_COUNT ) break;
      const bool sel = ( i == cur );
      cursor( 1, LIST_TOP + r, sel ? (BgPal)PAL_SEL : BG_PAL_0 );
      print( sel ? ">%s" : " %s", SFX_NAME[i] );
    }

    if( top > 0 )                 { cursor( 13, LIST_TOP,        BG_PAL_0 ); print( "^" ); }
    if( top + ROWS < SFX_COUNT )  { cursor( 13, LIST_TOP+ROWS-1, BG_PAL_0 ); print( "v" ); }

    cursor( 1, ROW_HELP,     (BgPal)PAL_HELP ); print( "UP/DN SELECT" );
    cursor( 1, ROW_HELP + 1, (BgPal)PAL_HELP ); print( "O/TAP PLAY" );
    cursor( 1, ROW_HELP + 2, (BgPal)PAL_HELP ); print( "X     BGM" );
  }

  void _draw() override {
    cls( Color::DARK_BLUE );

    if( first_draw ){
      // Palettes are PPU state and stick once set, so this runs on frame 0 only.
      pal( Color::WHITE, Color::YELLOW,      PAL_HEAD );
      pal( Color::WHITE, Color::LIGHT_PEACH, PAL_SEL  );
      pal( Color::WHITE, Color::LAVENDER,    PAL_HELP );
      first_draw = false;
    }

    if( dirty ){ drawList(); dirty = false; }

    if( ++nsample >= STAT_WIN ){
      // fps  = how many frame periods fit in a second of CPU cycles
      // work = share of one frame's cycle budget spent in _update()+_draw()
      fps     = sum_frame ? (int)( (u32)STAT_WIN * clk / sum_frame ) : 0;
      workpct = (int)( sum_work / (u32)STAT_WIN * 100 / budget );
      cursor( 1, ROW_STATS, (BgPal)PAL_HEAD );
      print( "%dfps W%d%%   ", fps, workpct );
      sum_frame = sum_work = 0;
      nsample   = 0;
    }

    sum_work += B8_DWT_CYCCNT - t_enter;
  }
};

int main(){
  App app;
  app.run();
  return 0;
}
