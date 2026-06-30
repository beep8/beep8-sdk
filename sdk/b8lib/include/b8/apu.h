/**
 * @file apu.h
 * @brief Audio Processing Unit (APU) for the BEEP-8 system.
 *
 * The BEEP-8 APU is a wavetable sound generator (WSG). This header describes
 * the hardware and provides the low-level driver API.
 *
 * The APU is a memory-mapped peripheral at @ref B8_APU_ADDR. It mixes:
 *  - @ref B8_APU_NUM_WSG_CH wavetable channels, each playing one of
 *    @ref B8_APU_NUM_WAVTYP waveforms (each waveform is made of
 *    @ref B8_APU_SAMPLES_PER_WAV 4-bit samples), and
 *  - @ref B8_APU_NUM_NOISE_CH pseudo-random noise channels.
 *
 * It then applies a global reverb (delay + gain) and outputs mono PCM at
 * @ref B8_APU_OUT_FREQ Hz.
 *
 * There are two ways to drive it:
 *  1. Write the memory-mapped registers directly (the @c B8_APU_* macros below).
 *  2. Use the convenience driver functions (@c b8Apu*), which wrap the common
 *     operations: reset, loading the waveform table, per-channel tone/noise,
 *     reverb, pitch conversion, and waiting on the per-step sync IRQ.
 *
 * Pitch model: each WSG channel owns a 32-bit phase accumulator that advances by
 * its @c FREQ value once per output sample. The waveform index is bits 16..20 of
 * the accumulator, so one full waveform (32 samples) is consumed every 2^21 phase
 * units. Therefore:
 *   @code output_hz = FREQ * B8_APU_OUT_FREQ / 2^21 @endcode
 * Use @ref b8ApuHzToFreq / @ref b8ApuGetNoteFreq to compute @c FREQ values.
 *
 * For more detailed information, please refer to the BEEP-8 data sheet.
 */
#pragma once
#include <b8/type.h>
#include <b8/register.h>

#ifdef  __cplusplus
extern  "C" {
#endif

/* ============================================================================
 * Hardware constants
 * ========================================================================== */
#define B8_APU_ADDR              (0xffff9000)  /**< APU base address.            */

#define B8_APU_NUM_WSG_CH        (16)    /**< Number of wavetable channels.      */
#define B8_APU_NUM_NOISE_CH      (2)     /**< Number of noise channels.          */
#define B8_APU_NUM_WAVTYP        (16)    /**< Number of waveform table slots.    */
#define B8_APU_SAMPLES_PER_WAV   (32)    /**< Samples per waveform.              */
#define B8_APU_WAV_SAMPLE_MAX    (15)    /**< Max 4-bit waveform sample value.   */

#define B8_APU_OUT_FREQ          (24000) /**< Output sample rate [Hz].           */
#define B8_APU_PHASE_BITS        (21)    /**< Phase units per full waveform = 1<<21. */

#define B8_APU_CHVOL_MAX         (4095)  /**< Max WSG channel volume (12-bit).   */
#define B8_APU_REVDELAY_MAX      (4095)  /**< Max reverb delay (12-bit).         */
#define B8_APU_REVGAIN_MAX       (255)   /**< Max reverb gain (8-bit).           */
#define B8_APU_NCHVOLDIV_MUTE    (16)    /**< Noise attenuation shift that mutes.*/

/* ============================================================================
 * Register map
 * ========================================================================== */
/** Sync IRQ enable (u1): write 1 to raise @ref B8_IRQ_APUS every APU step.     */
#define B8_APU_INTCTRL    _B8_REG( B8_APU_ADDR + 0x00 )
/** Waveform table address (u32): pointer to a @c u8[16][32] table. Writing this
 *  register DMA-copies the table into the APU; re-write it after editing it.    */
#define B8_APU_WAVDATADDR _B8_REG( B8_APU_ADDR + 0x04 )
/** Global reverb delay (u12, [0..B8_APU_REVDELAY_MAX]).                         */
#define B8_APU_REVDELAY   _B8_REG( B8_APU_ADDR + 0x08 )
/** Global reverb gain (u8, [0..B8_APU_REVGAIN_MAX] -> 0.0..1.0).                */
#define B8_APU_REVGAIN    _B8_REG( B8_APU_ADDR + 0x0c )
/** Number of WSG channels actually mixed (u4, init 8).                          */
#define B8_APU_MAXCH      _B8_REG( B8_APU_ADDR + 0x10 )

/** WSG channel @p n volume (u12, [0..B8_APU_CHVOL_MAX]).                        */
#define B8_APU_CHVOL(n)   _B8_REG( B8_APU_ADDR + 0x20  + (n)*32 + 0 )
/** WSG channel @p n waveform index (u4, [0..B8_APU_NUM_WAVTYP-1]).             */
#define B8_APU_WAVTYP(n)  _B8_REG( B8_APU_ADDR + 0x20  + (n)*32 + 4 )
/** WSG channel @p n phase increment (u32). See @ref b8ApuHzToFreq.             */
#define B8_APU_FREQ(n)    _B8_REG( B8_APU_ADDR + 0x20  + (n)*32 + 8 )

/** Noise channel @p n volume attenuation (u5): output >> NCHVOLDIV. 0 is loud,
 *  @ref B8_APU_NCHVOLDIV_MUTE (or more) is effectively silent.                  */
#define B8_APU_NCHVOLDIV(n) _B8_REG( B8_APU_ADDR + 0x200 + (n)*32 + 0 )
/** Noise channel @p n phase increment (u32).                                   */
#define B8_APU_NFREQ(n)     _B8_REG( B8_APU_ADDR + 0x200 + (n)*32 + 8 )

/* ============================================================================
 * Note numbering (for b8ApuGetNoteFreq)
 * ========================================================================== */
/** Key number of A0 (27.5 Hz); key numbers increase one per semitone.          */
#define B8_APU_KEY_A0   (0)
/** Key number of A4 (440 Hz), the reference pitch.                             */
#define B8_APU_KEY_A4   (48)

/* ============================================================================
 * Low-level driver API
 * ========================================================================== */

/**
 * @brief Reset the APU to a known, silent state.
 *
 * Loads a built-in default waveform table, silences every WSG and noise channel,
 * disables reverb, sets the active channel count to 8, and enables the per-step
 * sync IRQ (@ref B8_IRQ_APUS) so @ref b8ApuSyncWait can be used.
 */
extern void b8ApuReset(void);

/**
 * @brief Load a waveform table into the APU.
 *
 * @param wavtable Pointer to a @c u8[B8_APU_NUM_WAVTYP][B8_APU_SAMPLES_PER_WAV]
 *                 table; each sample is 4-bit (0..B8_APU_WAV_SAMPLE_MAX).
 *
 * The table is DMA-copied immediately. Call again after editing it in place.
 */
extern void b8ApuSetWavtable(const u8 *wavtable);

/**
 * @brief Configure the global reverb.
 *
 * @param delay Reverb delay in samples [0..B8_APU_REVDELAY_MAX].
 * @param gain  Reverb feedback gain [0..B8_APU_REVGAIN_MAX] (0 disables it).
 */
extern void b8ApuSetReverb(u32 delay, u32 gain);

/**
 * @brief Start (or update) a tone on a WSG channel.
 *
 * @param ch     WSG channel [0..B8_APU_NUM_WSG_CH-1].
 * @param wavtyp Waveform index [0..B8_APU_NUM_WAVTYP-1].
 * @param vol    Volume [0..B8_APU_CHVOL_MAX].
 * @param freq   Phase increment (see @ref b8ApuHzToFreq / @ref b8ApuGetNoteFreq).
 */
extern void b8ApuPlayTone(u32 ch, u32 wavtyp, u32 vol, u32 freq);

/**
 * @brief Silence a WSG channel (sets its volume to 0).
 * @param ch WSG channel [0..B8_APU_NUM_WSG_CH-1].
 */
extern void b8ApuStopTone(u32 ch);

/**
 * @brief Start (or update) a noise channel.
 *
 * @param ch     Noise channel [0..B8_APU_NUM_NOISE_CH-1].
 * @param voldiv Attenuation shift [0..B8_APU_NCHVOLDIV_MUTE]; smaller is louder.
 * @param freq   Phase increment (see @ref b8ApuHzToFreq).
 */
extern void b8ApuPlayNoise(u32 ch, u32 voldiv, u32 freq);

/**
 * @brief Silence a noise channel.
 * @param ch Noise channel [0..B8_APU_NUM_NOISE_CH-1].
 */
extern void b8ApuStopNoise(u32 ch);

/**
 * @brief Convert a frequency in Hz to a @c FREQ phase increment.
 * @param hz Desired output frequency in Hz.
 * @return The value to write to @ref B8_APU_FREQ / @ref B8_APU_NFREQ.
 */
extern u32 b8ApuHzToFreq(u32 hz);

/**
 * @brief Convert a musical key number to a @c FREQ phase increment.
 *
 * Key numbers are one-per-semitone with @ref B8_APU_KEY_A0 = A0 (27.5 Hz) and
 * @ref B8_APU_KEY_A4 = A4 (440 Hz).
 *
 * @param keynum Key number (may be 0 or larger; the practical range is ~0..87).
 * @return The value to write to @ref B8_APU_FREQ.
 */
extern u32 b8ApuGetNoteFreq(s16 keynum);

/**
 * @brief Block until the next APU sync interrupt (@ref B8_IRQ_APUS).
 *
 * Requires that the sync IRQ has been enabled (done by @ref b8ApuReset).
 * @return 0 on success; a negative error code on failure.
 */
extern int b8ApuSyncWait(void);

#ifdef  __cplusplus
}
#endif
