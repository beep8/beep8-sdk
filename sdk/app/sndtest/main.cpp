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

// 8 short BGM loops, each a 4-track MML piece (melody / bass / harmony / drum;
// harmony and drum are nullptr where a leaner arrangement sounds better). Each
// track's total note length is kept equal (or an even multiple) across the
// active tracks in a pattern, since every track loops back to its own start
// independently (see sound.cpp's track_advance) -- if the totals didn't line up the tracks
// would drift out of phase after a few loops instead of staying in lockstep.
// MINOR / ACTION / AMBIENT / SUSPENSE lean on a fast broken-chord bass
// (arpeggio) rather than block chords, per the brief asking for a few tracks
// with the bass carrying an arpeggio.
struct BgmDef {
  const char* name;
  const char* t0;   // melody
  const char* t1;   // bass
  const char* t2;   // harmony (may be null)
  const char* t3;   // drum, usually @n (may be null)
};

static const BgmDef BGM_DEFS[] = {
  { "MAJOR",
    "t130 o5 l8 v11 q6 @0 [ c e g e | d f a f ]2 [ >c< b g e | g e c4 ]",
    "t130 o3 l4 v11 q5 @1 [ c c g g | f f g g ]2",
    "t130 o4 l8 v7  q4 @2 [ r e r e | r f r f ]2",
    "@n t130 o5 l8 v9 [ c r > c < c r c > c4 < ]4" },

  { "MINOR",
    "t130 o5 l4 v11 q6 @0 [ a b c d | e d c b ]2",
    "t130 o2 l16 v10 q7 @1 [ [a c e a]4 | [e g b e]4 ]2",
    "t130 o4 l1 v6  q8 @2 [ a | e ]2",
    "@n t130 o3 l4 v6 [ c r c r ]4" },

  { "WALTZ",
    "t150 o5 l4 v11 q6 @0 [ c d e | f g a | g f e | d c2 ]2",
    "t150 o2 l4 v10 q6 @1 [ c2. | f2. | g2. | d2. ]2",
    "t150 o4 l4 v6  q7 @2 [ e2. | a2. | b2. | f2. ]2",
    0 },

  { "ACTION",
    "t180 o5 l8 v12 q5 @0 [ a a c a | g g b g ]4",
    "t180 o2 l16 v11 q7 @1 [ [a c e a]4 | [g b d g]4 ]2",
    0,
    "@n t180 o5 l8 v10 [ c r c c | c r c c ]4" },

  { "AMBIENT",
    "t80 o5 l2 v9 q8 @3 [ c r e r | g r > c < r ]2",
    "t80 o2 l8 v9 q6 @1 [ [c e g e]2 | [g b > d < g]2 ]4",
    "t80 o4 l1 v5 q8 @2 [ c | g ]4",
    0 },

  { "BOUNCY",
    "t140 o5 l8 v12 q4 @0 [ c e g e | f a > c < a ]4",
    "t140 o3 l4 v10 q3 @1 [ c c g g ]4",
    0,
    "@n t140 o5 l8 v8 [ c r c r ]8" },

  { "SUSPENSE",
    "t100 o5 l4 v8 q5 @6 [ a r r r | c r r r | e r r r | d r r r ]2",
    "t100 o2 l16 v10 q6 @1 [ [a c e a]4 ]8",
    0,
    "@n t100 o3 l4 v9 [ r r c r | r r r r | r r c r | r r r r ]2" },

  { "FANFARE",
    "t150 o5 l8 v13 q7 @5 [ c e g > c < g e c4 ]4",
    "t150 o3 l4 v12 q6 @1 [ c c g g ]4",
    "t150 o4 l1 v7  q8 @2 [ e | c ]2",
    "@n t150 o5 l8 v9 [ c r > c < c r c > c4 < ]4" },
};
static const int BGM_COUNT = (int)( sizeof(BGM_DEFS) / sizeof(BGM_DEFS[0]) );

// Background text grid: 16 tiles across, 30 visible down.
static const int ROW_TITLE = 0;
static const int ROW_BGM   = 1;
static const int ROW_STATS = 2;
static const int ROW_BGMST = 3;
static const int LIST_TOP  = 4;
static const int ROWS      = 22;      // visible slice of the preset list
static const int ROW_HELP  = LIST_TOP + ROWS + 1;

static const int PAL_HEAD  = 1;       // WHITE -> YELLOW
static const int PAL_SEL   = 2;       // WHITE -> LIGHT_PEACH
static const int PAL_HELP  = 3;       // WHITE -> LAVENDER

static const int STAT_WIN  = 30;      // frames averaged per stats update

class App : public Pico8 {
public:
  int  cur    = 0;
  int  top    = 0;
  int  bgmIdx = 0;
  bool dirty  = true;
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
    playBgm();
    t_prev = B8_DWT_CYCCNT;
  }

  void play(){ sndSfx( (SndSfx)cur ); }

  void playBgm(){
    const BgmDef& b = BGM_DEFS[ bgmIdx ];
    sndBgmPlay( b.t0, b.t1, b.t2, b.t3 );
  }

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
      else playBgm();
      dirty = true;
    }

    if( btnp(BUTTON_LEFT)  ){ bgmIdx = ( bgmIdx + BGM_COUNT - 1 ) % BGM_COUNT; if( sndBgmIsPlaying() ) playBgm(); dirty = true; }
    if( btnp(BUTTON_RIGHT) ){ bgmIdx = ( bgmIdx + 1 )              % BGM_COUNT; if( sndBgmIsPlaying() ) playBgm(); dirty = true; }
  }

  void drawList(){
    print( "\e[2J" );                 // clear the whole background text plane

    cursor( 1, ROW_TITLE, (BgPal)PAL_HEAD );
    print( "SFX %d/%d", cur + 1, (int)SFX_COUNT );

    cursor( 1, ROW_BGM, (BgPal)PAL_HEAD );
    print( "%d/%d %s", bgmIdx + 1, BGM_COUNT, BGM_DEFS[bgmIdx].name );

    cursor( 1, ROW_BGMST, (BgPal)PAL_HEAD );
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
    cursor( 1, ROW_HELP + 2, (BgPal)PAL_HELP ); print( "LR PTN X BGM" );
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
