// Sound helper audition tool: 49 six-track MML pieces and all 50 SFX presets,
// side by side, so both halves of <sound.h> can be listened to and tuned by ear.
//
// The music lives in bgm.h, which is where the comment about how a piece is put
// together is too -- six tracks, three sections, its own waveforms. In short:
// every piece runs a lead, a bass, a three-voice chord detuned a few cents apart
// (see 'k') and a drum track on the noise generator, through an A/B/C form on
// three different chord progressions, with the timbres loaded into the
// wavetable's own slots 8..15 rather than taken from the factory eight. Lead and
// bass both change waveform at a section head, and in C the top chord voice
// leaves the chord to double the lead an octave down -- so what an audition
// hears is one tune through several instruments, not one instrument all through.
//
// TEXT LAYER CHOICE: all the text here is drawn on the BACKGROUND text layer
// (cursor()/print(), TILE units, 16x30 tiles on a 128x240 screen) rather than
// the sprite layer (scursor()/sprint(), PIXEL units). Background text is
// written into a tilemap once and then costs a single PPU command per frame no
// matter how much of it there is, whereas sprite text re-issues one sprite per
// glyph every frame. The pixel art beside the title is the one thing that is
// not text: it goes on the drawing layer, which cls() wipes every frame, so
// unlike the text it is re-issued every frame whether it moved or not.
//
// UI SHAPE: BGM and SE are two horizontal selectors stacked one above the
// other. Up/down picks which of the two you are on, left/right walks that
// one's list -- and the list is drawn along that same horizontal axis, the
// name sitting between a '<' and a '>'. The two used to be vertical scrolling
// columns, which put the list at right angles to the key that moved it; the
// point of this layout is that each axis is driven by the keys pointing along
// it.
//
// No text is re-drawn unless something actually changed: a full 30-line
// repaint costs ~66k cycles, a whole frame's budget, so doing one per keypress
// visibly drops a frame. A move here repaints two short rows instead.
#include <pico8.h>
#include <sound.h>
#include "bgm.h"

using namespace pico8;

// Same order as enum SndSfx. Nothing here may exceed NAME_W columns, which is
// what keeps "DOUBLEJUMP" the longest name in the list.
static const char* SFX_NAME[ SFX_COUNT ] = {
  "JUMP", "DOUBLEJUMP", "LAND", "STEP", "SWIPE", "DASH", "SHOOT", "CHARGE",
  "LASER", "MISSILE", "ZAP", "RELOAD",
  "HIT", "BOUNCE", "BREAK", "BLOCK", "CRUSH", "DAMAGE",
  "EXPLODE", "BOOM", "BLAST", "RUMBLE",
  "COIN", "GEM", "HEAL", "POWERUP", "LEVELUP", "UNLOCK", "EXTRALIFE",
  "SELECT", "CONFIRM", "CANCEL", "DENY", "BLIP", "TEXT", "PAUSE",
  "ALARM", "COUNTDOWN", "START", "CLEAR", "VICTORY", "GAMEOVER",
  "SPLASH", "BUBBLE", "WIND", "FIRE", "DOOR", "ENGINE", "MAGIC", "WARP",
};

// Background text grid: 16 tiles across, 30 visible down. Everything sits in
// the top twelve rows -- title, the two selectors, the key legend -- and the
// rest of the 240px screen is left empty. Packing upwards is what a phone in
// portrait wants: the bottom of a tall screen is where the thumb rests, so it
// is the worst place to put something you have to read.
//
// Each selector is two rows: a label carrying the index (and, for BGM, whether
// it is playing) and the name itself between the two arrows that move it.
static const int ROW_TITLE = 0;       // ... with the pixel art strip beside it
static const int BGM_ROW   = 3;
static const int SE_ROW    = 6;
static const int ROW_HELP  = 12;

static const int NAME_W    = 14;      // columns 1..14, between the two arrows

// Which selector a tap lands on: the band around each is wider than the two
// rows it draws, because a finger on a 128px-wide screen is not a mouse
// pointer. Rows 0..1 are the art strip and belong to neither.
static const int BGM_HIT_TOP = 2, BGM_HIT_BOT = 5;
static const int SE_HIT_TOP  = 6, SE_HIT_BOT  = 8;
static const int HIT_LEFT    = 3;     // x <= this taps the '<' arrow
static const int HIT_RIGHT   = 12;    // x >= this taps the '>' arrow

// The pixel art strip, in PIXELS: rows 0..1 to the right of "SNDTEST", which
// itself ends at x=64.
static const int ART_X       = 74;    // the blob
static const int ART_Y       = 5;
static const int NOTE_X      = 88;    // where a note starts its climb
static const int NOTE_RISE   = 8;     // ... and how far it drifts, up and right
static const int NOTE_DRIFT  = 24;
static const int NOTE_PERIOD = 90;    // frames per note, 1.5s at 60Hz
static const int NOTES       = 3;

// Only four background palettes exist, so they carry four meanings: yellow is
// the chrome of the selector you are on, peach the name it is showing,
// lavender everything you are not on, and light blue for the help text.
static const int PAL_HEAD  = 1;       // WHITE -> YELLOW
static const int PAL_SEL   = 2;       // WHITE -> LIGHT_PEACH
static const int PAL_DIM   = 3;       // WHITE -> LAVENDER
static const int PAL_HELP  = 0;       // WHITE -> SKY_BLUE

// The cast, one bit per pixel, MSB leftmost. A blob that hums along to the
// music and the two notes it hums; 8x8 each, which is all the height there is
// beside a title drawn in 8px text.
static const u8 GLYPH_BODY [8] = { 0x3C, 0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x42 };
static const u8 GLYPH_NOTE1[8] = { 0x1C, 0x16, 0x12, 0x10, 0x10, 0x70, 0xF0, 0x60 };
static const u8 GLYPH_NOTE2[8] = { 0x3E, 0x22, 0x22, 0x22, 0x22, 0x66, 0xEE, 0x66 };

// Draw one of those glyphs as horizontal runs rather than a pset() per pixel:
// pset() is rectfill() of a 1x1 box, so it costs a whole PPU rect command
// each, and a run of six lit pixels buys that same one command six times over.
// The blob is 14 commands this way instead of 44.
static void blit8( const u8* rows, int x, int y, Color col ){
  for( int r = 0 ; r < 8 ; ++r ){
    const int bits = rows[r];
    int c = 0;
    while( c < 8 ){
      if( !( bits & ( 0x80 >> c ) ) ){ ++c; continue; }
      int e = c;
      while( e < 8 && ( bits & ( 0x80 >> e ) ) ) ++e;
      rectfill( x + c, y + r, x + e, y + r + 1, col );   // x1/y1 are exclusive
      c = e;
    }
  }
}

// Centre s into w columns of dst (not terminated). The blank padding is the
// working part: rows are repainted in place, so it is what erases the longer
// name that was there before.
static void center( char* dst, int w, const char* s ){
  int n = 0;
  while( s[n] ) ++n;
  if( n > w ) n = w;
  const int pad = ( w - n ) / 2;
  for( int i = 0 ; i < w ; ++i ) dst[i] = ' ';
  for( int i = 0 ; i < n ; ++i ) dst[ pad + i ] = s[i];
}

// One horizontal selector. Both are the same thing at different lengths, which
// is the point of the layout: the keys do not change meaning between them.
// Wrapping is deliberate -- with no list on screen to see the end of, falling
// off one side and reappearing on the other is the cheapest way to reach the
// far side of 49 entries.
struct Sel {
  const int count;
  int cur;
  explicit Sel( int c ) : count(c), cur(0) {}
  void move( int d ){ cur = ( cur + count + d ) % count; }
};

class App : public Pico8 {
public:
  Sel bgm = Sel( BGM_COUNT );
  Sel sfx = Sel( SFX_COUNT );

  int  focus       = 0;               // 0 = the BGM selector, 1 = the SE one
  int  tick        = 0;               // frames since boot, for the animation
  int  drawn_bgm   = -1;              // what the tilemap currently shows
  int  drawn_sfx   = -1;
  int  drawn_focus = -1;
  bool drawn_on    = false;           // ... and whether the label said "ON"
  bool dirty       = true;
  bool first_draw  = true;

  void _init() override {
    // NOTE: pal() is not called here. It emits a PPU command, so like every
    // drawing call it is only legal inside _draw() -- calling it from _init()
    // raises NOT_DURING_DRAWING and the app dies before the first frame. The
    // palette itself persists once written, so _draw() sets it up exactly once
    // (see first_draw below), the same way pakupaku does it.
    playBgm();
  }

  Sel& sel( int p ){ return ( p == 0 ) ? bgm : sfx; }

  static const char* itemName( int p, int i ){
    return ( p == 0 ) ? BGM_DEFS[i].name : SFX_NAME[i];
  }

  static int selRow( int p ){ return ( p == 0 ) ? BGM_ROW : SE_ROW; }

  void playSfx(){ sndSfx( (SndSfx)sfx.cur ); }

  void playBgm(){
    const BgmDef& b = BGM_DEFS[ bgm.cur ];
    sndBgmPlay( b.t[0], b.t[1], b.t[2], b.t[3], b.t[4], b.t[5] );
  }

  // Auditioning means hearing it, so landing on an entry plays it rather than
  // waiting to be asked. d == 0 re-plays whatever is already selected.
  void pick( int p, int d ){
    sel( p ).move( d );
    if( p == 0 ) playBgm(); else playSfx();
  }

  void _update() override {
    ++tick;

    // Two rows, so either vertical key just swaps between them.
    if( btnp(BUTTON_UP) || btnp(BUTTON_DOWN) ) focus ^= 1;

    const int d = ( btnp(BUTTON_RIGHT) ? 1 : 0 ) - ( btnp(BUTTON_LEFT) ? 1 : 0 );
    if( d ) pick( focus, d );

    if( btnp(BUTTON_O) ) playSfx();
    if( btnp(BUTTON_X) ){
      // X is the only way back to silence, so it toggles rather than retriggers.
      if( sndBgmIsPlaying() ) sndBgmStop();
      else                    playBgm();
    }

    if( btnp(BUTTON_MOUSE_LEFT) ){
      // Background text is on the tile grid, so a tap maps to a cell by /8.
      const int x = ( (int)mousex() ) / 8;
      const int y = ( (int)mousey() ) / 8;
      int p = -1;
      if     ( y >= BGM_HIT_TOP && y <= BGM_HIT_BOT ) p = 0;
      else if( y >= SE_HIT_TOP  && y <= SE_HIT_BOT  ) p = 1;
      if( p >= 0 ){
        // The arrows are the buttons a touch screen has; a tap between them
        // means "this row", which is a focus move and a re-audition.
        focus = p;
        pick( p, x <= HIT_LEFT ? -1 : ( x >= HIT_RIGHT ? 1 : 0 ) );
      }
    }
  }

  // The strip beside the title. It is wired to sndBgmIsPlaying() rather than
  // left to run free: a blob asleep with its eyes shut says "the music is
  // stopped" more plainly than the word OFF three rows below it does.
  void drawArt(){
    const bool on = sndBgmIsPlaying();

    static const int BOB[4] = { 0, 1, 2, 1 };
    const int by = ART_Y + ( on ? BOB[ ( tick >> 3 ) & 3 ] : 2 );

    blit8( GLYPH_BODY, ART_X, by, on ? Color::PINK : Color::DARK_GREY );

    // A blink every 2.5s is what stops it reading as a decal; while the music
    // is off the eyes just stay shut.
    if( !on || ( tick % 150 ) < 6 ){
      rectfill( ART_X + 1, by + 4, ART_X + 3, by + 5, Color::BLACK );
      rectfill( ART_X + 5, by + 4, ART_X + 7, by + 5, Color::BLACK );
    } else {
      rectfill( ART_X + 2, by + 3, ART_X + 3, by + 5, Color::BLACK );
      rectfill( ART_X + 5, by + 3, ART_X + 6, by + 5, Color::BLACK );
    }

    if( !on ) return;

    rectfill( ART_X + 3, by + 6, ART_X + 5, by + 7, Color::BLACK );  // singing

    // Three notes on one path, evenly spread around it, so the strip always
    // has something crossing it however you catch it.
    static const Color NOTE_COL[NOTES] = {
      Color::YELLOW, Color::LIGHT_PEACH, Color::WHITE,
    };
    for( int i = 0 ; i < NOTES ; ++i ){
      const int p = ( tick + i * ( NOTE_PERIOD / NOTES ) ) % NOTE_PERIOD;
      blit8( ( i & 1 ) ? GLYPH_NOTE2 : GLYPH_NOTE1,
             NOTE_X + p * NOTE_DRIFT / NOTE_PERIOD,
             NOTE_RISE - p * NOTE_RISE / NOTE_PERIOD,
             NOTE_COL[i] );
    }
  }

  void drawSel( int p ){
    Sel& s = sel( p );
    const bool f = ( focus == p );
    const int  r = selRow( p );

    // Label row. The trailing blanks matter here as much as anywhere: "OFF"
    // has to cover the "ON " that was there, and " 9/49" the "10/49".
    cursor( 0, r, f ? (BgPal)PAL_HEAD : (BgPal)PAL_DIM );
    if( p == 0 ) print( "%cBGM  %2d/%d %s", f ? '>' : ' ', s.cur + 1, s.count,
                        sndBgmIsPlaying() ? "ON " : "OFF" );
    else         print( "%cSE   %2d/%d    ", f ? '>' : ' ', s.cur + 1, s.count );

    // Name row, printed as one string so the two arrows stay at fixed columns
    // whatever the name between them is.
    char line[ NAME_W + 3 ];
    line[0] = '<';
    center( line + 1, NAME_W, itemName( p, s.cur ) );
    line[ NAME_W + 1 ] = '>';
    line[ NAME_W + 2 ] = 0;
    cursor( 0, r + 1, f ? (BgPal)PAL_SEL : (BgPal)PAL_DIM );
    print( "%s", line );
  }

  void drawAll(){
    print( "\e[2J" );                 // clear the whole background text plane
    cursor( 1, ROW_TITLE, (BgPal)PAL_HEAD ); print( "SNDTEST" );
    drawSel( 0 );
    drawSel( 1 );
    cursor( 1, ROW_HELP,     (BgPal)PAL_HELP ); print( "UP/DN  BGM/SE" );
    cursor( 1, ROW_HELP + 2, (BgPal)PAL_HELP ); print( "L/R    SELECT" );
    cursor( 1, ROW_HELP + 4, (BgPal)PAL_HELP ); print( "Z SE   X BGM" );
  }

  void _draw() override {
    cls( Color::DARK_BLUE );

    if( first_draw ){
      // Palettes are PPU state and stick once set, so this runs on frame 0 only.
      pal( Color::WHITE, Color::YELLOW,      PAL_HEAD );
      pal( Color::WHITE, Color::LIGHT_PEACH, PAL_SEL  );
      pal( Color::WHITE, Color::LAVENDER,    PAL_DIM  );
      pal( Color::WHITE, Color::BLUE,        PAL_HELP );
      first_draw = false;
    }

    const bool on   = sndBgmIsPlaying();
    const bool swap = ( focus != drawn_focus );   // both rows change colour

    if( dirty ){
      drawAll();
      dirty = false;
    } else {
      if( swap || bgm.cur != drawn_bgm || on != drawn_on ) drawSel( 0 );
      if( swap || sfx.cur != drawn_sfx )                   drawSel( 1 );
    }

    drawn_bgm   = bgm.cur;
    drawn_sfx   = sfx.cur;
    drawn_focus = focus;
    drawn_on    = on;

    drawArt();
  }
};

int main(){
  App app;
  app.run();
  return 0;
}
