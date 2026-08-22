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
 * Timing is kept in units of 1/256 of a frame. Note durations rarely divide
 * evenly into whole frames, so every note carries its remainder over into the
 * next one (see track_advance) instead of rounding — otherwise a fast track
 * would drift audibly against a slow one within a few bars.
 *
 * MML is parsed lazily, straight off the caller's string, one note at a time.
 * That is why the strings must outlive playback (documented in sound.h): it
 * buys zero allocation and zero fixed size limit on a piece of music.
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

// Sub-frame timing resolution: one frame == TICK units.
const int TICK = 256;

// The APU raises B8_IRQ_APUS once per APU step, and the hardware steps at twice
// the frame rate (200 samples of the 24kHz output per step). The sequencer
// keeps its timebase in frames, so it wants every second interrupt.
const int SYNCS_PER_TICK = 2;

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

// True once tick_thread is running, i.e. the sequencer has its own clock and a
// manual sndTick() would only double the tempo. See sndTick().
volatile bool g_threaded = false;

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
// the APU itself, one interrupt per 200 generated samples, so a tick is pinned
// to a fixed amount of *audio*, whatever the renderer is doing.
void* tick_thread( void* ){
  // b8ApuReset() already armed the IRQ; ensure_init() muted it again and left
  // switching it on to us, so that no backlog can pile up on the semaphore
  // between the reset and this thread reaching its first wait.
  B8_APU_INTCTRL = 1;

  for(;;){
    bool ok = true;
    for( int i = 0 ; i < SYNCS_PER_TICK ; ++i )
      if( b8ApuSyncWait() < 0 ) ok = false;

    // Nothing recoverable can make the wait fail, but spinning on it at full
    // tilt would take the CPU away from the game. Fall back to a frame's sleep.
    if( !ok ){ usleep( 16666 ); continue; }

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
  if( pthread_create( &pid, &attr, tick_thread, 0 ) == 0 ){
    g_threaded = true;
  } else {
    // Out of TCBs (32 per program). Nothing will advance the sequencer unless
    // the program calls sndTick() itself, so say so rather than sounding like
    // a broken APU.
    fprintf( stderr, "sound: no thread for the sequencer; call sndTick() at 60Hz\n" );
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
// Sound effects
// ---------------------------------------------------------------------------

struct SfxDef {
  unsigned char  kind;    // 0 = tone, 1 = noise
  unsigned char  wav;     // tone waveform
  unsigned char  mode;    // 0 = glide hz0->hz1 | 1 = step at the half | 2 = step at the thirds
  unsigned char  frames;  // total duration
  unsigned short hz0, hz1, hz2;   // hz2 is only read in mode 2
  unsigned short v0, v1;  // tone: CHVOL 0..4095 | noise: attenuation shift 0..16
};

// Indexed by SndSfx — keep in the same order as that enum, which is grouped by the
// kind of game event rather than by how the sound is built.
//
//                         kind wav mode  frm     hz0    hz1    hz2     v0    v1
const SfxDef SFX[ SFX_COUNT ] = {
  // -- player action --
  /* SFX_JUMP     */ {  0,  0,  0,  10,   200,   700,     0,  1800,  300 },
  /* SFX_LAND     */ {  1,  0,  0,   7,   260,   100,     0,     0,    7 },
  /* SFX_STEP     */ {  1,  0,  0,   4,   700,   450,     0,     1,    7 },
  /* SFX_SWIPE    */ {  1,  0,  0,  11,  4000,   500,     0,     2,    9 },
  /* SFX_SHOOT    */ {  0,  1,  0,   8,  1400,   350,     0,  1300,  100 },
  /* SFX_CHARGE   */ {  0,  1,  0,  30,   200,   900,     0,   900, 1500 },

  // -- impact and destruction --
  /* SFX_HIT      */ {  1,  0,  0,   6,   600,   150,     0,     0,    7 },
  /* SFX_BOUNCE   */ {  0,  0,  0,   7,   660,   440,     0,  1400,  200 },
  /* SFX_BREAK    */ {  1,  0,  0,  14,  2200,   500,     0,     0,    9 },
  /* SFX_BLOCK    */ {  0,  6,  0,   7,  1760,  1400,     0,  1500,  200 },
  /* SFX_EXPLODE  */ {  1,  0,  0,  26,   800,   120,     0,     0,   10 },
  /* SFX_DAMAGE   */ {  0,  1,  0,  18,   400,    90,     0,  1800,  200 },

  // -- pickups and rewards --
  /* SFX_COIN     */ {  0,  0,  1,   8,   988,  1319,     0,  1700,  900 },
  /* SFX_HEAL     */ {  0,  3,  0,  22,   587,  1175,     0,  1200,  500 },
  /* SFX_POWERUP  */ {  0,  0,  0,  20,   523,  1568,     0,  1500,  700 },
  /* SFX_LEVELUP  */ {  0,  0,  2,  30,   523,   784,  1047,  1600, 1200 },
  /* SFX_UNLOCK   */ {  0,  5,  2,  12,   440,   659,   880,  1500, 1100 },

  // -- menus and UI --
  /* SFX_SELECT   */ {  0,  0,  0,   6,   880,   880,     0,  1200,  200 },
  /* SFX_CONFIRM  */ {  0,  0,  1,  10,   880,  1319,     0,  1400,  700 },
  /* SFX_CANCEL   */ {  0,  0,  1,  10,   880,   587,     0,  1400,  500 },
  /* SFX_DENY     */ {  0,  1,  0,  12,   220,   175,     0,  1500,  300 },
  /* SFX_BLIP     */ {  0,  0,  0,   4,  1320,  1320,     0,   900,  100 },

  // -- game state --
  /* SFX_ALARM    */ {  0,  0,  2,  24,   880,   587,   880,  1500, 1200 },
  /* SFX_CLEAR    */ {  0,  0,  1,  24,   784,  1568,     0,  1600, 1200 },
  /* SFX_GAMEOVER */ {  0,  2,  0,  40,   392,   131,     0,  1800,  300 },

  // -- environment --
  /* SFX_SPLASH   */ {  1,  0,  0,  12,  3500,   900,     0,     1,    9 },
  /* SFX_WARP     */ {  0,  4,  0,  18,   300,  2200,     0,  1400,  400 },
};
struct SfxVoice {
  const SfxDef* def   = 0;
  int           frame = 0;
  unsigned      age   = 0;
  bool          active= false;
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
  if( v.frame >= d->frames ){ sfx_voice_stop( v, ch, noise ); return; }

  const int i  = v.frame;
  const int n  = d->frames;
  int hz;
  if     ( d->mode == 1 ) hz = ( i * 2 < n ) ? d->hz0 : d->hz1;
  else if( d->mode == 2 ) hz = ( i * 3 < n ) ? d->hz0 : ( ( i * 3 < n * 2 ) ? d->hz1 : d->hz2 );
  else                    hz = lerpi( d->hz0, d->hz1, i, n );

  if( noise ){
    int vd = lerpi( d->v0, d->v1, i, n ) + noise_trim( g_sfx_vol_pct );
    b8ApuPlayNoise( ch, (u32)clampi( vd, 0, B8_APU_NCHVOLDIV_MUTE ), b8ApuHzToFreq( (u32)hz ) );
  } else {
    const int vol = lerpi( d->v0, d->v1, i, n ) * g_sfx_vol_pct / 100;
    // Pitch and volume move every frame, the waveform never does — set it on
    // the first frame only (same MMIO-write argument as the BGM decay above).
    if( i == 0 ) b8ApuPlayTone( ch, d->wav, (u32)clampi( vol, 0, B8_APU_CHVOL_MAX ), b8ApuHzToFreq( (u32)hz ) );
    else {
      B8_APU_FREQ ( ch ) = b8ApuHzToFreq( (u32)hz );
      B8_APU_CHVOL( ch ) = (u32)clampi( vol, 0, B8_APU_CHVOL_MAX );
    }
  }
  ++v.frame;
}

void sfx_tick(){
  for( int i = 0 ; i < SFX_VOICES ; ++i )
    sfx_voice_step( g_sfx[i], SFX_CH0 + (u32)i, false );
  sfx_voice_step( g_sfx_noise, SFX_NOISE_CH, true );
}

// ---------------------------------------------------------------------------
// MML sequencer
// ---------------------------------------------------------------------------

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
  int   dur_left      = 0;   // 1/256-frame units until the next command
  int   gate_left     = 0;   // 1/256-frame units until note-off
  bool  sounding      = false;
  int   cur_vol       = 0;
  u32   cur_freq      = 0;
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
  t.cur_freq = b8ApuGetNoteFreq( (s16)keynum );
  if( t.noise ){
    // Same pitch value drives the noise clock, so low octaves read as kicks and
    // high ones as hats.
    const int vd = clampi( B8_APU_NCHVOLDIV_MUTE - t.vol + noise_trim( g_bgm_vol_pct ),
                           0, B8_APU_NCHVOLDIV_MUTE );
    b8ApuPlayNoise( BGM_NOISE_CH, (u32)vd, t.cur_freq );
  } else {
    t.cur_vol = chvol( t.vol );
    b8ApuPlayTone( BGM_CH0 + (u32)idx, (u32)t.wav, (u32)t.cur_vol, t.cur_freq );
  }
  t.sounding = true;
}

void set_noise( MmlTrack& t, int idx, bool on ){
  if( t.noise == on ) return;
  if( t.sounding ){ note_off( t, idx ); t.sounding = false; }
  else             note_off( t, idx );   // silence whichever channel it just left
  t.noise = on;
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

int norm_len( int n ){ return clampi( n, 1, 64 ); }

// Length of one note in 1/256-frame units. 60fps * (60/BPM) sec per beat * 4
// beats per whole note == 14400/BPM frames per whole note.
int len_units( int len, int dots ){
  int u   = 3686400 / ( g_tempo * norm_len( len ) );
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
        t.gate_left = u + TICK;           // hold straight through the tie
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
    const int u = len_units( len, dots );
    t.dur_left  = u;
    t.gate_left = ( t.gate >= 8 ) ? ( u + TICK ) : ( u * t.gate / 8 );
    note_on( t, idx, t.octave * 12 + semi - 9 );   // o4 a == key 48 == A4 440Hz
    return true;
  }
}

void track_advance( MmlTrack& t, int idx ){
  if( !t.active ) return;

  if( t.sounding ){
    t.gate_left -= TICK;
    if( t.gate_left <= 0 ){
      note_off( t, idx );
      t.sounding = false;
    } else if( !t.noise ){
      // A flat volume for the whole note reads as an organ; bleeding a few
      // percent per frame is enough to make it read as plucked.
      //
      // Only the volume moves here, so poke CHVOL directly rather than going
      // through b8ApuPlayTone(), which would rewrite WAVTYP and FREQ with the
      // values they already hold. This runs for every sounding track on every
      // frame, and each MMIO write costs a bus dispatch in the emulator, so
      // that is 4 writes per frame instead of 12 with four tracks playing.
      t.cur_vol -= t.cur_vol >> 5;
      B8_APU_CHVOL( BGM_CH0 + (u32)idx ) = (u32)t.cur_vol;
    }
  }

  t.dur_left -= TICK;

  // Zero-length constructs (an empty repeat, a run of commands) must not be
  // able to spin here; 64 commands per frame is far past any real music.
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
    t.dur_left = TICK;                    // first note lands on the next tick
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
    g_sfx_noise.frame  = 0;
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
  g_sfx[pick].frame  = 0;
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

void sndTick(){
  // The tick thread owns the clock whenever it is running; ticking again from
  // the caller's loop would simply run the music at double speed.
  if( !g_inited || g_threaded ) return;
  Lock lk;
  tick_locked();
}
