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
// The list is only re-written when something actually changes, and moving the
// selection re-writes just the two rows that changed rather than the whole
// screen: a full 30-line repaint costs ~66k cycles, a full frame's budget, so
// doing one per keypress visibly drops a frame (it used to be ~137k, two
// frames, before bgprint's cursor path was made cheaper).
#include <pico8.h>
#include <sound.h>
#include <sys/time.h>
#include <b8/syscall.h>

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

// 12 short BGM loops, each a 4-track MML piece (melody / bass / harmony / drum;
// harmony and drum are nullptr where a leaner arrangement sounds better). Each
// track's total note length is kept equal (or an even multiple) across the
// active tracks in a pattern, since every track loops back to its own start
// independently (see sound.cpp's track_advance) -- if the totals didn't line up the tracks
// would drift out of phase after a few loops instead of staying in lockstep.
// MINOR / ACTION / AMBIENT / SUSPENSE lean on a fast broken-chord bass
// (arpeggio) rather than block chords, per the brief asking for a few tracks
// with the bass carrying an arpeggio.
//
// The first eight use nothing but notes, so they are also the control group for
// anyone tuning the sequencer. The last four exist to audition one modulation
// command each -- vibrato, the volume envelope, portamento and sweep, detune --
// against those: see the "Shaping the sound" table in sound.h.
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

  // -- the modulation demos --

  { "VIBRATO",                                  // mp on the lead, k on the pad
    "t110 o5 l2 v11 q8 me1 mp6,45,200 [ e g | a g ]2",
    "t110 o3 l1 v10 q8 me0 [ c | f ]2",
    "t110 o4 l1 v6  q8 me0 k7 [ g | c ]2",
    0 },

  { "PLUCK",                                    // me: hard pluck over a long bass
    "t150 o5 l16 v12 q8 me13 [ c e g > c < g e ]8",
    "t150 o3 l4  v11 q6 me2  [ c c g g ]3",
    "t150 o4 l2  v6  q8 me0  [ e g ]3",
    "@n t150 o5 l8 v9 [ c r c c ]6" },

  { "SLIDE",                                    // mg portamento, ms falling blips
    "t120 o4 l4 v11 q8 me0 mg120 [ c g > c < g ]4",
    "t120 o2 l2 v10 q7 me3 [ c f ]4",
    "t120 o5 l8 v7  q3 me8 ms-900 [ c r c r c r c r ]4",
    "@n t120 o4 l4 v8 [ c r c r ]4" },

  { "THICK",                                    // the same line twice, k cents apart
    "t100 o5 l2 v10 q8 me0 k-6 mv5,35 [ a > c < | b a ]2",
    "t100 o5 l2 v10 q8 me0 k6       [ a > c < | b a ]2",
    "t100 o2 l1 v11 q8 me1 [ a | e ]2",
    0 },
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

static const u64 NS_PER_SEC   = 1000000000ULL;
static const u64 NS_PER_FRAME = NS_PER_SEC / 60;

// Wall clock in nanoseconds.
//
// NOTE: B8_DWT_CYCCNT looks like the obvious thing to use here and is not
// usable -- b8os' scheduler calls ArchDriverGetCycleAndClear() and ZEROES the
// DWT counter on every scheduling event (os.c, _b8OsProcessScheduler), so
// a difference taken across one reads garbage. clock_gettime() reads the total
// the OS accumulates from exactly that counter, and is monotonic.
static u64 now_ns(){
  struct timespec ts;
  clock_gettime( CLOCK_MONOTONIC, &ts );
  return (u64)ts.tv_sec * NS_PER_SEC + (u64)ts.tv_nsec;
}

class App : public Pico8 {
public:
  int  cur    = 0;
  int  top    = 0;
  int  bgmIdx = 0;
  bool dirty  = true;                 // header/BGM line changed: repaint everything
  bool first_draw = true;

  // What the tilemap currently shows, so _draw() can tell a selection move
  // (two rows to repaint) from a scroll or a BGM change (the whole screen).
  int  drawn_cur = -1;
  int  drawn_top = -1;

  // --- frame time accounting (see the note on now_ns() above) ---
  u64 t_enter    = 0;                 // clock at the top of this _update()
  u64 t_prev     = 0;                 // ... of the previous one
  u64 sum_work   = 0;                 // ns spent in _update()+_draw()
  u64 sum_frame  = 0;                 // ns per whole frame period
  int nsample    = 0;
  int fps        = 0;
  int workpct    = 0;

  void _init() override {
    // NOTE: pal() is not called here. It emits a PPU command, so like every
    // drawing call it is only legal inside _draw() -- calling it from _init()
    // raises NOT_DURING_DRAWING and the app dies before the first frame. The
    // palette itself persists once written, so _draw() sets it up exactly once
    // (see first_draw below), the same way pakupaku does it.
    playBgm();
    t_prev = now_ns();
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
    t_enter    = now_ns();
    sum_frame += t_enter - t_prev;
    t_prev     = t_enter;

    // Moving the selection does not set dirty: _draw() notices cur changed and
    // repaints only the rows involved.
    if( btnp(BUTTON_UP)   ){ cur = ( cur + SFX_COUNT - 1 ) % SFX_COUNT; scrollToCur(); }
    if( btnp(BUTTON_DOWN) ){ cur = ( cur + 1 ) % SFX_COUNT;             scrollToCur(); }
    if( btnp(BUTTON_O)    ) play();

    if( btnp(BUTTON_MOUSE_LEFT) ){
      // Background text is on the tile grid, so a tap maps to a row by /8.
      const int row = ( (int)mousey() ) / 8 - LIST_TOP;
      if( row >= 0 && row < ROWS && top + row < SFX_COUNT ){
        cur = top + row;
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

  // One list row, by preset index. The trailing blanks matter: rows are
  // repainted in place now, so a shorter name has to erase the longer one.
  void drawRow( int i ){
    if( i < top || i >= top + ROWS || i < 0 || i >= SFX_COUNT ) return;
    const bool sel = ( i == cur );
    cursor( 1, LIST_TOP + i - top, sel ? (BgPal)PAL_SEL : BG_PAL_0 );
    print( sel ? ">%s" : " %s", SFX_NAME[i] );
  }

  void drawTitle(){
    cursor( 1, ROW_TITLE, (BgPal)PAL_HEAD );
    print( "SFX %d/%d ", cur + 1, (int)SFX_COUNT );
  }

  void drawList(){
    print( "\e[2J" );                 // clear the whole background text plane

    drawTitle();

    cursor( 1, ROW_BGM, (BgPal)PAL_HEAD );
    print( "%d/%d %s", bgmIdx + 1, BGM_COUNT, BGM_DEFS[bgmIdx].name );

    cursor( 1, ROW_BGMST, (BgPal)PAL_HEAD );
    print( sndBgmIsPlaying() ? "BGM ON " : "BGM OFF" );

    for( int r = 0 ; r < ROWS ; ++r ) drawRow( top + r );

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

    if( dirty || top != drawn_top ){
      // Something structural changed (BGM selection, or the window scrolled):
      // the whole plane has to be re-written.
      drawList();
      dirty = false;
    } else if( cur != drawn_cur ){
      // Only the highlight moved, and it moved within the same window: the
      // two rows that changed colour plus the counter is all that is stale.
      drawTitle();
      drawRow( drawn_cur );
      drawRow( cur );
    }
    drawn_cur = cur;
    drawn_top = top;

    if( ++nsample >= STAT_WIN ){
      // fps  = how many frame periods fit in a second
      // work = share of one 60Hz frame's time spent in _update()+_draw()
      fps     = sum_frame ? (int)( (u64)STAT_WIN * NS_PER_SEC / sum_frame ) : 0;
      workpct = (int)( sum_work * 100 / ( (u64)STAT_WIN * NS_PER_FRAME ) );
      cursor( 1, ROW_STATS, (BgPal)PAL_HEAD );
      print( "%dfps W%d%%   ", fps, workpct );
      sum_frame = sum_work = 0;
      nsample   = 0;
    }

    sum_work += now_ns() - t_enter;
  }
};

int main(){
  App app;
  app.run();
  return 0;
}
