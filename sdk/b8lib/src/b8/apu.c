#include <beep8.h>
#include <sys/errno.h>

/*
 * Default waveform table loaded by b8ApuReset().
 * 16 waveforms x 32 samples; each sample is 4-bit (0..15). The first eight are
 * the classic set used by the BEEP-8 sound editor (square, ramp, and several
 * shaped tones); the rest default to silence.
 *
 * Not static: <sound.h>'s helper keeps an editable RAM copy of this table so a
 * game can define waveforms of its own, and starts it from these bytes.
 */
const u8 b8ApuDefaultWavtable[ B8_APU_NUM_WAVTYP ][ B8_APU_SAMPLES_PER_WAV ] = {
  { 14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  {  0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9,10,10,11,11,12,12,13,13,14,14,15,15 },
  {  0, 2, 3, 4, 6, 6, 4, 5, 9,10,10, 8,10, 7,10, 9, 7, 7, 9, 6, 8, 6, 5, 6,10,12,10,10,12,13,14,15 },
  { 11,13,12,13,12,10, 8, 8, 8,10,12,13,14,13,11, 8, 4, 2, 1, 2, 3, 5, 7, 7, 7, 5, 3, 2, 1, 2, 4, 7 },
  {  7,10,12,13,14,13,12, 8, 7, 4, 2, 1, 0, 1, 2, 4, 7,11,13,14,13,11, 7, 3, 1, 0, 1, 3, 7,14, 7, 0 },
  { 10,12,12,10, 7, 7, 8,11,13,14,13,10, 6, 5, 5, 7, 9, 9, 8, 4, 1, 0, 1, 3, 6, 7, 7, 4, 2, 2, 4, 7 },
  {  7,14,12, 9,12,14,10, 7,12,15,13, 8,10,11, 7, 2, 8,13, 9, 4, 5, 7, 2, 0, 3, 8, 5, 1, 3, 6, 3, 1 },
  { 14,14, 7, 7,14,14, 7, 7,14,14, 7, 7,14,14, 7, 7, 0, 0, 7, 7, 0, 0, 7, 7, 0, 0, 7, 7, 0, 0, 7, 7 },
};

void b8ApuSetWavtable( const u8 *wavtable ){
  B8_APU_WAVDATADDR = (u32)(uintptr_t)wavtable;
}

void b8ApuSetReverb( u32 delay, u32 gain ){
  B8_APU_REVDELAY = delay & B8_APU_REVDELAY_MAX;
  B8_APU_REVGAIN  = gain  & B8_APU_REVGAIN_MAX;
}

void b8ApuReset( void ){
  for( u32 ch = 0 ; ch < B8_APU_NUM_WSG_CH ; ++ch ){
    B8_APU_CHVOL( ch )  = 0;
    B8_APU_WAVTYP( ch ) = 0;
    B8_APU_FREQ( ch )   = 0;
  }
  for( u32 ch = 0 ; ch < B8_APU_NUM_NOISE_CH ; ++ch ){
    B8_APU_NCHVOLDIV( ch ) = B8_APU_NCHVOLDIV_MUTE;
    B8_APU_NFREQ( ch )     = 0;
  }

  b8ApuSetWavtable( &b8ApuDefaultWavtable[0][0] );
  b8ApuSetReverb( 0, 0 );
  B8_APU_MAXCH = 8;

  b8SysSetupIrqWait( B8_IRQ_APUS );
  B8_APU_INTCTRL = 1;
}

void b8ApuPlayTone( u32 ch, u32 wavtyp, u32 vol, u32 freq ){
  if( ch >= B8_APU_NUM_WSG_CH ) return;
  B8_APU_WAVTYP( ch ) = wavtyp & (B8_APU_NUM_WAVTYP - 1);
  B8_APU_FREQ( ch )   = freq;
  B8_APU_CHVOL( ch )  = vol > B8_APU_CHVOL_MAX ? B8_APU_CHVOL_MAX : vol;
}

void b8ApuStopTone( u32 ch ){
  if( ch >= B8_APU_NUM_WSG_CH ) return;
  B8_APU_CHVOL( ch ) = 0;
}

void b8ApuPlayNoise( u32 ch, u32 voldiv, u32 freq ){
  if( ch >= B8_APU_NUM_NOISE_CH ) return;
  B8_APU_NFREQ( ch )     = freq;
  B8_APU_NCHVOLDIV( ch ) = voldiv > B8_APU_NCHVOLDIV_MUTE ? B8_APU_NCHVOLDIV_MUTE : voldiv;
}

void b8ApuStopNoise( u32 ch ){
  if( ch >= B8_APU_NUM_NOISE_CH ) return;
  B8_APU_NCHVOLDIV( ch ) = B8_APU_NCHVOLDIV_MUTE;
}

u32 b8ApuHzToFreq( u32 hz ){
  /* freq = hz * 2^21 / B8_APU_OUT_FREQ, rounded to nearest. */
  u64 num = (u64)hz << B8_APU_PHASE_BITS;
  return (u32)( ( num + (B8_APU_OUT_FREQ / 2) ) / B8_APU_OUT_FREQ );
}

u32 b8ApuGetNoteFreq( s16 keynum ){
  /* FREQ values for octave 4 (A4..G#5), starting at A4 = 440 Hz. */
  static const u32 tonetbl[ 12 ] = {
    38447, 40719, 43166, 45700, 48409, 51292,
    54351, 57584, 60992, 64662, 68506, 72613
  };
  const s16 base_octave = B8_APU_KEY_A4 / 12;  /* = 4 */
  const s16 octave = keynum / 12;
  const s16 mod    = keynum % 12;
  const u32 tone   = tonetbl[ mod ];

  if( octave < base_octave ){
    return tone >> ( base_octave - octave );
  } else {
    return tone << ( octave - base_octave );
  }
}

int b8ApuSyncWait( void ){
  return b8SysIrqWait( B8_IRQ_APUS );
}
