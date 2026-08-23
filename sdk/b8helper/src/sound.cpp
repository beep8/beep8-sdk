/**
 * @file sound.cpp
 * @brief Implementation of the BEEP-8 sound helper (see <sound.h>).
 *
 * Layout of the APU, which this file owns end to end:
 *
 *   WSG 0..5   music tracks 0..5      (b8ApuReset() sets MAXCH = 8, so only
 *   WSG 6..7   sound-effect voices     WSG 0..7 are mixed at all)
 *   NOISE 0    sound effects
 *   NOISE 1    music, for '@n' drum tracks
 *
 * Six of the eight tone channels go to the music because that is what the music
 * has to spend them on -- a melody, a counter-melody, a three-note chord and a
 * bass already come to six, and a pair of tracks detuned a few cents apart (see
 * 'k') costs two of them for one voice. Sound effects need far fewer: they are
 * short, they rarely overlap more than twice, and a third simultaneous blip is
 * better dropped than paid for out of the chord.
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
 * The waveform table is the one piece of the chip that is not per-note state:
 * this file keeps an editable RAM copy of it (g_wav) so that '@w' and
 * sndSetWave() can redefine any of the 16 slots, and points the APU there
 * instead of at the ROM default the reset installs.
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
const u32 BGM_CH0       = 0;   // WSG 0..5
const int BGM_TRACKS    = 6;
const u32 SFX_CH0       = 6;   // WSG 6..7
const int SFX_VOICES    = 2;
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

// ---------------------------------------------------------------------------
// Waveform table
// ---------------------------------------------------------------------------
//
// The APU re-reads its whole waveform table out of RAM every time WAVDATADDR is
// written, so owning a RAM copy of that table is the whole of what it takes to
// let a game define timbres of its own: edit a row, write the register again.
//
// b8ApuReset() points the chip at the ROM default table; ensure_init() copies
// those bytes into g_wav and points it here instead. That happens on the first
// sound a game makes, whether or not it ever defines a waveform, so '@w' and
// sndSetWave() work from the first note on with nothing to initialise -- and
// slots 0..7 still hold the factory tones, so a game that ignores all of this
// sounds exactly as it did before.

const int WAV_SLOTS   = B8_APU_NUM_WAVTYP;       // 16 waveforms ...
const int WAV_SAMPLES = B8_APU_SAMPLES_PER_WAV;  // ... of 32 4-bit samples each

// The sample value that maps to 0 on the way out (the chip reads 0..15 as
// -8..+7), i.e. the level a waveform is silent at.
const u8 WAV_MID = 8;

u8 g_wav[ WAV_SLOTS ][ WAV_SAMPLES ];

// A 512-byte DMA copy: cheap, but not free and not needed unless something
// actually changed, so every caller below goes through wav_store() first.
void wav_upload(){ b8ApuSetWavtable( &g_wav[0][0] ); }

// Overwrite one slot. Returns whether the bytes actually moved.
bool wav_store( int slot, const u8* samples ){
  if( slot < 0 || slot >= WAV_SLOTS ) return false;
  bool changed = false;
  for( int i = 0 ; i < WAV_SAMPLES ; ++i ){
    const u8 v = (u8)clampi( samples[i], 0, B8_APU_WAV_SAMPLE_MAX );
    if( g_wav[ slot ][ i ] != v ){ g_wav[ slot ][ i ] = v; changed = true; }
  }
  return changed;
}

int hex_digit( char c ){
  if( c >= '0' && c <= '9' ) return c - '0';
  if( c >= 'a' && c <= 'f' ) return c - 'a' + 10;
  if( c >= 'A' && c <= 'F' ) return c - 'A' + 10;
  return -1;
}

// Read a waveform written as hex digits -- one digit per 4-bit sample, so a
// full waveform is exactly 32 of them and the shape is legible as text.
// Whitespace and '|' are skipped, as everywhere else in the MML, so a shape can
// be grouped into readable runs. Reading stops at the first other character, or
// at 32 digits; a short shape is padded with WAV_MID rather than left as noise.
// Returns how many characters were consumed.
int read_hex_wave( const char* s, u8* out ){
  const char* p = s;
  int n = 0;
  for( ; *p && n < WAV_SAMPLES ; ++p ){
    if( *p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == '|' ) continue;
    const int d = hex_digit( *p );
    if( d < 0 ) break;
    out[ n++ ] = (u8)d;
  }
  for( int i = n ; i < WAV_SAMPLES ; ++i ) out[i] = WAV_MID;
  return (int)( p - s );
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

  // Take the waveform table over from the ROM copy b8ApuReset() just installed,
  // so that a slot can be redefined later without resetting the chip.
  for( int w = 0 ; w < WAV_SLOTS ; ++w )
    for( int i = 0 ; i < WAV_SAMPLES ; ++i )
      g_wav[w][i] = b8ApuDefaultWavtable[w][i];
  wav_upload();

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
  unsigned char  wav;     // tone waveform, 0..7 (see the note above the table)
  unsigned char  mode;    // 0 = glide hz0->hz1 | 1 = step at the half | 2 = step at the thirds
  unsigned char  ticks;   // total duration, in 1/120 s ticks
  unsigned short hz0, hz1, hz2;   // hz2 is only read in mode 2
  unsigned short v0, v1;  // tone: CHVOL 0..4095 | noise: attenuation shift 0..16
  // One LFO per preset, feeding both depths at once -- a wobble is nearly
  // always wanted on the pitch and the volume together, and sharing the phase
  // keeps the table to four extra columns.
  unsigned short lpd;     //   pitch depth, in cents (an octave is 1200)
  unsigned char  lhz;     // LFO speed in Hz; 0 = no LFO at all
  unsigned char  lvd;     //   volume depth, in percent (tones only -- see below)
  unsigned char  lsh;     //   shape: 0 sine, 1 triangle, 2 square, 3 falling ramp
  // A preset may be two voices rather than one: 'lay' names a second def in
  // SFX_LAYER below (1-based, 0 = this is a single voice), fired at the same
  // moment as this one and 'dly' ticks after it. That is how a blast gets a
  // pitched thump under its noise body, and how a fanfare gets a harmony --
  // see the comment above SFX_LAYER.
  unsigned char  dly;     // start delay, in ticks (layers only; 0 on a main def)
  unsigned char  lay;     // 1 + index into SFX_LAYER, or 0
};

// The tone presets pick their timbre out of waveform slots 0..7, the factory
// eight (0 square, 1 saw, 2 rough, 3 double-lobed, 4 spiky sine, 5 soft,
// 6 metallic, 7 stepped square). Slots 8..15 are deliberately never touched:
// those belong to the game, and an effect that reached into them would change
// its own voice the moment the game defined an instrument of its own.
//
// Noise pitch is worth one warning. NFREQ clocks a shift register, and the
// chip only reloads that register when the accumulator's top half rolls over,
// so the hz written here comes out as 32x that many steps a second -- which
// saturates at the 24 kHz output rate once hz passes ~750. Everything above
// that is the same white noise; the range that actually *sounds* different is
// 20..750, and that is where the low, heavy presets below live.
//
// Noise level needs the same care from the other end. The generator's output is
// a full-scale 16-bit number shifted right by NCHVOLDIV, so a noise preset at
// shift 0 is already the whole of the mixer's headroom on its own: the presets
// that carry a tone layer sit at shift 1 instead, which is what leaves room for
// that layer to be heard rather than clamped away.

// Indexed by SndSfx -- keep in the same order as that enum, which is grouped by
// the kind of game event rather than by how the sound is built.
//
//                          kind wav mode  tck    hz0    hz1    hz2     v0    v1   lpd  lhz  lvd  lsh  dly  lay
const SfxDef SFX[ SFX_COUNT ] = {
  // -- player action --
  /* SFX_JUMP       */ {  0,  0,  0,  20,   200,   700,     0,  1800,  300,   15,  10,   0,   1,   0,   0 },
  /* SFX_DOUBLEJUMP */ {  0,  7,  0,  14,   400,  1200,     0,  1500,  200,   20,  14,   0,   1,   0,   0 },
  /* SFX_LAND       */ {  1,  0,  0,  14,   260,   100,     0,     1,    7,  250,   8,   0,   3,   0,   1 },
  /* SFX_STEP       */ {  1,  0,  0,   8,   700,   450,     0,     1,    7,  200,  12,   0,   2,   0,   0 },
  /* SFX_SWIPE      */ {  1,  0,  0,  22,  1400,   380,     0,     2,    9,  600,   6,   0,   3,   0,   0 },
  /* SFX_DASH       */ {  1,  0,  0,  18,   300,  1400,     0,     3,   10,  400,   9,   0,   1,   0,   2 },
  /* SFX_SHOOT      */ {  0,  1,  0,  16,  1400,   350,     0,  1300,  100,   30,  16,   0,   2,   0,   0 },
  /* SFX_CHARGE     */ {  0,  1,  0,  60,   200,   900,     0,   900, 1500,   30,   7,  25,   0,   0,   3 },

  // -- weapons --
  /* SFX_LASER      */ {  0,  7,  0,  12,  3000,   600,     0,  1600,  100,   40,  18,   0,   2,   0,   4 },
  /* SFX_MISSILE    */ {  1,  0,  0,  30,   200,   900,     0,     4,   10,  500,   5,   0,   0,   0,   5 },
  /* SFX_ZAP        */ {  0,  6,  0,  20,  1200,   900,     0,  1500,  200,  120,  20,  60,   2,   0,   6 },
  /* SFX_RELOAD     */ {  1,  0,  0,   6,  1000,   700,     0,     2,    8,    0,   0,   0,   0,   0,   7 },

  // -- impact and destruction --
  /* SFX_HIT        */ {  1,  0,  0,  12,   600,   150,     0,     0,    7,  300,  10,   0,   3,   0,   0 },
  /* SFX_BOUNCE     */ {  0,  0,  0,  14,   660,   440,     0,  1400,  200,   25,  12,   0,   1,   0,   0 },
  /* SFX_BREAK      */ {  1,  0,  0,  28,  1600,   300,     0,     0,    9,  700,  16,   0,   2,   0,   0 },
  /* SFX_BLOCK      */ {  0,  6,  0,  14,  1760,  1400,     0,  1500,  200,   30,  15,   0,   2,   0,   8 },
  /* SFX_CRUSH      */ {  1,  0,  0,  34,   420,    90,     0,     1,    9,  500,  11,   0,   2,   0,   9 },
  /* SFX_DAMAGE     */ {  0,  1,  0,  36,   400,    90,     0,  1800,  200,   60,  13,  40,   1,   0,   0 },
  /* SFX_EXPLODE    */ {  1,  0,  0,  52,   800,   120,     0,     0,   10,  400,   5,   0,   3,   0,   0 },
  /* SFX_BOOM       */ {  1,  0,  0,  70,   300,    45,     0,     1,   11,  500,   4,   0,   3,   0,  10 },
  /* SFX_BLAST      */ {  1,  0,  0,  40,  1000,    55,     0,     1,   10,  900,   7,   0,   3,   0,  11 },
  /* SFX_RUMBLE     */ {  1,  0,  0, 100,   130,    55,     0,     2,   12,  600,   3,   0,   0,   0,  12 },

  // -- pickups and rewards --
  /* SFX_COIN       */ {  0,  0,  1,  16,   988,  1319,     0,  1700,  900,   12,  12,   0,   1,   0,   0 },
  /* SFX_GEM        */ {  0,  7,  2,  22,  1319,  1760,  2637,  1500,  400,   15,  14,  25,   0,   0,  13 },
  /* SFX_HEAL       */ {  0,  3,  0,  44,   587,  1175,     0,  1200,  500,   25,   6,  20,   0,   0,   0 },
  /* SFX_POWERUP    */ {  0,  0,  0,  40,   523,  1568,     0,  1500,  700,   20,  10,  20,   1,   0,   0 },
  /* SFX_LEVELUP    */ {  0,  0,  2,  60,   523,   784,  1047,  1600, 1200,   20,   6,   0,   0,   0,  14 },
  /* SFX_UNLOCK     */ {  0,  5,  2,  24,   440,   659,   880,  1500, 1100,   18,  12,   0,   1,   0,   0 },
  /* SFX_EXTRALIFE  */ {  0,  0,  2,  44,   784,  1047,  1568,  1600, 1200,   15,   8,  20,   0,   0,  15 },

  // -- menus and UI --
  /* SFX_SELECT     */ {  0,  0,  0,  12,   880,   880,     0,  1200,  200,   15,  18,   0,   2,   0,   0 },
  /* SFX_CONFIRM    */ {  0,  0,  1,  20,   880,  1319,     0,  1400,  700,   12,  10,   0,   1,   0,   0 },
  /* SFX_CANCEL     */ {  0,  0,  1,  20,   880,   587,     0,  1400,  500,   12,  10,   0,   1,   0,   0 },
  /* SFX_DENY       */ {  0,  1,  0,  24,   220,   175,     0,  1500,  300,    0,  14,  55,   2,   0,   0 },
  /* SFX_BLIP       */ {  0,  0,  0,   8,  1320,  1320,     0,   900,  100,   25,  20,   0,   2,   0,   0 },
  // TEXT and RELOAD's noise head are the two things here with no LFO at all,
  // and deliberately: at 5 and 6 ticks the fastest LFO the 120 Hz sequencer can
  // run gets one sample of its cycle, which is a one-off pitch offset rather
  // than any kind of movement. What they want instead is to be *identical*
  // every time, since they fire once per character and once per shot.
  /* SFX_TEXT       */ {  0,  7,  0,   5,  1760,  1600,     0,   800,    0,    0,   0,   0,   0,   0,   0 },
  /* SFX_PAUSE      */ {  0,  3,  1,  24,  1047,   698,     0,  1400,  600,   10,   8,  35,   0,   0,   0 },

  // -- game state --
  /* SFX_ALARM      */ {  0,  0,  2,  48,   880,   587,   880,  1500, 1200,    0,   9,  45,   2,   0,   0 },
  /* SFX_COUNTDOWN  */ {  0,  0,  0,  18,   880,   880,     0,  1500,  300,    0,   6,  15,   0,   0,   0 },
  /* SFX_START      */ {  0,  0,  1,  30,  1047,  1568,     0,  1700,  900,   15,   9,  20,   1,   0,  16 },
  /* SFX_CLEAR      */ {  0,  0,  1,  48,   784,  1568,     0,  1600, 1200,   20,   6,  20,   0,   0,   0 },
  /* SFX_VICTORY    */ {  0,  0,  2,  90,   784,  1047,  1568,  1700, 1300,   20,   7,  20,   0,   0,  17 },
  /* SFX_GAMEOVER   */ {  0,  2,  0,  80,   392,   131,     0,  1800,  300,   35,   4,  20,   0,   0,   0 },

  // -- environment --
  /* SFX_SPLASH     */ {  1,  0,  0,  24,  1200,   320,     0,     1,    9,  800,   7,   0,   3,   0,   0 },
  /* SFX_BUBBLE     */ {  0,  4,  0,  14,   300,  1000,     0,  1300,    0,   50,  16,   0,   1,   0,   0 },
  /* SFX_WIND       */ {  1,  0,  0,  90,   700,   500,     0,     3,   10,  900,   2,   0,   0,   0,   0 },
  /* SFX_FIRE       */ {  1,  0,  0,  60,   800,   600,     0,     3,    9, 1200,  17,   0,   2,   0,  18 },
  /* SFX_DOOR       */ {  1,  0,  0,  40,   260,   140,     0,     3,   10,  400,   4,   0,   0,   0,  19 },
  /* SFX_ENGINE     */ {  0,  1,  0,  60,    95,   115,     0,  1500,  900,   90,  19,  45,   2,   0,  20 },
  /* SFX_MAGIC      */ {  0,  4,  2,  40,  1568,  2093,  2637,  1400,  300,   80,  15,  30,   0,   0,  21 },
  /* SFX_WARP       */ {  0,  4,  0,  36,   300,  2200,     0,  1400,  400,   70,  15,  30,   0,   0,   0 },
};

// The second voice of the presets above, in the order those presets reference
// it -- 'lay' is 1 + an index into here. A layer is an ordinary SfxDef with two
// rules: its own 'lay' is always 0 (layers do not nest), and it never picks the
// noise generator when its parent already has, because there is only one of
// those. In practice a layer is one of three things -- the pitched body under a
// noise hit, the noise texture over a pitched one, or the same line an octave
// or a third away, started a couple of ticks late so the two arrive as one
// thicker voice rather than as a chorus.
//
//                          kind wav mode  tck    hz0    hz1    hz2     v0    v1   lpd  lhz  lvd  lsh  dly  lay
const SfxDef SFX_LAYER[] = {
  /*  1 LAND      */   {  0,  3,  0,  12,   130,    60,     0,  1700,    0,    0,   0,   0,   0,   0,   0 },
  /*  2 DASH      */   {  0,  1,  0,  16,   300,   900,     0,  1000,    0,   25,   7,  30,   0,   0,   0 },
  /*  3 CHARGE    */   {  0,  3,  0,  54,   202,   906,     0,   500,  900,   20,   5,  20,   1,   6,   0 },
  /*  4 LASER     */   {  0,  0,  0,  10,  4000,   900,     0,   800,    0,   60,  18,   0,   2,   2,   0 },
  /*  5 MISSILE   */   {  0,  1,  0,  28,   150,   420,     0,  1100,  200,   35,   8,  35,   0,   0,   0 },
  /*  6 ZAP       */   {  1,  0,  0,  18,   700,   400,     0,     3,   10,  900,  20,   0,   2,   0,   0 },
  /*  7 RELOAD    */   {  0,  6,  0,  10,  1600,  1500,     0,  1400,  100,   20,  14,   0,   2,   7,   0 },
  /*  8 BLOCK     */   {  0,  7,  0,  12,  2640,  2100,     0,   700,    0,   40,  15,   0,   2,   1,   0 },
  /*  9 CRUSH     */   {  0,  2,  0,  30,   120,    58,     0,  1500,    0,   40,   6,  30,   1,   0,   0 },
  /* 10 BOOM      */   {  0,  0,  0,  60,   130,    55,     0,  1900,    0,   60,   5,  40,   3,   0,   0 },
  /* 11 BLAST     */   {  0,  7,  0,  24,   320,    60,     0,  1700,    0,   70,   9,  45,   3,   0,   0 },
  /* 12 RUMBLE    */   {  0,  2,  0,  90,    82,    58,     0,  1200,    0,   90,   2,  55,   0,   4,   0 },
  /* 13 GEM       */   {  0,  4,  2,  20,  2637,  3520,  5274,   600,    0,   20,  14,  30,   0,   2,   0 },
  /* 14 LEVELUP   */   {  0,  3,  2,  56,   659,   988,  1319,   700,  500,   25,   6,  20,   0,   4,   0 },
  /* 15 EXTRALIFE */   {  0,  7,  2,  40,  1568,  2093,  3136,   600,  400,   20,   8,  25,   0,   4,   0 },
  /* 16 START     */   {  0,  7,  1,  28,  2093,  3136,     0,   700,  300,   20,   9,  25,   1,   2,   0 },
  /* 17 VICTORY   */   {  0,  3,  2,  86,   523,   659,  1047,   900,  700,   25,   5,  25,   0,   4,   0 },
  /* 18 FIRE      */   {  0,  2,  0,  56,   160,   120,     0,   700,    0,   80,   9,  50,   3,   0,   0 },
  /* 19 DOOR      */   {  0,  5,  0,  36,   150,    90,     0,  1100,    0,   50,   5,  40,   1,   0,   0 },
  /* 20 ENGINE    */   {  1,  0,  0,  58,   260,   300,     0,     6,    9,  500,  19,   0,   2,   0,   0 },
  /* 21 MAGIC     */   {  0,  7,  2,  36,  2349,  3136,  3951,   600,    0,  100,  18,  35,   1,   5,   0 },
};

// Nothing ties a preset's 'lay' to the length of the table above it, so this is
// what a row that names a layer nobody wrote costs: a dropped second voice
// rather than a wild FREQ read out of whatever rodata follows.
const unsigned SFX_LAYERS = sizeof( SFX_LAYER ) / sizeof( SFX_LAYER[0] );

struct SfxVoice {
  const SfxDef* def   = 0;
  int           tick  = 0;   // negative while the voice is still in its 'dly'
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
  // A layer starts life with a negative tick count -- that is its 'dly'. It
  // holds the channel from the moment it is fired (so a following effect steals
  // some other voice, not this one) but writes nothing to the chip until it
  // actually begins, which is what keeps the delayed half silent rather than
  // sounding the tail of whatever was there before.
  if( v.tick < 0 ){ ++v.tick; return; }
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
    lfo     = lfo_sample    ( d->lsh, v.phase );
    lfo_v   = lfo_vol_sample( d->lsh, v.phase );
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

// Hand one def to the voice that should carry it. There is exactly one noise
// generator on the effects side, so a noise def always lands on it; a tone def
// prefers a free voice and otherwise steals the one that has been running
// longest. Fired twice in a row for a layered preset, the second call cannot
// pick the voice the first just took (it is now the youngest), which is what
// puts a two-tone preset on both voices without any bookkeeping.
void sfx_fire( const SfxDef* d ){
  SfxVoice* v;
  if( d->kind == 1 ) v = &g_sfx_noise;
  else {
    int pick = -1;
    for( int i = 0 ; i < SFX_VOICES ; ++i ){
      if( !g_sfx[i].active ){ pick = i; break; }
      if( pick < 0 || g_sfx[i].age < g_sfx[pick].age ) pick = i;
    }
    v = &g_sfx[pick];
  }
  v->def    = d;
  v->tick   = -(int)d->dly;
  v->phase  = 0;
  v->age    = ++g_age;
  v->active = true;
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
  // Roughly perceptual: v=15 -> 2025, v=10 -> 900, v=8 -> 576. The mixer sums
  // every channel as sample(-8..+7) * CHVOL and clamps the total to int16, so
  // the headroom that matters is how many tracks can line up in phase at once.
  // The curve is scaled for six of them; a piece that only uses three can push
  // its 'v' higher to take the room back.
  const int a = v * v * 9 * g_bgm_vol_pct / 100;
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

// '@w8={0123456789abcdeffedcba9876543210}' -- define waveform slot 8 in place,
// from inside the music itself, so that a piece carries its own timbres instead
// of needing a matching sndSetWave() call next to it.
//
// The braces are not decoration: hex digits and note names share the letters
// a-f, so a miscounted shape without them would silently eat the notes that
// follow it. With them, a malformed definition costs only itself.
void read_wavedef( MmlTrack& t ){
  const int slot = clampi( read_uint( t.p, 0 ), 0, WAV_SLOTS - 1 );
  while( *t.p == ' ' || *t.p == '\t' || *t.p == '=' ) ++t.p;
  if( *t.p != '{' ) return;                 // malformed: ignored, like any junk
  ++t.p;

  u8 shape[ WAV_SAMPLES ];
  t.p += read_hex_wave( t.p, shape );
  while( *t.p && *t.p != '}' ) ++t.p;       // anything past the 32nd digit
  if( *t.p == '}' ) ++t.p;

  // A looping track re-reads its own '@w' on every wrap; wav_store() reports
  // that nothing moved, and the DMA is skipped.
  if( wav_store( slot, shape ) ) wav_upload();
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
        if     ( *t.p == 'n' || *t.p == 'N' ){ ++t.p; set_noise( t, idx, true ); }
        else if( *t.p == 'w' || *t.p == 'W' ){ ++t.p; read_wavedef( t ); }
        else { t.wav = clampi( read_uint( t.p, 0 ), 0, WAV_SLOTS - 1 ); set_noise( t, idx, false ); }
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
void bgm_start( const char* const* src, bool loop ){
  bgm_silence();

  g_bgm_loop = loop;
  g_tempo    = 120;

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
  // The layer is fired first and the main voice second, so that on a preset
  // whose two halves are both tones the main one is the younger of the pair --
  // and therefore the one the *next* effect steals last. An interrupted blast
  // then loses its harmony rather than its body.
  if( d->lay && d->lay <= SFX_LAYERS ) sfx_fire( &SFX_LAYER[ d->lay - 1 ] );
  sfx_fire( d );
}

void sndBgmPlay( const char* t0, const char* t1, const char* t2,
                 const char* t3, const char* t4, const char* t5 ){
  const char* src[ BGM_TRACKS ] = { t0, t1, t2, t3, t4, t5 };
  ensure_init();
  Lock lk;
  bgm_start( src, true );
}

void sndBgmPlayOnce( const char* t0, const char* t1, const char* t2,
                     const char* t3, const char* t4, const char* t5 ){
  const char* src[ BGM_TRACKS ] = { t0, t1, t2, t3, t4, t5 };
  ensure_init();
  Lock lk;
  bgm_start( src, false );
}

void sndBgmStop(){
  if( !g_inited ) return;
  Lock lk;
  bgm_silence();
}

bool sndBgmIsPlaying(){ return g_bgm_on; }

void sndSetWave( int slot, const char* shape ){
  if( !shape ) return;
  u8 s[ WAV_SAMPLES ];
  read_hex_wave( shape, s );
  ensure_init();
  Lock lk;
  if( wav_store( slot, s ) ) wav_upload();
}

void sndSetWaveData( int slot, const unsigned char* samples ){
  if( !samples ) return;
  ensure_init();
  Lock lk;
  if( wav_store( slot, samples ) ) wav_upload();
}

void sndResetWaves(){
  ensure_init();
  Lock lk;
  bool changed = false;
  for( int w = 0 ; w < WAV_SLOTS ; ++w )
    if( wav_store( w, &b8ApuDefaultWavtable[w][0] ) ) changed = true;
  if( changed ) wav_upload();
}

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
