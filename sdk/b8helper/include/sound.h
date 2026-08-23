#pragma once
/**
 * @file sound.h
 * @brief BEEP-8 sound helper — preset sound effects + MML background music.
 *
 * This is the *game-facing* sound API. It owns the whole APU: once you use it,
 * do not touch @c b8Apu* or the @c B8_APU_* registers yourself, or the two will
 * fight over channels. (The low-level driver in <b8/apu.h> is still there for
 * tools like the sound editor that want raw register control.)
 *
 * Two things only:
 *
 *   - @ref sndSfx — fire-and-forget preset sound effects. One call, no state to
 *     keep, no per-frame bookkeeping.
 *   - @ref sndBgmPlay — background music written as MML text (the classic
 *     "t120 o4 l8 cdefg" note-string notation), up to 4 simultaneous tracks,
 *     each with its own vibrato, tremolo, envelope, sweep and portamento.
 *
 * Nothing needs to be initialised and nothing needs to be ticked. The first
 * call that actually asks for a sound resets the APU and starts a small
 * sequencer thread; from then on the music is clocked by the APU itself, one
 * tick per sync interrupt at 120 Hz, with no connection to the game loop at
 * all — so a heavy frame cannot drag the tempo with it, and the frame rate
 * does not have to be 60 to keep time. A game that never calls into this
 * header never touches the APU and never spawns the thread.
 *
 * That thread is the only other thread in the picture, and every entry point
 * below is safe to call while it runs; you do not need a lock of your own.
 *
 * @code
 *   void _init() override {
 *     sndBgmPlay("t130 o4 l8 v11 q7 mp6,35,120 [ceg>c<]2 [dfa>d<]2",  // melody, with vibrato
 *                "t130 o2 l4 v10 me2 [c c]2 [d d]2",                  // bass, long decay
 *                "@n t130 o4 l8 v9 [c r c c]4");                      // drums (noise)
 *   }
 *   void _update() override {
 *     if( btnp(BUTTON_O) ) sndSfx(SFX_JUMP);
 *   }
 * @endcode
 *
 * @note Browsers only start audio after the first tap/click on the page
 *       (autoplay policy), so the opening moment of a game may be silent no
 *       matter what this API does. That is normal — showing a "TAP TO START"
 *       hint until the first input is the usual fix.
 */

/**
 * @brief Preset sound effects for @ref sndSfx.
 *
 * Each one is a short, self-contained blip — pitch sweep, volume envelope and
 * duration are all baked in, so the same event always sounds the same in every
 * game. Pick the one whose *name* matches the game event; do not try to build
 * effects out of several of these at once.
 */
enum SndSfx {
  // -- player action --
  SFX_JUMP,      /**< Short upward chirp — jumping, hopping, springs.        */
  SFX_LAND,      /**< Low thud — landing, something heavy settling.          */
  SFX_STEP,      /**< Soft noise scuff — footsteps, rustle.                  */
  SFX_SWIPE,     /**< Airy whoosh — dashing, a melee swing, a dodge.         */
  SFX_SHOOT,     /**< Descending zap — firing a shot, lasers.                */
  SFX_CHARGE,    /**< Long rising hum — charging a shot or a jump.           */

  // -- impact and destruction --
  SFX_HIT,       /**< Dry noise tick — bullet connects, small impact.        */
  SFX_BOUNCE,    /**< Short round blip — ball off a wall or paddle.          */
  SFX_BREAK,     /**< Bright noise burst — bricks, glass, crates.            */
  SFX_BLOCK,     /**< Metallic clink — a parry, a shield, a hit that failed. */
  SFX_EXPLODE,   /**< Long noise burst — explosions, destruction, death.     */
  SFX_DAMAGE,    /**< Falling buzz — the player taking damage.               */

  // -- pickups and rewards --
  SFX_COIN,      /**< Two-note ping — coins, pickups, score.                 */
  SFX_HEAL,      /**< Soft rise — healing, recovery, a shield going up.      */
  SFX_POWERUP,   /**< Rising sweep — power-ups, weapon upgrades.             */
  SFX_LEVELUP,   /**< Three-note rise — levelling up, a milestone.           */
  SFX_UNLOCK,    /**< Bright three-note — a door, a switch, something opening.*/

  // -- menus and UI --
  SFX_SELECT,    /**< Tiny click — moving the cursor between menu items.     */
  SFX_CONFIRM,   /**< Two-note rise — accepting, starting, OK.               */
  SFX_CANCEL,    /**< Two-note fall — backing out of a menu.                 */
  SFX_DENY,      /**< Low buzz — an action that is not allowed right now.    */
  SFX_BLIP,      /**< Very short neutral blip — text advance, ticks.         */

  // -- game state --
  SFX_ALARM,     /**< Alternating two-tone — warning, low health, timer low. */
  SFX_CLEAR,     /**< Rising two-note fanfare — stage clear, success.        */
  SFX_GAMEOVER,  /**< Slow descending tone — game over.                      */

  // -- environment --
  SFX_SPLASH,    /**< Wet noise wash — water, liquid.                        */
  SFX_WARP,      /**< Fast rising sweep — teleport, warp, entering a pipe.   */

  SFX_COUNT      /**< Number of presets (not a sound).                       */
};

/**
 * @brief Play a preset sound effect once.
 *
 * Fire-and-forget: call it on the frame the event happens and forget about it.
 * Safe to call every frame if you want (it retriggers), safe to call while
 * music is playing, and safe to call more often than there are voices — the
 * oldest effect is dropped to make room.
 *
 * @param id  One of @ref SndSfx.
 */
void sndSfx( SndSfx id );

/**
 * @brief Start background music from MML text, looping forever.
 *
 * Up to four tracks play simultaneously; pass @c nullptr (or just omit) for the
 * ones you don't need. Any previously playing music is replaced.
 *
 * @warning The strings are read as the music plays, so they must stay alive for
 *          as long as the music does. String literals (the normal case) always
 *          satisfy this; a local @c char[] buffer does not.
 *
 * ### MML syntax
 *
 * Whitespace and @c | are ignored, so you can lay music out readably.
 * Letters may be upper or lower case.
 *
 * | Command      | Meaning                                                       |
 * |--------------|---------------------------------------------------------------|
 * | @c t120      | Tempo in BPM, 20–300 (global — the last track to say it wins). Default 120. |
 * | @c o4        | Octave, 1–7. @c o4&nbsp;c is middle C. Default 4.              |
 * | @c &gt; / @c &lt; | Octave up / down by one.                                 |
 * | @c l8        | Default note length: 1, 2, 4, 8, 16, 32 (whole … 32nd). Default 8. |
 * | @c v10       | Volume, 0–15. Default 10.                                     |
 * | @c q6        | Gate time, 1–8: the note sounds for q/8 of its length, the rest is a gap. 8 = fully legato. Default 6. |
 * | @c \@0       | Waveform, 0–7. 0 = pulse/square, 1 = saw, 2–7 = shaped tones. Default 0. |
 * | @c \@n       | Switch this track to the noise generator — use it for drums and hats. |
 * | @c cdefgab   | A note. Optionally followed by @c + or @c # (sharp), @c - (flat), then a length (@c c16), then dots (@c c4.). |
 * | @c r         | A rest, with the same optional length (@c r4).                 |
 * | @c ^         | Tie: extend the previous note by another length (@c c4^8).     |
 * | @c [ … ]4    | Repeat the bracketed part 4 times (2 if the count is omitted). Nests up to 4 deep. |
 * | @c n60       | A note by absolute key number, 0–95 (48 = @c o4&nbsp;a = A4 440 Hz). A length needs a comma: @c n60,16. |
 *
 * ### Shaping the sound
 *
 * These are per-track and stay set until changed, so they normally sit at the
 * head of a track — though changing one mid-track is how a voice changes
 * character partway through a piece.
 *
 * | Command       | Meaning                                                      |
 * |---------------|--------------------------------------------------------------|
 * | @c mp6,40     | Vibrato: 6 Hz, ±40 cents. Full form @c mp&lt;hz&gt;,&lt;cents&gt;,&lt;delay_ms&gt;,&lt;shape&gt; — the last two are optional. @c mp0 turns it off. |
 * | @c mv5,50     | Tremolo: 5 Hz, dipping 50 % at the bottom of each cycle. Same four arguments. @c mv0 turns it off. |
 * | @c me4        | Volume envelope decay, 0–15. @c me0 holds the level for the whole note (organ, pad); higher decays faster, @c me12 is a hard pluck. Default 6. |
 * | @c me0,300    | … with a 300 ms fade-in: @c me&lt;decay&gt;,&lt;attack_ms&gt;. |
 * | @c ms-600     | Pitch sweep, in cents per second, from the start of every note. Negative falls, positive rises. @c ms0 off. |
 * | @c mg80       | Portamento: every note slides in from the previous one over 80 ms. @c mg0 off. |
 * | @c k-8        | Detune the whole track by ±cents (−2400…2400). Two tracks a few cents apart read as one thick voice. |
 *
 * LFO shapes are @c 0 sine (the default), @c 1 triangle, @c 2 square — that is
 * a trill rather than a waver — and @c 3 a falling ramp.
 *
 * Both LFOs are sampled on the sequencer's own 120 Hz tick, so 1–10 Hz is the
 * range that sounds like a smooth waver; past that it becomes a deliberately
 * stepped effect, and 20 Hz is the ceiling. On an @c \@n noise track only
 * @c mp does anything: that generator's volume moves in 6 dB steps, far too
 * coarse for a tremolo or an envelope, so a drum keeps the flat level its
 * @c v gave it.
 *
 * @param t0  Track 0 (usually the melody). Required.
 * @param t1  Track 1 (usually bass or harmony), or @c nullptr.
 * @param t2  Track 2, or @c nullptr.
 * @param t3  Track 3 (often @c \@n drums), or @c nullptr.
 */
void sndBgmPlay( const char* t0,
                 const char* t1 = 0,
                 const char* t2 = 0,
                 const char* t3 = 0 );

/**
 * @brief Same as @ref sndBgmPlay, but stops at the end instead of looping.
 *
 * Use this for jingles — stage clear, game over, a fanfare — where the music is
 * a one-shot event rather than a backdrop.
 */
void sndBgmPlayOnce( const char* t0,
                     const char* t1 = 0,
                     const char* t2 = 0,
                     const char* t3 = 0 );

/** @brief Stop the music immediately (sound effects keep working). */
void sndBgmStop();

/** @brief True while music is still playing — useful after @ref sndBgmPlayOnce. */
bool sndBgmIsPlaying();

/**
 * @brief Master volume trim for music, as a percentage (0–100, default 100).
 * @note Applies from the next note on; it does not re-scale notes already sounding.
 */
void sndBgmVolume( int pct );

/** @brief Master volume trim for sound effects, as a percentage (0–100, default 100). */
void sndSfxVolume( int pct );

/** @brief Stop everything — music and all sound effects. */
void sndStopAll();
