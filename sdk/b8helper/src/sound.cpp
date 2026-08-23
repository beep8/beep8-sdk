/**
 * @file sound.cpp
 * @brief Implementation of the BEEP-8 sound helper (see <sound.h>).
 *
 * Layout of the APU, which this file owns end to end:
 *
 *   WSG 0..3   music tracks 0..3      (b8ApuReset() sets MAXCH = 8, so only
 *   WSG 4..7   sound-effect voices     WSG 0..7 are mixed at all)
 *   NOISE 0    sound effects
 *   NOISE 1    music, for '@n' drum tracks
 *
 * Timing is kept in ticks, one tick being one step of the APU (1/120 s), and
 * subdivided 256 ways below that. Note durations rarely divide evenly into
 * whole ticks, so every note carries its remainder over into the next one (see
 * track_advance) instead of rounding — otherwise a fast track would drift
 * audibly against a slow one within a few bars.
 *
 * MML is parsed lazily, straight off the caller's string, one note at a time.
 * That is why the strings must outlive playback (documented in sound.h): it
 * buys zero allocation and zero fixed size limit on a piece of music.
 *
 * On top of the notes sits one modulation layer, driven from the same tick:
 * two LFOs (mp vibrato, mv tremolo), a volume envelope (me), a detune (k), a
 * pitch sweep (ms) and a portamento (mg). Everything there is expressed in
 * musical units -- cents, percent, Hz, milliseconds -- and converted once, so
 * the per-tick work is a table lookup and a couple of multiplies per track.
 * See track_modulate(); freq_shift_cents() is the arithmetic underneath it.
 *
 * The sequencer runs on its own thread (tick_thread), woken by the APU's own
 * sync interrupt rather than by the game loop -- see ensure_init(). Everything
 * below therefore runs on two threads: the game thread through the public API,
 * and the tick thread through tick_locked(). g_lock serialises the two; the
 * only globals read outside it are the ones marked volatile.
 */
#include <sound.h>
#include <b8/apu.h>
#include <b8/pthread.h>
#include <b8/semaphore.h>
#include <b8/syscall.h>   // usleep
#include <stdio.h>

namespace {

// ---------------------------------------------------------------------------
// Channel policy
// ---------------------------------------------------------------------------
const u32 BGM_CH0       = 0;   // WSG 0..3
const int BGM_TRACKS    = 4;
const u32 SFX_CH0       = 4;   // WSG 4..7
const int SFX_VOICES    = 4;
const u32 SFX_NOISE_CH  = 0;
const u32 BGM_NOISE_CH  = 1;

const int MML_REP_DEPTH = 4;

// The sequencer's clock. The APU raises B8_IRQ_APUS once per step and steps
// 120 times a second (200 samples of its 24kHz output per step), so one tick is
// one interrupt: no counting, no phase to get wrong, and note boundaries land
// on half-frames rather than being quantised to the display.
const int TICKS_PER_SEC = 120;

// Sub-tick timing resolution: one tick == UNITS_PER_TICK units.
const int UNITS_PER_TICK = 256;

// Enough for parse_next() and one MMIO-poking tick; nothing here recurses.
const int TICK_STACK = 0x800;

// Set once by ensure_init(), before the tick thread that reads it exists, and
// never cleared. Read unlocked by the entry points that must not create the
// lock they would otherwise take.
volatile bool g_inited = false;

// Guards every global below plus the APU registers. Valid only once g_inited.
sem_t g_lock;

// Volume trims are a single aligned word written by the game thread and read by
// the tick thread; on this single-core CPU that store is atomic, so they are the
// one pair of globals deliberately left outside g_lock -- taking it would mean
// two syscalls for what is a scale factor on the next note.
volatile int g_bgm_vol_pct = 100;
volatile int g_sfx_vol_pct = 100;

// Likewise readable unlocked, for sndBgmIsPlaying().
volatile bool g_bgm_on = false;

int  g_tempo       = 120;
bool g_bgm_loop    = true;
unsigned g_age     = 0;

// RAII around g_lock. Only ever constructed once g_inited is true.
struct Lock {
  Lock()  { sem_wait( &g_lock ); }
  ~Lock() { sem_post( &g_lock ); }
};

int clampi( int v, int lo, int hi ){ return v < lo ? lo : ( v > hi ? hi : v ); }

int lerpi( int a, int b, int i, int n ){
  if( n <= 1 ) return b;
  return a + ( b - a ) * i / n;
}

void tick_locked();

// The sequencer's clock. It deliberately does not run on the game loop: a frame
// that overruns vsync -- one heavy _draw(), a cursor being dragged, a stall in
// the browser -- would stretch every note in the bar with it, which is exactly
// the tempo wobble this thread exists to remove. B8_IRQ_APUS instead comes from
// the APU itself, one interrupt per 200 generated samples, and one tick per
// interrupt, so every tick is the same amount of *audio* whatever the renderer
// is doing.
void* tick_thread( void* ){
  // b8ApuReset() already armed the IRQ; ensure_init() muted it again and left
  // switching it on to us, so that no backlog can pile up on the semaphore
  // between the reset and this thread reaching its first wait.
  B8_APU_INTCTRL = 1;

  for(;;){
    // Nothing recoverable can make the wait fail, but spinning on it at full
    // tilt would take the CPU away from the game. Fall back to a tick's sleep.
    if( b8ApuSyncWait() < 0 ){ usleep( 1000000 / TICKS_PER_SEC ); continue; }

    Lock lk;
    tick_locked();
  }
  return 0;
}

// The APU is only ever touched once a game actually asks for a sound, so a
// silent game behaves exactly as it did before this helper existed -- no APU
// reset, no sync interrupt, and no tick thread.
//
// Called from the public API only, i.e. always on the game thread: the tick
// thread does not exist until the tail of this function, and never calls in.
void ensure_init(){
  if( g_inited ) return;

  b8ApuReset();        // enables B8_IRQ_APUS (and B8_APU_INTCTRL) as a side effect
  B8_APU_INTCTRL = 0;  // ... which tick_thread turns back on once it is waiting

  sem_init( &g_lock, 0, 1 );
  g_inited = true;     // Lock is usable from here on, so the thread may start

  pthread_attr_t attr;
  pthread_attr_init( &attr );
  pthread_attr_setstacksize( &attr, TICK_STACK );
  pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );

  pthread_t pid;
  if( pthread_create( &pid, &attr, tick_thread, 0 ) != 0 ){
    // Out of TCBs (32 per program). There is no second way to drive the
    // sequencer, so say so rather than sounding like a broken APU.
    fprintf( stderr, "sound: no thread for the sequencer; there will be no sound\n" );
  }
}

// Noise volume is an attenuation shift (smaller = louder), so a percentage trim
// can only be approximated in whole shifts.
int noise_trim( int pct ){
  if( pct >= 90 ) return 0;
  if( pct >= 60 ) return 1;
  if( pct >= 35 ) return 2;
  if( pct >= 15 ) return 3;
  if( pct >   0 ) return 4;
  return B8_APU_NCHVOLDIV_MUTE;
}

// ---------------------------------------------------------------------------
// Modulation primitives, shared by the sound effects and the sequencer
// ---------------------------------------------------------------------------
//
// Everything that moves within a note -- vibrato, tremolo, the volume envelope,
// pitch sweeps, portamento -- is driven from the same 120 Hz tick as the notes
// themselves. That tick is therefore also the LFOs' sample rate: a 6 Hz
// vibrato gets 20 samples per cycle and sounds smooth, while anything past
// ~12 Hz turns into a deliberate stepped effect rather than a smooth waver.
// LFO_HZ_MAX caps it where the two readings meet.
const int LFO_HZ_MAX  = 20;
const int LFO_CENTS_MAX = 200;   // pitch depth: 200 cents == a whole tone
const int LFO_DELAY_MAX = 2000;  // ms

// One LFO cycle as 64 signed samples. Only the sine needs a table -- the other
// three shapes fall straight out of the phase arithmetic.
const s8 LFO_SINE[64] = {
     0,   12,   25,   37,   49,   60,   71,   81,
    90,   98,  106,  112,  117,  122,  125,  126,
   127,  126,  125,  122,  117,  112,  106,   98,
    90,   81,   71,   60,   49,   37,   25,   12,
     0,  -12,  -25,  -37,  -49,  -60,  -71,  -81,
   -90,  -98, -106, -112, -117, -122, -125, -126,
  -127, -126, -125, -122, -117, -112, -106,  -98,
   -90,  -81,  -71,  -60,  -49,  -37,  -25,  -12,
};

// Shape of one LFO cycle at a 16-bit phase, as -128..127.
s32 lfo_sample( int wave, u16 phase ){
  switch( wave ){
    case 1: {                                        // triangle
      const s32 q = phase >> 7;                      // 0..511
      return ( q < 256 ) ? ( q - 128 ) : ( 383 - q );
    }
    case 2: return ( phase < 32768 ) ? 127 : -128;   // square -- a trill
    case 3: return 127 - (s32)( phase >> 8 );        // falling ramp
    default: return LFO_SINE[ phase >> 10 ];         // sine
  }
}

// Phase increment per tick for an LFO running at hz.
u16 lfo_inc( int hz ){ return (u16)( hz * 65536 / TICKS_PER_SEC ); }

// Where in its cycle each shape is at maximum. A *volume* LFO is read through
// this offset, so a note starts at exactly the level its 'v' asked for and only
// ever dips below it; a *pitch* LFO reads the raw phase, which starts the sine
// and the triangle at the written pitch rather than a depth away from it.
const u16 LFO_PEAK[4] = { 0x4000, 0x8000, 0x0000, 0x0000 };
s32 lfo_vol_sample( int wave, u16 phase ){
  return lfo_sample( wave, (u16)( phase + LFO_PEAK[ wave & 3 ] ) );
}

// Per-tick volume multiplier, in Q16, indexed by the 'me' decay rate. Index 0
// holds the level for the whole note (an organ); 1..15 halve it in 2.1s, 1.5s,
// 1.1s, 0.75s, 0.53s, 0.38s, 0.27s, 0.19s, 0.13s, 0.09s, 0.07s, 0.05s, 0.03s,
// 0.03s, 0.02s respectively. DEF_DECAY reproduces the fixed pluck this file
// used to hard-code before 'me' existed.
const u32 DECAY_MUL[16] = {
   65536,  65359,  65286,  65182,  65033,  64830,  64534,  64132,
   63590,  62757,  61534,  60097,  58386,  55109,  52016,  46341,
};
const int DEF_DECAY = 6;

// 2^(n/12) in Q16 -- one semitone per step.
const u32 SEMI_Q16[12] = {
   65536,  69433,  73562,  77936,  82570,  87480,
   92682,  98193, 104032, 110218, 116772, 123715,
};

// (2^(c/1200) - 1) in Q20, for the 0..99 cents inside one semitone.
const u16 CENT_Q20[100] = {
      0,   606,  1212,  1819,  2426,  3033,  3640,  4248,  4857,  5465,
   6074,  6684,  7293,  7903,  8514,  9125,  9736, 10347, 10959, 11571,
  12184, 12797, 13410, 14024, 14638, 15252, 15867, 16482, 17097, 17713,
  18329, 18945, 19562, 20179, 20797, 21415, 22033, 22651, 23270, 23890,
  24509, 25129, 25750, 26370, 26992, 27613, 28235, 28857, 29479, 30102,
  30726, 31349, 31973, 32598, 33222, 33847, 34473, 35098, 35725, 36351,
  36978, 37605, 38233, 38861, 39489, 40118, 40747, 41376, 42006, 42636,
  43267, 43897, 44529, 45160, 45792, 46424, 47057, 47690, 48324, 48957,
  49591, 50226, 50861, 51496, 52132, 52768, 53404, 54041, 54678, 55315,
  55953, 56591, 57230, 57869, 58508, 59148, 59788, 60428, 61069, 61710,
};

// Transpose a FREQ value (a phase increment, linear in Hz) by a number of
// cents. Splitting the shift into octaves, semitones and cents holds the result
// inside ~1 cent of true equal temperament everywhere -- and what is left is
// FREQ's own resolution at the bottom of the range, not this arithmetic. The
// obvious one-line "freq + freq * cents * k" approximation is already 3 cents
// out at a semitone and falls apart entirely by an octave, which is fine for a
// shallow vibrato and useless for the sweeps and detunes that share this.
u32 freq_shift_cents( u32 f, s32 cents ){
  if( cents == 0 || f == 0 ) return f;
  cents = clampi( cents, -9600, 9600 );

  // Floor rather than truncate, so the cents remainder is always positive and
  // the two tables below only ever need their upward halves.
  s32 semi = cents / 100;
  s32 rem  = cents - semi * 100;
  if( rem < 0 ){ rem += 100; --semi; }
  s32 oct = semi / 12;
  semi -= oct * 12;
  if( semi < 0 ){ semi += 12; --oct; }

  // Both ratios are applied before the octave shift: shifting a low note's
  // small FREQ down first would throw away the bits they need.
  u32 v = (u32)( ( (unsigned long long)f * SEMI_Q16[ semi ] ) >> 16 );
  v += (u32)( ( (unsigned long long)v * CENT_Q20[ rem ] ) >> 20 );
  if     ( oct > 0 ) v = ( oct >= 12 ) ? 0x1fffffu : ( v << oct );
  else if( oct < 0 ) v = ( -oct >= 24 ) ? 0u : ( v >> -oct );
  return v > 0x1fffffu ? 0x1fffffu : v;   // past ~24 kHz the phase is meaningless
}

// ---------------------------------------------------------------------------
// Sound effects
// ---------------------------------------------------------------------------

struct SfxDef {
  unsigned char  kind;    // 0 = tone, 1 = noise
  unsigned char  wav;     // tone waveform
  unsigned char  mode;    // 0 = glide hz0->hz1 | 1 = step at the half | 2 = step at the thirds
  unsigned char  ticks;   // total duration, in 1/120 s ticks
  unsigned short hz0, hz1, hz2;   // hz2 is only read in mode 2
  unsigned short v0, v1;  // tone: CHVOL 0..4095 | noise: attenuation shift 0..16
  // One sine LFO per preset, feeding both depths at once -- a wobble is nearly
  // always wanted on the pitch and the volume together, and sharing the phase
  // keeps the table to three extra columns.
  unsigned char  lhz;     // LFO speed in Hz; 0 = no LFO at all
  unsigned char  lpd;     //   pitch depth, in cents
  unsigned char  lvd;     //   volume depth, in percent (tones only -- see below)
};

// Indexed by SndSfx — keep in the same order as that enum, which is grouped by the
// kind of game event rather than by how the sound is built.
//
//                         kind wav mode  tck    hz0    hz1    hz2     v0    v1  lhz  lpd  lvd
const SfxDef SFX[ SFX_COUNT ] = {
  // -- player action --
  /* SFX_JUMP     */ {  0,  0,  0,  20,   200,   700,     0,  1800,  300,   0,   0,   0 },
  /* SFX_LAND     */ {  1,  0,  0,  14,   260,   100,     0,     0,    7,   0,   0,   0 },
  /* SFX_STEP     */ {  1,  0,  0,   8,   700,   450,     0,     1,    7,   0,   0,   0 },
  /* SFX_SWIPE    */ {  1,  0,  0,  22,  4000,   500,     0,     2,    9,   0,   0,   0 },
  /* SFX_SHOOT    */ {  0,  1,  0,  16,  1400,   350,     0,  1300,  100,   0,   0,   0 },
  /* SFX_CHARGE   */ {  0,  1,  0,  60,   200,   900,     0,   900, 1500,   7,  30,  25 },

  // -- impact and destruction --
  /* SFX_HIT      */ {  1,  0,  0,  12,   600,   150,     0,     0,    7,   0,   0,   0 },
  /* SFX_BOUNCE   */ {  0,  0,  0,  14,   660,   440,     0,  1400,  200,   0,   0,   0 },
  /* SFX_BREAK    */ {  1,  0,  0,  28,  2200,   500,     0,     0,    9,  16, 200,   0 },
  /* SFX_BLOCK    */ {  0,  6,  0,  14,  1760,  1400,     0,  1500,  200,   0,   0,   0 },
  /* SFX_EXPLODE  */ {  1,  0,  0,  52,   800,   120,     0,     0,   10,   5, 150,   0 },
  /* SFX_DAMAGE   */ {  0,  1,  0,  36,   400,    90,     0,  1800,  200,  13,  60,  40 },

  // -- pickups and rewards --
  /* SFX_COIN     */ {  0,  0,  1,  16,   988,  1319,     0,  1700,  900,   0,   0,   0 },
  /* SFX_HEAL     */ {  0,  3,  0,  44,   587,  1175,     0,  1200,  500,   6,  25,  20 },
  /* SFX_POWERUP  */ {  0,  0,  0,  40,   523,  1568,     0,  1500,  700,   0,   0,   0 },
  /* SFX_LEVELUP  */ {  0,  0,  2,  60,   523,   784,  1047,  1600, 1200,   6,  20,   0 },
  /* SFX_UNLOCK   */ {  0,  5,  2,  24,   440,   659,   880,  1500, 1100,   0,   0,   0 },

  // -- menus and UI --
  /* SFX_SELECT   */ {  0,  0,  0,  12,   880,   880,     0,  1200,  200,   0,   0,   0 },
  /* SFX_CONFIRM  */ {  0,  0,  1,  20,   880,  1319,     0,  1400,  700,   0,   0,   0 },
  /* SFX_CANCEL   */ {  0,  0,  1,  20,   880,   587,     0,  1400,  500,   0,   0,   0 },
  /* SFX_DENY     */ {  0,  1,  0,  24,   220,   175,     0,  1500,  300,  14,   0,  55 },
  /* SFX_BLIP     */ {  0,  0,  0,   8,  1320,  1320,     0,   900,  100,   0,   0,   0 },

  // -- game state --
  /* SFX_ALARM    */ {  0,  0,  2,  48,   880,   587,   880,  1500, 1200,   9,   0,  45 },
  /* SFX_CLEAR    */ {  0,  0,  1,  48,   784,  1568,     0,  1600, 1200,   6,  20,   0 },
  /* SFX_GAMEOVER */ {  0,  2,  0,  80,   392,   131,     0,  1800,  300,   4,  35,  20 },

  // -- environment --
  /* SFX_SPLASH   */ {  1,  0,  0,  24,  3500,   900,     0,     1,    9,   7, 180,   0 },
  /* SFX_WARP     */ {  0,  4,  0,  36,   300,  2200,     0,  1400,  400,  15,  70,   0 },
};
struct SfxVoice {
  const SfxDef* def   = 0;
  int           tick  = 0;
  unsigned      age   = 0;
  bool          active= false;
  u16           phase = 0;   // LFO phase; reset on every retrigger
};

SfxVoice g_sfx[ SFX_VOICES ];   // tone voices, WSG SFX_CH0 + n
SfxVoice g_sfx_noise;           // the single noise voice, NOISE SFX_NOISE_CH

void sfx_voice_stop( SfxVoice& v, u32 ch, bool noise ){
  if( !v.active ) return;
  v.active = false;
  if( noise ) b8ApuStopNoise( ch );
  else        b8ApuStopTone( ch );
}

void sfx_voice_step( SfxVoice& v, u32 ch, bool noise ){
  if( !v.active ) return;
  const SfxDef* d = v.def;
  if( v.tick >= d->ticks ){ sfx_voice_stop( v, ch, noise ); return; }

  const int i  = v.tick;
  const int n  = d->ticks;
  int hz;
  if     ( d->mode == 1 ) hz = ( i * 2 < n ) ? d->hz0 : d->hz1;
  else if( d->mode == 2 ) hz = ( i * 3 < n ) ? d->hz0 : ( ( i * 3 < n * 2 ) ? d->hz1 : d->hz2 );
  else                    hz = lerpi( d->hz0, d->hz1, i, n );

  // The LFO rides on top of the sweep the three modes above lay down. Its phase
  // starts at zero on every retrigger, so a preset always sounds identical.
  s32 lfo = 0, lfo_v = 0;
  if( d->lhz ){
    lfo     = lfo_sample    ( 0, v.phase );
    lfo_v   = lfo_vol_sample( 0, v.phase );
    v.phase = (u16)( v.phase + lfo_inc( d->lhz ) );
  }

  u32 freq = b8ApuHzToFreq( (u32)hz );
  if( d->lpd ) freq = freq_shift_cents( freq, (s32)d->lpd * lfo / 128 );

  if( noise ){
    // Noise volume is an attenuation shift, so a percentage tremolo could only
    // move it in 6 dB stairs; on a noise preset lvd is ignored and the pitch is
    // the only thing the LFO touches.
    int vd = lerpi( d->v0, d->v1, i, n ) + noise_trim( g_sfx_vol_pct );
    b8ApuPlayNoise( ch, (u32)clampi( vd, 0, B8_APU_NCHVOLDIV_MUTE ), freq );
  } else {
    int vol = lerpi( d->v0, d->v1, i, n ) * g_sfx_vol_pct / 100;
    // Downward only, so a tremolo can never push a loud preset into the
    // mixer's ceiling: full level at the top of the cycle, lvd percent down at
    // the bottom.
    if( d->lvd ) vol -= vol * (int)d->lvd * (int)( 127 - lfo_v ) / 25500;
    // Pitch and volume move every tick, the waveform never does — set it on
    // the first tick only (same MMIO-write argument as the BGM decay above).
    if( i == 0 ) b8ApuPlayTone( ch, d->wav, (u32)clampi( vol, 0, B8_APU_CHVOL_MAX ), freq );
    else {
      B8_APU_FREQ ( ch ) = freq;
      B8_APU_CHVOL( ch ) = (u32)clampi( vol, 0, B8_APU_CHVOL_MAX );
    }
  }
  ++v.tick;
}

void sfx_tick(){
  for( int i = 0 ; i < SFX_VOICES ; ++i )
    sfx_voice_step( g_sfx[i], SFX_CH0 + (u32)i, false );
  sfx_voice_step( g_sfx_noise, SFX_NOISE_CH, true );
}

// ---------------------------------------------------------------------------
// MML sequencer
// ---------------------------------------------------------------------------

// One LFO's worth of per-track state. inc == 0 means the LFO is off.
struct Lfo {
  u16 inc   = 0;   // phase increment per tick
  u16 depth = 0;   // cents (pitch) or percent (volume)
  u16 delay = 0;   // ticks after note-on before it starts moving
  u8  wave  = 0;
  u16 phase = 0;
};

struct MmlTrack {
  const char* start   = 0;
  const char* p       = 0;
  bool  active        = false;
  bool  noise         = false;
  int   octave        = 4;
  int   deflen        = 8;
  int   vol           = 10;
  int   wav           = 0;
  int   gate          = 6;
  int   dur_left      = 0;   // 1/256-tick units until the next command
  int   gate_left     = 0;   // 1/256-tick units until note-off
  bool  sounding      = false;

  // -- voicing: set by the m* and k commands, and kept across notes --
  Lfo   plfo;                // 'mp' vibrato
  Lfo   vlfo;                // 'mv' tremolo
  int   decay         = DEF_DECAY;   // 'me', index into DECAY_MUL
  int   attack_ms     = 0;   // 'me' second argument
  int   detune        = 0;   // 'k',  cents
  int   sweep         = 0;   // 'ms', cents per second from note-on
  int   glide_ms      = 0;   // 'mg', portamento time from the previous note

  // -- per-note state, rebuilt by note_on() --
  int   note_ticks    = 0;   // ticks since note-on; drives delay/sweep/glide
  u32   base_freq     = 0;   // the note's own pitch, detune included
  u32   out_freq      = 0;   // what FREQ was last actually set to
  int   out_vol       = -1;  // ... and CHVOL; -1 so the first write always lands
  u32   glide_from    = 0;
  int   glide_ticks   = 0;
  int   glide_left    = 0;
  int   attack_step   = 0;
  int   attack_left   = 0;
  int   peak_vol      = 0;   // level the decay starts from
  int   cur_vol       = 0;   // the envelope's current output, before tremolo

  const char* rep_p[ MML_REP_DEPTH ] = { 0, 0, 0, 0 };
  int   rep_n[ MML_REP_DEPTH ]       = { 0, 0, 0, 0 };
  int   rep_sp        = 0;
};

MmlTrack g_trk[ BGM_TRACKS ];

int chvol( int v ){
  // Roughly perceptual: v=15 -> 2925, v=10 -> 1300, v=8 -> 832. Four tracks at
  // full tilt then still sit inside the mixer's headroom.
  const int a = v * v * 13 * g_bgm_vol_pct / 100;
  return clampi( a, 0, B8_APU_CHVOL_MAX );
}

void note_off( MmlTrack& t, int idx ){
  if( t.noise ) b8ApuStopNoise( BGM_NOISE_CH );
  else          b8ApuStopTone( BGM_CH0 + (u32)idx );
}

void note_on( MmlTrack& t, int idx, int keynum ){
  // b8ApuGetNoteFreq() indexes an octave table with keynum % 12, so a negative
  // key would read out of bounds. Clamp rather than trusting the MML.
  keynum = clampi( keynum, 0, 95 );
  u32 f = b8ApuGetNoteFreq( (s16)keynum );
  if( t.detune ) f = freq_shift_cents( f, t.detune );

  t.base_freq  = f;
  t.note_ticks = 0;
  t.plfo.phase = 0;
  t.vlfo.phase = 0;

  // Portamento slides in from wherever the track's pitch was left, so the very
  // first note of a track -- nothing to slide from -- simply starts in place.
  if( t.glide_ms > 0 && t.out_freq ){
    t.glide_ticks = clampi( t.glide_ms * TICKS_PER_SEC / 1000, 1, 480 );
    t.glide_left  = t.glide_ticks;
    t.glide_from  = t.out_freq;
    f             = t.out_freq;
  } else {
    t.glide_left  = 0;
  }
  t.out_freq = f;

  if( t.noise ){
    // Same pitch value drives the noise clock, so low octaves read as kicks and
    // high ones as hats.
    const int vd = clampi( B8_APU_NCHVOLDIV_MUTE - t.vol + noise_trim( g_bgm_vol_pct ),
                           0, B8_APU_NCHVOLDIV_MUTE );
    b8ApuPlayNoise( BGM_NOISE_CH, (u32)vd, f );
  } else {
    t.peak_vol = chvol( t.vol );
    if( t.attack_ms > 0 ){
      const int at  = clampi( t.attack_ms * TICKS_PER_SEC / 1000, 1, 480 );
      t.attack_left = at;
      t.attack_step = t.peak_vol / at;
      if( t.attack_step < 1 ) t.attack_step = 1;
      t.cur_vol     = 0;
    } else {
      t.attack_left = 0;
      t.cur_vol     = t.peak_vol;
    }
    t.out_vol = t.cur_vol;
    b8ApuPlayTone( BGM_CH0 + (u32)idx, (u32)t.wav, (u32)t.cur_vol, f );
  }
  t.sounding = true;
}

// Everything that moves *within* a note: portamento, sweep, the two LFOs and
// the volume envelope. It runs once per tick for every sounding track, so both
// registers are written only when the value actually changed -- a plain note on
// a plain voice then still costs the single CHVOL write it always did, and one
// with no envelope and no modulation at all costs nothing.
void track_modulate( MmlTrack& t, int idx ){
  const int n = t.note_ticks;

  // ---- pitch ----
  u32 f = t.base_freq;
  if( t.glide_left > 0 ){
    f = (u32)lerpi( (int)t.glide_from, (int)t.base_freq,
                    t.glide_ticks - t.glide_left, t.glide_ticks );
    --t.glide_left;
  }

  s32 cents = 0;
  if( t.sweep ) cents += t.sweep * n / TICKS_PER_SEC;
  if( t.plfo.inc && n >= (int)t.plfo.delay ){
    cents       += (s32)t.plfo.depth * lfo_sample( t.plfo.wave, t.plfo.phase ) / 128;
    t.plfo.phase = (u16)( t.plfo.phase + t.plfo.inc );
  }
  if( cents ) f = freq_shift_cents( f, cents );

  if( f != t.out_freq ){
    t.out_freq = f;
    if( t.noise ) B8_APU_NFREQ( BGM_NOISE_CH )       = f;
    else          B8_APU_FREQ ( BGM_CH0 + (u32)idx ) = f;
  }

  // A noise channel's volume is an attenuation shift, so both the envelope and
  // the tremolo would come out as 6 dB stairs. A drum track keeps the flat
  // level note_on() gave it and only its pitch is modulated.
  if( t.noise ) return;

  // ---- volume ----
  if( t.attack_left > 0 ){
    // The last step lands exactly on peak_vol rather than one rounding short.
    --t.attack_left;
    t.cur_vol = ( t.attack_left == 0 || t.cur_vol + t.attack_step >= t.peak_vol )
                  ? t.peak_vol : t.cur_vol + t.attack_step;
  } else if( t.decay ){
    // A flat volume for the whole note reads as an organ -- which is what decay
    // 0 is for. Anything else bleeds away at the rate DECAY_MUL sets, so the
    // note reads as plucked.
    t.cur_vol = (int)( ( (u32)t.cur_vol * DECAY_MUL[ t.decay ] ) >> 16 );
  }

  int v = t.cur_vol;
  if( t.vlfo.inc && n >= (int)t.vlfo.delay ){
    // Downward only: the note sits at its written volume at the top of the
    // cycle and `depth` percent quieter at the bottom, so a tremolo can never
    // push a loud note into the mixer's ceiling.
    const s32 sm = lfo_vol_sample( t.vlfo.wave, t.vlfo.phase );
    v           -= (int)( (s32)v * (s32)t.vlfo.depth * ( 127 - sm ) / 25500 );
    t.vlfo.phase = (u16)( t.vlfo.phase + t.vlfo.inc );
  }
  v = clampi( v, 0, B8_APU_CHVOL_MAX );
  if( v != t.out_vol ){
    t.out_vol = v;
    B8_APU_CHVOL( BGM_CH0 + (u32)idx ) = (u32)v;
  }
}

void set_noise( MmlTrack& t, int idx, bool on ){
  if( t.noise == on ) return;
  if( t.sounding ){ note_off( t, idx ); t.sounding = false; }
  else             note_off( t, idx );   // silence whichever channel it just left
  t.noise    = on;
  t.out_freq = 0;   // nothing for the next 'mg' portamento to slide from
}

int read_uint( const char*& p, int def ){
  if( *p < '0' || *p > '9' ) return def;
  int v = 0;
  while( *p >= '0' && *p <= '9' ){
    v = v * 10 + ( *p - '0' );
    if( v > 9999 ) v = 9999;
    ++p;
  }
  return v;
}

// Same, but for the arguments that carry a sign ('k' and 'ms'). The cursor is
// only moved when a number really was there, so a bare '-' cannot swallow it.
int read_int( const char*& q, int def ){
  const char* p = q;
  int sign = 1;
  if     ( *p == '-' ){ sign = -1; ++p; }
  else if( *p == '+' ){ ++p; }
  if( *p < '0' || *p > '9' ) return def;
  const int v = read_uint( p, 0 );
  q = p;
  return sign * v;
}

// The m* commands take comma-separated arguments, all but the first optional.
bool next_arg( const char*& p ){
  if( *p != ',' ) return false;
  ++p;
  return true;
}

int norm_len( int n ){ return clampi( n, 1, 64 ); }

// Length of one note in 1/256-tick units. 120 ticks/sec * (60/BPM) sec per beat
// * 4 beats per whole note == 28800/BPM ticks per whole note, and 256 units to
// the tick makes 7372800/BPM.
int len_units( int len, int dots ){
  int u   = 7372800 / ( g_tempo * norm_len( len ) );
  int add = u;
  for( int i = 0 ; i < dots ; ++i ){ add /= 2; u += add; }
  return u > 0 ? u : 1;
}

void read_len( MmlTrack& t, int& len, int& dots ){
  const int n = read_uint( t.p, 0 );
  len  = n > 0 ? n : t.deflen;
  dots = 0;
  while( *t.p == '.' && dots < 3 ){ ++dots; ++t.p; }
}

int semitone_of( char c ){
  switch( c ){
    case 'c': return  0;
    case 'd': return  2;
    case 'e': return  4;
    case 'f': return  5;
    case 'g': return  7;
    case 'a': return  9;
    case 'b': return 11;
  }
  return -1;
}

// Arm a note of the given length and start it sounding. q8 holds straight
// through to the next command; anything less leaves a gap, so the note reads as
// detached.
void begin_note( MmlTrack& t, int idx, int keynum, int len, int dots ){
  const int u = len_units( len, dots );
  t.dur_left  = u;
  t.gate_left = ( t.gate >= 8 ) ? ( u + UNITS_PER_TICK ) : ( u * t.gate / 8 );
  note_on( t, idx, keynum );
}

// Read one LFO's arguments: speed[,depth[,delay_ms[,wave]]]. Speed 0 turns it
// off, which is what 'mp0' / 'mv0' are for.
void read_lfo( MmlTrack& t, Lfo& L, int depth_max ){
  const int hz = clampi( read_uint( t.p, 0 ), 0, LFO_HZ_MAX );
  int depth = 0, delay = 0, wave = 0;
  if( next_arg( t.p ) ) depth = read_uint( t.p, 0 );
  if( next_arg( t.p ) ) delay = read_uint( t.p, 0 );
  if( next_arg( t.p ) ) wave  = read_uint( t.p, 0 );

  L.depth = (u16)clampi( depth, 0, depth_max );
  L.delay = (u16)( clampi( delay, 0, LFO_DELAY_MAX ) * TICKS_PER_SEC / 1000 );
  L.wave  = (u8)clampi( wave, 0, 3 );
  L.inc   = ( hz > 0 && L.depth > 0 ) ? lfo_inc( hz ) : 0;
  L.phase = 0;
}

// Consume commands until something that takes time (a note, a rest or a tie)
// starts. Returns false once the track has finished for good.
bool parse_next( MmlTrack& t, int idx ){
  int wraps = 0;

  for(;;){
    char c = *t.p;

    if( c == 0 ){
      t.rep_sp = 0;                       // tolerate an unbalanced '['
      // The second wrap without a single note means the string has no playable
      // content at all -- stop instead of spinning forever on it.
      if( g_bgm_loop && ++wraps < 2 ){ t.p = t.start; continue; }
      note_off( t, idx );
      t.sounding = false;
      t.active   = false;
      return false;
    }

    ++t.p;

    if( c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '|' ) continue;
    if( c >= 'A' && c <= 'Z' ) c = (char)( c - 'A' + 'a' );

    switch( c ){
      case 't': g_tempo  = clampi( read_uint( t.p, 120 ), 20, 300 ); continue;
      case 'o': t.octave = clampi( read_uint( t.p,   4 ),  1,   7 ); continue;
      case '>': if( t.octave < 7 ) ++t.octave; continue;
      case '<': if( t.octave > 1 ) --t.octave; continue;
      case 'l': t.deflen = norm_len( read_uint( t.p, 8 ) );          continue;
      case 'v': t.vol    = clampi( read_uint( t.p, 10 ), 0, 15 );    continue;
      case 'q': t.gate   = clampi( read_uint( t.p,  6 ), 1,  8 );    continue;
      case 'k': t.detune = clampi( read_int ( t.p,  0 ), -2400, 2400 ); continue;

      // The 'm' family: everything that moves inside a note.
      case 'm': {
        char k = *t.p;
        if( k >= 'A' && k <= 'Z' ) k = (char)( k - 'A' + 'a' );
        if( k ) ++t.p;
        switch( k ){
          case 'p': read_lfo( t, t.plfo, LFO_CENTS_MAX ); break;
          case 'v': read_lfo( t, t.vlfo, 100 );           break;
          case 'e':
            t.decay     = clampi( read_uint( t.p, DEF_DECAY ), 0, 15 );
            t.attack_ms = next_arg( t.p ) ? clampi( read_uint( t.p, 0 ), 0, 2000 ) : 0;
            break;
          case 's': t.sweep    = clampi( read_int ( t.p, 0 ), -2400, 2400 ); break;
          case 'g': t.glide_ms = clampi( read_uint( t.p, 0 ),     0, 2000 ); break;
          default:  break;                 // an unknown m? is ignored like any other junk
        }
        continue;
      }

      // Absolute note by key number, 48 == A4 440 Hz. The length has to be
      // comma-separated: 'n60' has already eaten every digit that follows.
      case 'n': {
        const int key = clampi( read_uint( t.p, 0 ), 0, 95 );
        int len = t.deflen, dots = 0;
        if( next_arg( t.p ) ) read_len( t, len, dots );
        else while( *t.p == '.' && dots < 3 ){ ++dots; ++t.p; }
        begin_note( t, idx, key, len, dots );
        return true;
      }

      case '@':
        if( *t.p == 'n' || *t.p == 'N' ){ ++t.p; set_noise( t, idx, true ); }
        else { t.wav = clampi( read_uint( t.p, 0 ), 0, 7 ); set_noise( t, idx, false ); }
        continue;

      case '[':
        if( t.rep_sp < MML_REP_DEPTH ){
          t.rep_p[ t.rep_sp ] = t.p;
          t.rep_n[ t.rep_sp ] = -1;       // count is only known at the ']'
          ++t.rep_sp;
        }
        continue;

      case ']': {
        const int n = read_uint( t.p, 2 );
        if( t.rep_sp > 0 ){
          const int sp = t.rep_sp - 1;
          if( t.rep_n[ sp ] < 0 ) t.rep_n[ sp ] = n;
          if( --t.rep_n[ sp ] > 0 ) t.p = t.rep_p[ sp ];
          else                      --t.rep_sp;
        }
        continue;
      }

      case 'r': {
        int len, dots;
        read_len( t, len, dots );
        note_off( t, idx );
        t.sounding = false;
        t.dur_left = len_units( len, dots );
        return true;
      }

      case '^': {
        int len, dots;
        read_len( t, len, dots );
        const int u = len_units( len, dots );
        t.dur_left  = u;
        t.gate_left = u + UNITS_PER_TICK; // hold straight through the tie
        return true;
      }

      default: break;
    }

    int semi = semitone_of( c );
    if( semi < 0 ) continue;              // anything unrecognised is ignored

    if     ( *t.p == '+' || *t.p == '#' ){ ++semi; ++t.p; }
    else if( *t.p == '-'                ){ --semi; ++t.p; }

    int len, dots;
    read_len( t, len, dots );
    begin_note( t, idx, t.octave * 12 + semi - 9, len, dots );  // o4 a == key 48 == A4 440Hz
    return true;
  }
}

void track_advance( MmlTrack& t, int idx ){
  if( !t.active ) return;

  if( t.sounding ){
    t.gate_left -= UNITS_PER_TICK;
    if( t.gate_left <= 0 ){
      note_off( t, idx );
      t.sounding = false;
    } else {
      track_modulate( t, idx );
      ++t.note_ticks;
    }
  }

  t.dur_left -= UNITS_PER_TICK;

  // Zero-length constructs (an empty repeat, a run of commands) must not be
  // able to spin here; 64 commands per tick is far past any real music.
  for( int guard = 0 ; t.dur_left <= 0 && t.active && guard < 64 ; ++guard ){
    const int carry = t.dur_left;         // <= 0: what this note overran by
    if( !parse_next( t, idx ) ) break;
    t.dur_left += carry;                  // ... paid back out of the next one
  }
}

void bgm_silence(){
  for( int i = 0 ; i < BGM_TRACKS ; ++i ){
    b8ApuStopTone( BGM_CH0 + (u32)i );
    g_trk[i] = MmlTrack();
  }
  b8ApuStopNoise( BGM_NOISE_CH );
  g_bgm_on = false;
}

// Caller holds g_lock, and has already run ensure_init().
void bgm_start( const char* t0, const char* t1, const char* t2, const char* t3, bool loop ){
  bgm_silence();

  g_bgm_loop = loop;
  g_tempo    = 120;

  const char* src[ BGM_TRACKS ] = { t0, t1, t2, t3 };
  bool any = false;
  for( int i = 0 ; i < BGM_TRACKS ; ++i ){
    if( !src[i] || !src[i][0] ) continue;
    MmlTrack& t = g_trk[i];
    t.start    = src[i];
    t.p        = src[i];
    t.active   = true;
    t.dur_left = UNITS_PER_TICK;          // first note lands on the next tick
    any = true;
  }
  g_bgm_on = any;
}

// One sequencer step. Caller holds g_lock.
void tick_locked(){
  sfx_tick();

  if( !g_bgm_on ) return;
  bool any = false;
  for( int i = 0 ; i < BGM_TRACKS ; ++i ){
    track_advance( g_trk[i], i );
    if( g_trk[i].active ) any = true;
  }
  if( !any ) g_bgm_on = false;
}

} // namespace

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void sndSfx( SndSfx id ){
  // SndSfx has no negative enumerators, so the compiler is free to give it an
  // unsigned underlying type -- `id < 0` would then be dead code. One unsigned
  // compare rejects both an out-of-range value and a negative one cast in.
  if( (unsigned)id >= (unsigned)SFX_COUNT ) return;
  ensure_init();
  Lock lk;

  const SfxDef* d = &SFX[ id ];

  if( d->kind == 1 ){
    g_sfx_noise.def    = d;
    g_sfx_noise.tick   = 0;
    g_sfx_noise.phase  = 0;
    g_sfx_noise.age    = ++g_age;
    g_sfx_noise.active = true;
    return;
  }

  // Prefer a free voice; otherwise steal the one that has been running longest.
  int pick = -1;
  for( int i = 0 ; i < SFX_VOICES ; ++i ){
    if( !g_sfx[i].active ){ pick = i; break; }
    if( pick < 0 || g_sfx[i].age < g_sfx[pick].age ) pick = i;
  }

  g_sfx[pick].def    = d;
  g_sfx[pick].tick   = 0;
  g_sfx[pick].phase  = 0;
  g_sfx[pick].age    = ++g_age;
  g_sfx[pick].active = true;
}

void sndBgmPlay( const char* t0, const char* t1, const char* t2, const char* t3 ){
  ensure_init();
  Lock lk;
  bgm_start( t0, t1, t2, t3, true );
}

void sndBgmPlayOnce( const char* t0, const char* t1, const char* t2, const char* t3 ){
  ensure_init();
  Lock lk;
  bgm_start( t0, t1, t2, t3, false );
}

void sndBgmStop(){
  if( !g_inited ) return;
  Lock lk;
  bgm_silence();
}

bool sndBgmIsPlaying(){ return g_bgm_on; }

void sndBgmVolume( int pct ){ g_bgm_vol_pct = clampi( pct, 0, 100 ); }
void sndSfxVolume( int pct ){ g_sfx_vol_pct = clampi( pct, 0, 100 ); }

void sndStopAll(){
  if( !g_inited ) return;
  Lock lk;
  bgm_silence();
  for( int i = 0 ; i < SFX_VOICES ; ++i )
    sfx_voice_stop( g_sfx[i], SFX_CH0 + (u32)i, false );
  sfx_voice_stop( g_sfx_noise, SFX_NOISE_CH, true );
}
