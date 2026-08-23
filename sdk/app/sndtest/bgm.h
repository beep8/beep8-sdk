#pragma once
/**
 * @file bgm.h
 * @brief The music sndtest auditions: 49 pieces, six MML tracks each.
 *
 * Every piece is built the same way, so that browsing the table is a way of
 * learning what <sound.h>'s MML can do rather than a wall of note names:
 *
 *   - Six tracks -- a lead, a bass, three chord voices detuned a few cents
 *     apart (see 'k') so the pad beats gently against itself, and a drum track
 *     on the noise generator.  A piece with no drums leaves that track null and
 *     spends nothing on it.
 *   - Three sections, A B C, four bars each -- two in the feature demos at the
 *     end -- played through once and then looped.  A is the tune, B answers it
 *     over a different progression, and C turns somewhere else again, usually
 *     with the lead on another waveform: a section head is the natural place to
 *     change what a voice *is* rather than only what it plays.
 *   - Every track of a piece is exactly the same number of whole notes long.
 *     Each loops back to its own start independently (see sound.cpp's
 *     track_advance), so a track a sixteenth short would walk out of phase
 *     with the rest within a few bars.
 *
 * A section only ever changes a knob its track header also sets: the header is
 * re-read on every loop, and that is what puts section A back the way it
 * started.  Set something in B that the header never mentions and it is still
 * set the second time round A.
 *
 * ### The waveform bank
 *
 * The WAV_* macros below are 32-hex-digit waveforms for '@w' (one digit per
 * sample, 0 the bottom of the wave, f the top, 8 the middle).  Slots 0..7 hold
 * the factory tones that sndSfx builds its presets out of and are left alone;
 * each piece loads the three or four timbres it wants into slots 8..15 in its
 * lead track's header, before any track has played a note.  The bank is a
 * handful of classic shapes -- a triangle, a sine, two pulse widths -- plus
 * additive tones built out of a few harmonics apiece, which is what the
 * wavetable is for and what a plain square cannot do.
 *
 * Everything is laid out on a strict bar grid -- every bar of every track is
 * exactly one whole note -- so a piece can be read down a column as well as
 * along a line.  Edit freely; the one rule is that the six tracks stay the
 * same total length.
 */

// The waveform bank: 32 samples of 4 bits, one hex digit each.
#define WAV_BASSR    "8acefffedccbba998776554432111246"   // fundamental plus a little 2nd: round bass
#define WAV_BELL     "8cbbaabaadfda8798798631366566554"   // inharmonic partials: bell, chime
#define WAV_FIFTH    "8dffdb97555676668aaa9abbb9753113"   // a fifth stacked on top: bright, open
#define WAV_GLASS    "8deeedcdfdcdeeed8322234313432223"   // odd harmonics only: glassy, clarinet-ish
#define WAV_HOLLOW   "fffffffaaaffffff1111111666111111"   // a notched square: woody, muted
#define WAV_METAL    "accd55bbeefa8eb86443bb5522168258"   // high inharmonic partials: clangorous
#define WAV_ORGAN    "8ceeffedeedb98768a98753223211224"   // octaves stacked on the fundamental
#define WAV_PIANO    "8dffeeedca889aa98766788643222113"   // a bright even-harmonic tone
#define WAV_PULSE12  "ffff1111111111111111111111111111"   // 12.5% pulse: thinner, more nasal
#define WAV_PULSE25  "ffffffff111111111111111111111111"   // 25% pulse: the classic chip lead
#define WAV_REED     "8dfdcbbbabbbacdc8434655565554313"   // strong odd harmonics: oboe, reed
#define WAV_SINE     "89bcdeefffeedcb98754322111223457"   // sine: the plainest tone there is
#define WAV_SOFTSAW  "ffffffffeedccba98765443221111111"   // sawtooth with the corner rounded off
#define WAV_STRING   "8dfedccbaaaa99998777766665443213"   // a full harmonic series: bowed strings
#define WAV_SYNC     "fedcba8765432fedca98765432fdcba9"   // a saw running 2.5x: hard-sync buzz
#define WAV_TRI      "1234456789abccdefedccba987654432"   // triangle: soft, hollow, few harmonics
#define WAV_VOX      "8dfeba9aaa99accb8544677666765213"   // a formant peak: vocal, vowel-like

struct BgmDef {
  const char* name;
  const char* t[6];      // lead / bass / three chord voices / drums
};

static const BgmDef BGM_DEFS[] = {
  // SUNRISE -- F major. A I-iii-IV-V, B vi-IV-I-V, C the IV-V-iii-vi royal road
  { "SUNRISE", {
    "@w8={" WAV_TRI "}@w9={" WAV_ORGAN "}@w10={" WAV_BASSR "} "
      "@8 t112 v11 q8 me1 mp5,30,250 "
      /*A*/ "o5 f4. a8 o6 c2 | o5 a4. o6 c8 e2 | o6 d4 c4 o5 a+2 | o6 c4 o5 a4 g2 "
      /*B*/ "o6 d8 e8 f4 d2 | o6 f8 d8 c4 o5 a+2 | o6 c8 o5 a8 f4 a2 | o6 c4. o5 a+8 g2 "
      /*C*/ "@9 v10 mp7,45,120 o6 a+4 a4 g4 f4 | o6 e8 g8 a4 g2 | o6 a4 g4 e4 c4 | "
            "o6 d4 f4 a2 ",
    "@10 t112 v11 q6 me3 "
      /*A*/ "o2 [f4]4 | [a4]4 | [a+4]4 | [c4]4 "
      /*B*/ "o2 [d8]8 | [a+8]8 | [f8]8 | [c8]8 "
      /*C*/ "o2 a+4 o3 d4 f4 o2 c+4 | c4 e4 g4 g+4 | a4 o3 c4 e4 o2 d+4 | d4 f4 a4 a4 ",
    "@9 t112 v6 q8 me0,400 mv3,22 k-8 "
      /*A*/ "o4 a1 | o5 c1 | d1 | e1 "
      /*B*/ "o5 f2 f2 | d2 d2 | o4 a2 a2 | e2 e2 "
      /*C*/ "o5 d4. d4. d4 | e4. e4. e4 | c4. c4. c4 | f4. f4. f4 ",
    "@9 t112 v6 q8 me0,400 mv3,22 k0 "
      /*A*/ "o5 c1 | e1 | f1 | g1 "
      /*B*/ "o4 a2 a2 | f2 f2 | c2 c2 | g2 g2 "
      /*C*/ "o5 f4. f4. f4 | g4. g4. g4 | e4. e4. e4 | a4. a4. a4 ",
    "@9 t112 v6 q8 me0,400 mv3,22 k8 "
      /*A*/ "o5 f1 | a1 | a+1 | o6 c1 "
      /*B*/ "o5 d2 d2 | o4 a+2 a+2 | f2 f2 | c2 c2 "
      /*C*/ "o4 a+4. a+4. a+4 | o5 c4. c4. c4 | o4 a4. a4. a4 | o5 d4. d4. d4 ",
    "@n t112 q3 "
      /*A*/ "[ [ v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ [ v13 o1 a16 v9 o6 d+16 d+16 d+16 v12 o4 d+16 v9 o6 d+16 d+16 d+16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ [ v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 ]2 ]3 | "
            "v13 o5 c16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 "
  } },
  // MEADOW -- D major. A I-IV-vi-V, B ii-V-I-vi, C the mixolydian I-bVII-IV-I
  { "MEADOW", {
    "@w8={" WAV_SINE "}@w9={" WAV_STRING "}@w10={" WAV_TRI "} "
      "@8 t126 v11 q7 me3 mp4,25,300 "
      /*A*/ "o5 d8 f+8 a4 o6 d2 | o6 d8 o5 b8 g4 b2 | o5 b8 o6 d8 f+4 d2 | "
            "o6 e4 c+4 o5 a2 "
      /*B*/ "o5 e8 g8 b4 o6 e2 | o6 e8 c+8 o5 a4 e2 | o5 f+8 a8 o6 d4 f+2 | "
            "o6 d4 o5 b4 f+2 "
      /*C*/ "@10 v11 q6 me6 mp0 o6 d4 e4 f+4 e4 | o6 g4 e4 c2 | o6 d4 o5 b4 g4 b4 | "
            "o6 a2 f+4 d4 ",
    "@10 t126 v11 q6 me4 "
      /*A*/ "o2 d8 o3 d8 o2 d8 o3 d8 o2 d8 o3 d8 o2 d8 o3 d8 | "
            "o2 g8 o3 g8 o2 g8 o3 g8 o2 g8 o3 g8 o2 g8 o3 g8 | "
            "o2 b8 o3 b8 o2 b8 o3 b8 o2 b8 o3 b8 o2 b8 o3 b8 | "
            "o2 a8 o3 a8 o2 a8 o3 a8 o2 a8 o3 a8 o2 a8 o3 a8 "
      /*B*/ "o2 [e4]4 | [a4]4 | [d4]4 | [b4]4 "
      /*C*/ "o2 [d8]8 | [c8]8 | [g8]8 | [d8]8 ",
    "@9 t126 v6 q8 me0 mv4,25 k-7 "
      /*A*/ "o4 f+1 | b1 | o5 d1 | c+1 "
      /*B*/ "o4 g2 g2 | c+2 c+2 | f+2 f+2 | d2 d2 "
      /*C*/ "o4 [f+4]4 | [e4]4 | [b4]4 | [f+4]4 ",
    "@9 t126 v6 q8 me0 mv4,25 k0 "
      /*A*/ "o4 a1 | o5 d1 | f+1 | e1 "
      /*B*/ "o4 b2 b2 | o5 e2 e2 | a2 a2 | f+2 f+2 "
      /*C*/ "o4 [a4]4 | [g4]4 | [d4]4 | [a4]4 ",
    "@9 t126 v6 q8 me0 mv4,25 k7 "
      /*A*/ "o5 d1 | g1 | b1 | a1 "
      /*B*/ "o5 e2 e2 | a2 a2 | d2 d2 | o4 b2 b2 "
      /*C*/ "o5 [d4]4 | [c4]4 | o4 [g4]4 | [d4]4 ",
    "@n t126 q3 "
      /*A*/ "[ [ v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ [ v9 o6 d+16 a16 d+16 a16 ]4 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ [ v13 o1 a16 v9 o6 d+16 d+16 d+16 v12 o4 d+16 v9 o6 d+16 d+16 d+16 ]2 ]3 | "
            "v13 o5 c16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 "
  } },
  // SKYLINE -- C# minor. A a dorian i7-IV7 vamp, B iim7b5-V7-i, C bVI-bVII-i
  { "SKYLINE", {
    "@w8={" WAV_VOX "}@w9={" WAV_GLASS "}@w10={" WAV_BASSR "}@w11={" WAV_BELL "} "
      "@8 t138 v11 q6 me2 mp6,32,150 "
      /*A*/ "o6 c+8 d+8 e4 g+2 | o6 a+4 g+4 f+2 | o6 e8 c+8 o5 b4 g+2 | "
            "o5 a+8 o6 c+8 e4 f+2 "
      /*B*/ "o6 f+4 d+4 a4 f+4 | o6 g+2 f+4 d+4 | o6 c+1 | o6 e4 d+4 c+2 "
      /*C*/ "@11 v11 q8 me0 mp3,20,400 o6 a4 g+4 e4 c+4 | o6 b4 a4 f+4 d+4 | "
            "o6 g+2 e4 c+4 | o7 c+2 o6 g+4 e4 ",
    "@10 t138 v11 q6 me4 "
      /*A*/ "o2 c+8 r8 c+8 c+8 r8 c+8 g+8 r8 | f+8 r8 f+8 f+8 r8 f+8 o3 c+8 r8 | "
            "o2 c+8 r8 c+8 c+8 r8 c+8 g+8 r8 | f+8 r8 f+8 f+8 r8 f+8 o3 c+8 r8 "
      /*B*/ "o2 d+4 f+4 a4 g4 | g+4 o3 c4 d+4 o2 d4 | c+4 e4 g+4 c4 | c+4 e4 g+4 d4 "
      /*C*/ "o2 a8 o3 a8 o2 a8 o3 a8 o2 a8 o3 a8 o2 a8 o3 a8 | "
            "o2 b8 o3 b8 o2 b8 o3 b8 o2 b8 o3 b8 o2 b8 o3 b8 | "
            "o2 c+8 o3 c+8 o2 c+8 o3 c+8 o2 c+8 o3 c+8 o2 c+8 o3 c+8 | "
            "o2 c+8 o3 c+8 o2 c+8 o3 c+8 o2 c+8 o3 c+8 o2 c+8 o3 c+8 ",
    "@9 t138 v6 q8 me0 mv5,30 k-9 "
      /*A*/ "r8 o5 e8 r8 e8 r8 e8 r8 e8 | r8 o4 a+8 r8 a+8 r8 a+8 r8 a+8 | "
            "r8 e8 r8 e8 r8 e8 r8 e8 | r8 a+8 r8 a+8 r8 a+8 r8 a+8 "
      /*B*/ "o4 f+8 r8 f+8 r8 f+8 r8 f+8 r8 | c8 r8 c8 r8 c8 r8 c8 r8 | "
            "e8 r8 e8 r8 e8 r8 e8 r8 | e8 r8 e8 r8 e8 r8 e8 r8 "
      /*C*/ "o5 c+1 | d+1 | e1 | e1 ",
    "@9 t138 v6 q8 me0 mv5,30 k0 "
      /*A*/ "r8 o4 g+8 r8 g+8 r8 g+8 r8 g+8 | r8 o5 c+8 r8 c+8 r8 c+8 r8 c+8 | "
            "r8 o4 g+8 r8 g+8 r8 g+8 r8 g+8 | r8 o5 c+8 r8 c+8 r8 c+8 r8 c+8 "
      /*B*/ "o4 a8 r8 a8 r8 a8 r8 a8 r8 | d+8 r8 d+8 r8 d+8 r8 d+8 r8 | "
            "g+8 r8 g+8 r8 g+8 r8 g+8 r8 | g+8 r8 g+8 r8 g+8 r8 g+8 r8 "
      /*C*/ "o5 e1 | f+1 | g+1 | g+1 ",
    "@9 t138 v6 q8 me0 mv5,30 k9 "
      /*A*/ "r8 o4 b8 r8 b8 r8 b8 r8 b8 | r8 o5 e8 r8 e8 r8 e8 r8 e8 | "
            "r8 o4 b8 r8 b8 r8 b8 r8 b8 | r8 o5 e8 r8 e8 r8 e8 r8 e8 "
      /*B*/ "o5 c+8 r8 c+8 r8 c+8 r8 c+8 r8 | f+8 r8 f+8 r8 f+8 r8 f+8 r8 | "
            "c+8 r8 c+8 r8 c+8 r8 c+8 r8 | c+8 r8 c+8 r8 c+8 r8 c+8 r8 "
      /*C*/ "o4 a1 | b1 | o5 c+1 | c+1 ",
    "@n t138 q3 "
      /*A*/ "[ [ v13 o1 a16 v8 o4 d+16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 v13 o1 a16 ]2 ]3 | "
            "a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ [ v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ [ v13 o1 a16 v9 o6 d+16 d+16 d+16 v12 o4 d+16 v9 o6 d+16 d+16 d+16 ]2 ]3 | "
            "v13 o5 c16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 "
  } },
  // VICTORY -- C major. A I-IV-V-I, B the vi-ii-V-I circle, C the heroic I-bVI-bVII-I
  { "VICTORY", {
    "@w8={" WAV_PULSE25 "}@w9={" WAV_FIFTH "}@w10={" WAV_SOFTSAW "}@w11={" WAV_SYNC "} "
      "@8 t150 v11 q7 me2 mp7,25,200 "
      /*A*/ "o6 e4 e4 c8 o5 g8 e4 | o6 f4 a4 o7 c2 | o6 b4 g4 d2 | o6 c8 e8 g8 o7 c8 e2 "
      /*B*/ "o6 a4 e4 c2 | o6 d8 f8 a4 f2 | o6 g4 d4 o5 b2 | o6 c2 e2 "
      /*C*/ "@11 v11 q6 me4 mp8,35,80 o6 g4 o7 c4 e2 | o6 g+4 o7 c4 d+2 | o6 a+4 o7 d4 f2 | "
            "o7 e2 c2 ",
    "@10 t150 v11 q6 me4 "
      /*A*/ "o2 [c4]4 | [f4]4 | [g4]4 | [c4]4 "
      /*B*/ "o2 a4 o3 c4 e4 o2 d+4 | d4 f4 a4 f+4 | g4 b4 o3 d4 o2 c+4 | c4 e4 g4 g+4 "
      /*C*/ "o2 c4 c4 g4 c4 | g+4 g+4 o3 d+4 o2 g+4 | a+4 a+4 o3 f4 o2 a+4 | c4 c4 g4 c4 ",
    "@9 t150 v6 q8 me0 mv4,20 k-7 "
      /*A*/ "o5 e1 | a1 | b1 | e1 "
      /*B*/ "o5 [c4]4 | [f4]4 | o4 [b4]4 | o5 [e4]4 "
      /*C*/ "o5 e4. e4. e4 | c4. c4. c4 | d4. d4. d4 | e4. e4. e4 ",
    "@9 t150 v6 q8 me0 mv4,20 k0 "
      /*A*/ "o4 g1 | o5 c1 | d1 | g1 "
      /*B*/ "o5 [e4]4 | [a4]4 | [d4]4 | [g4]4 "
      /*C*/ "o4 g4. g4. g4 | d+4. d+4. d+4 | f4. f4. f4 | g4. g4. g4 ",
    "@9 t150 v6 q8 me0 mv4,20 k7 "
      /*A*/ "o5 c1 | f1 | g1 | o6 c1 "
      /*B*/ "o4 [a4]4 | o5 [d4]4 | [g4]4 | o6 [c4]4 "
      /*C*/ "o5 c4. c4. c4 | o4 g+4. g+4. g+4 | a+4. a+4. a+4 | o5 c4. c4. c4 ",
    "@n t150 q3 "
      /*A*/ "[ [ v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ [ v13 o1 a16 v9 o6 d+16 d+16 d+16 v12 o4 d+16 v9 o6 d+16 d+16 d+16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 ]3 | "
            "v13 o5 c16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 "
  } },
  // PARADE -- G major. A I-ii-V-I, B IV-iv-I-V with a borrowed minor iv, C I-VI7-ii-V
  { "PARADE", {
    "@w8={" WAV_REED "}@w9={" WAV_ORGAN "}@w10={" WAV_BASSR "}@w11={" WAV_PIANO "} "
      "@8 t132 v11 q6 me3 mp5,20,300 "
      /*A*/ "o6 g4 d4 o5 b4 g4 | o6 a4 e4 c4 e4 | o6 f+4 a4 d4 f+4 | o6 g2 d2 "
      /*B*/ "o6 e4 g4 o7 c2 | o6 d+4 g4 o7 c2 | o6 d4 o5 b4 g2 | o6 a4 f+4 d2 "
      /*C*/ "@11 v11 q8 me1 mp6,40,120 o6 b8 a8 g4 d2 | o6 g+8 b8 o7 e4 o6 b2 | "
            "o6 a8 o7 c8 e4 c2 | o6 f+8 a8 o7 d4 o6 a2 ",
    "@10 t132 v11 q6 me4 "
      /*A*/ "o2 g2 g2 | a2 a2 | d2 d2 | g2 g2 "
      /*B*/ "o2 [c4]4 | [c4]4 | [g4]4 | [d4]4 "
      /*C*/ "o2 g8 o3 g8 o2 g8 o3 g8 o2 g8 o3 g8 o2 g8 o3 g8 | "
            "o2 e8 o3 e8 o2 e8 o3 e8 o2 e8 o3 e8 o2 e8 o3 e8 | "
            "o2 a8 o3 a8 o2 a8 o3 a8 o2 a8 o3 a8 o2 a8 o3 a8 | "
            "o2 d8 o3 d8 o2 d8 o3 d8 o2 d8 o3 d8 o2 d8 o3 d8 ",
    "@9 t132 v6 q8 me0 mv3,25 k-7 "
      /*A*/ "o4 b1 | o5 c1 | o4 f+1 | b1 "
      /*B*/ "o5 e2 e2 | d+2 d+2 | o4 b2 b2 | f+2 f+2 "
      /*C*/ "o4 [b4]4 | [g+4]4 | o5 [c4]4 | o4 [f+4]4 ",
    "@9 t132 v6 q8 me0 mv3,25 k0 "
      /*A*/ "o5 d1 | e1 | a1 | d1 "
      /*B*/ "o4 g2 g2 | g2 g2 | d2 d2 | a2 a2 "
      /*C*/ "o5 [d4]4 | o4 [b4]4 | o5 [e4]4 | [a4]4 ",
    "@9 t132 v6 q8 me0 mv3,25 k7 "
      /*A*/ "o4 g1 | a1 | o5 d1 | g1 "
      /*B*/ "o5 c2 c2 | c2 c2 | o4 g2 g2 | d2 d2 "
      /*C*/ "o4 [g4]4 | [d4]4 | [a4]4 | o5 [d4]4 ",
    "@n t132 q3 "
      /*A*/ "[ v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ [ v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 ]4 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ [ v13 o1 a16 v8 o4 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 ]2 ]3 | "
            "v13 o5 c16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 "
  } },
  // FANFARE -- A I-V/V-V-I, B the bIII-bVI-bVII-I lift, C a vi-V-IV-III flamenco fall
  { "FANFARE", {
    "@w8={" WAV_PULSE12 "}@w9={" WAV_STRING "}@w10={" WAV_SOFTSAW "} "
      "@8 t150 v11 q7 me1 mp6,40,120 "
      /*A*/ "o6 c4 e4 g4 o7 c4 | o6 a4 f+4 d2 | o6 b4 o7 d4 g2 | o7 c2 o6 g2 "
      /*B*/ "o6 a+4 g4 d+2 | o7 c4 o6 a+4 g+2 | o6 a+4 o7 d4 f2 | o7 e4 g4 o6 c2 "
      /*C*/ "@9 v11 q8 me0 mp4,30,250 o6 a4 o7 c4 e2 | o6 g4 b4 o7 d2 | o6 f4 a4 o7 c2 | "
            "o6 g+4 b4 o7 e2 ",
    "@10 t150 v11 q6 me4 "
      /*A*/ "o2 c2 c2 | d2 d2 | g2 g2 | c2 c2 "
      /*B*/ "o2 d+4 d+4 a+4 d+4 | g+4 g+4 o3 d+4 o2 g+4 | a+4 a+4 o3 f4 o2 a+4 | "
            "c4 c4 g4 c4 "
      /*C*/ "o2 [a4]4 | [g4]4 | [f4]4 | [e4]4 ",
    "@9 t150 v6 q8 me0,300 mv3,18 k-10 "
      /*A*/ "o5 e1 | f+1 | b1 | e1 "
      /*B*/ "o4 g4. g4. g4 | o5 c4. c4. c4 | d4. d4. d4 | e4. e4. e4 "
      /*C*/ "o5 [c4]4 | o4 [b4]4 | [a4]4 | [g+4]4 ",
    "@9 t150 v6 q8 me0,300 mv3,18 k0 "
      /*A*/ "o4 g1 | a1 | o5 d1 | g1 "
      /*B*/ "o4 a+4. a+4. a+4 | o5 d+4. d+4. d+4 | f4. f4. f4 | g4. g4. g4 "
      /*C*/ "o5 [e4]4 | [d4]4 | [c4]4 | o4 [b4]4 ",
    "@9 t150 v6 q8 me0,300 mv3,18 k10 "
      /*A*/ "o5 c1 | c1 | o4 g1 | o5 c1 "
      /*B*/ "o5 d+4. d+4. d+4 | g+4. g+4. g+4 | a+4. a+4. a+4 | o6 c4. c4. c4 "
      /*C*/ "o4 [a4]4 | [g4]4 | [f4]4 | [e4]4 ",
    "@n t150 q3 "
      /*A*/ "v13 o5 c16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 | "
            "[ [ v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 ]2 ]2 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ [ v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 ]4 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 v12 o4 d+16 v8 d+16 v12 d+16 v8 d+16 v12 d+16 v8 d+16 v12 d+16 v8 d+16 "
  } },
  // TWILIGHT -- A minor. A i-VI-III-VII, B i-v-VI-iv, C i-iv-bVII-III
  { "TWILIGHT", {
    "@w8={" WAV_SINE "}@w9={" WAV_HOLLOW "}@w10={" WAV_TRI "}@w11={" WAV_BELL "} "
      "@8 t96 v11 q8 me1 mp4,38,300 "
      /*A*/ "o6 a2. e4 | o6 c2 f4 c4 | o6 e2. c4 | o6 d2 o5 b4 d4 "
      /*B*/ "o6 c4 o5 a4 e2 | o6 e4 b4 g2 | o6 f4 a4 o7 c2 | o6 d4 f4 a2 "
      /*C*/ "@11 v10 q6 me5 mp6,25,150 o6 a8 g8 e4 a2 | o6 f8 e8 d4 f2 | o6 g8 f8 d4 g2 | "
            "o7 c2 o6 g2 ",
    "@10 t96 v11 q6 me2 "
      /*A*/ "o2 a1 | f1 | c1 | g1 "
      /*B*/ "o2 a2 a2 | e2 e2 | f2 f2 | d2 d2 "
      /*C*/ "o2 a8 o3 e8 a8 e8 o2 a8 o3 e8 a8 e8 | o2 d8 a8 o3 d8 o2 a8 d8 a8 o3 d8 o2 a8 | "
            "g8 o3 d8 g8 d8 o2 g8 o3 d8 g8 d8 | o2 c8 g8 o3 c8 o2 g8 c8 g8 o3 c8 o2 g8 ",
    "@9 t96 v6 q8 me0 mv3,30 k-9 "
      /*A*/ "o5 c1 | o4 a1 | e1 | b1 "
      /*B*/ "o5 c2 c2 | o4 g2 g2 | a2 a2 | f2 f2 "
      /*C*/ "o5 c2 r4 c4 | f2 r4 f4 | o4 b2 r4 b4 | o5 e2 r4 e4 ",
    "@9 t96 v6 q8 me0 mv3,30 k0 "
      /*A*/ "o5 e1 | c1 | o4 g1 | d1 "
      /*B*/ "o5 e2 e2 | o4 b2 b2 | o5 c2 c2 | o4 a2 a2 "
      /*C*/ "o5 e2 r4 e4 | a2 r4 a4 | d2 r4 d4 | g2 r4 g4 ",
    "@9 t96 v6 q8 me0 mv3,30 k9 "
      /*A*/ "o4 a1 | f1 | c1 | g1 "
      /*B*/ "o4 a2 a2 | e2 e2 | f2 f2 | d2 d2 "
      /*C*/ "o4 a2 r4 a4 | o5 d2 r4 d4 | g2 r4 g4 | o6 c2 r4 c4 ",
    "@n t96 q3 "
      /*A*/ "[ v13 o1 a16 r4. r16 v12 o4 d+16 r4 r16 v9 o6 d+16 r16 ]4 "
      /*B*/ "[ v13 o1 a16 r4 r16 v8 o4 d+16 r16 v12 d+16 r4 r16 v8 d+16 r16 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ v13 o1 a16 r4. r16 v12 o4 d+16 r4 r16 v9 o6 d+16 r16 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r4. r16 "
  } },
  // CAVERN -- D minor. A a phrygian i-bII, B the andalusian i-bVII-bVI-V, C i-iv-i-V
  { "CAVERN", {
    "@w8={" WAV_HOLLOW "}@w9={" WAV_GLASS "}@w10={" WAV_BASSR "}@w11={" WAV_VOX "} "
      "@8 t88 v11 q8 me0 mp3,45,400 "
      /*A*/ "o5 d1 | o5 d+2. d4 | o5 f2 a2 | o6 c2 o5 a+2 "
      /*B*/ "o6 d2 c2 | o6 c2 o5 a+2 | o5 a+2 a2 | o5 a1 "
      /*C*/ "@11 v10 q6 me4 mp5,30,200 o6 d4 f4 a2 | o6 g4 a+4 d2 | o6 f4 d4 o5 a2 | "
            "o6 c+4 e4 a2 ",
    "@10 t88 v11 q6 me1 "
      /*A*/ "o2 d1 | d+1 | d1 | d+1 "
      /*B*/ "o2 d2 d2 | c2 c2 | a+2 a+2 | a2 a2 "
      /*C*/ "o2 d2 r4 d4 | g2 r4 g4 | d2 r4 d4 | a2 r4 a4 ",
    "@9 t88 v6 q8 me0,500 mv2,35 k-12 "
      /*A*/ "o5 f1 | g1 | f1 | g1 "
      /*B*/ "o5 f1 | e1 | d1 | c+1 "
      /*C*/ "o5 f2 r4 f4 | a+2 r4 a+4 | f2 r4 f4 | c+2 r4 c+4 ",
    "@9 t88 v6 q8 me0,500 mv2,35 k0 "
      /*A*/ "o4 a1 | a+1 | a1 | a+1 "
      /*B*/ "o4 a1 | g1 | f1 | e1 "
      /*C*/ "o4 a2 r4 a4 | o5 d2 r4 d4 | o4 a2 r4 a4 | e2 r4 e4 ",
    "@9 t88 v6 q8 me0,500 mv2,35 k12 "
      /*A*/ "o5 d1 | d+1 | d1 | d+1 "
      /*B*/ "o5 d1 | c1 | o4 a+1 | a1 "
      /*C*/ "o5 d2 r4 d4 | g2 r4 g4 | d2 r4 d4 | o4 a2 r4 a4 ",
    "@n t88 q3 "
      /*A*/ "[ v13 o1 a16 r8 a16 r2 a16 r8 a16 ]4 "
      /*B*/ "[ v13 o1 a16 r8 a16 r2 a16 r8 a16 ]3 | "
            "a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ v13 o1 a16 r2 r8. v12 o4 d+16 r8. ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r4. r16 "
  } },
  // NIGHTFALL -- Bb major into G minor. A IV-I-V-vi, B ii-V-iii-vi, C a minor line cliche
  { "NIGHTFALL", {
    "@w8={" WAV_VOX "}@w9={" WAV_STRING "}@w10={" WAV_TRI "}@w11={" WAV_GLASS "} "
      "@8 t104 v11 q6 me2 mp5,35,220 "
      /*A*/ "o6 a+2 o7 d4 o6 a+4 | o6 d2 f4 d4 | o6 c2 a4 f4 | o6 g2 a+4 g4 "
      /*B*/ "o6 g8 a+8 o7 c4 o6 g2 | o6 a8 o7 c8 f4 c2 | o6 f8 a8 d4 a2 | o6 a+8 g8 d4 g2 "
      /*C*/ "@11 v10 q8 me0 mp4,45,300 o6 g2 d2 | o6 f+2 d2 | o6 f2 d2 | o6 e2 d2 ",
    "@10 t104 v11 q6 me3 "
      /*A*/ "o2 d+2 d+2 | a+2 a+2 | f2 f2 | g2 g2 "
      /*B*/ "o2 [c4]4 | [f4]4 | [d4]4 | [g4]4 "
      /*C*/ "o2 g2 r4 g4 | g2 r4 g4 | g2 r4 g4 | g2 r4 g4 ",
    "@9 t104 v6 q8 me0,350 mv3,26 k-8 "
      /*A*/ "o4 g1 | d1 | a1 | a+1 "
      /*B*/ "o5 d+2 d+2 | o4 a2 a2 | f2 f2 | a+2 a+2 "
      /*C*/ "o4 a+1 | a+1 | a+1 | a+1 ",
    "@9 t104 v6 q8 me0,350 mv3,26 k0 "
      /*A*/ "o4 a+1 | f1 | c1 | d1 "
      /*B*/ "o4 g2 g2 | o5 c2 c2 | o4 a2 a2 | o5 d2 d2 "
      /*C*/ "o5 d1 | d1 | d1 | d1 ",
    "@9 t104 v6 q8 me0,350 mv3,26 k8 "
      /*A*/ "o5 d+1 | o4 a+1 | f1 | g1 "
      /*B*/ "o5 c2 c2 | f2 f2 | d2 d2 | g2 g2 "
      /*C*/ "o4 g1 | f+1 | f1 | e1 ",
    "@n t104 q3 "
      /*A*/ "[ v13 o1 a16 r4 r16 v8 o4 d+16 r16 v12 d+16 r4 r16 v8 d+16 r16 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ [ v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ v13 o1 a16 r4. r16 v12 o4 d+16 r4 r16 v9 o6 d+16 r16 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r4. r16 "
  } },
  // SHADOW -- E minor. A i-VI-iv-V, B i-VII-VI-VII, C bVI-bVII-i
  { "SHADOW", {
    "@w8={" WAV_METAL "}@w9={" WAV_HOLLOW "}@w10={" WAV_SOFTSAW "} "
      "@8 t108 v11 q6 me3 mp5,30,200 "
      /*A*/ "o6 e4 g4 b2 | o6 e4 c4 g2 | o6 c4 a4 e2 | o6 d+4 f+4 b2 "
      /*B*/ "o6 b8 a8 g4 e2 | o6 a8 g8 f+4 d2 | o6 g8 f+8 e4 c2 | o6 f+8 e8 d4 f+2 "
      /*C*/ "@9 v11 q8 me0 mp3,50,250 o6 c4 e4 g2 | o6 d4 f+4 a2 | o6 e2 b2 | o7 e2 o6 b2 ",
    "@10 t108 v11 q6 me3 "
      /*A*/ "o2 [e4]4 | [c4]4 | [a4]4 | [b4]4 "
      /*B*/ "o2 [e8]8 | [d8]8 | [c8]8 | [d8]8 "
      /*C*/ "o2 c4 c4 g4 c4 | d4 d4 a4 d4 | e4 e4 b4 e4 | e4 e4 b4 e4 ",
    "@9 t108 v6 q8 me0 mv4,30 k-10 "
      /*A*/ "o4 g1 | e1 | c1 | d+1 "
      /*B*/ "o4 g8 r8 g8 r8 g8 r8 g8 r8 | f+8 r8 f+8 r8 f+8 r8 f+8 r8 | "
            "e8 r8 e8 r8 e8 r8 e8 r8 | f+8 r8 f+8 r8 f+8 r8 f+8 r8 "
      /*C*/ "o5 e4. e4. e4 | f+4. f+4. f+4 | g4. g4. g4 | g4. g4. g4 ",
    "@9 t108 v6 q8 me0 mv4,30 k0 "
      /*A*/ "o4 b1 | g1 | e1 | f+1 "
      /*B*/ "o4 b8 r8 b8 r8 b8 r8 b8 r8 | a8 r8 a8 r8 a8 r8 a8 r8 | "
            "g8 r8 g8 r8 g8 r8 g8 r8 | a8 r8 a8 r8 a8 r8 a8 r8 "
      /*C*/ "o4 g4. g4. g4 | a4. a4. a4 | b4. b4. b4 | b4. b4. b4 ",
    "@9 t108 v6 q8 me0 mv4,30 k10 "
      /*A*/ "o5 e1 | c1 | o4 a1 | b1 "
      /*B*/ "o5 e8 r8 e8 r8 e8 r8 e8 r8 | d8 r8 d8 r8 d8 r8 d8 r8 | "
            "c8 r8 c8 r8 c8 r8 c8 r8 | d8 r8 d8 r8 d8 r8 d8 r8 "
      /*C*/ "o5 c4. c4. c4 | d4. d4. d4 | e4. e4. e4 | e4. e4. e4 ",
    "@n t108 q3 "
      /*A*/ "[ v13 o1 a16 r4. r16 v12 o4 d+16 r4. r16 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ [ v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ [ v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 ]2 ]3 | "
            "v13 o5 c16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 "
  } },
  // REQUIEM -- D minor. A the harmonic i-iv-V-i, B a neapolitan iv-bII-V-i, C i-v-bVI-bVII
  { "REQUIEM", {
    "@w8={" WAV_ORGAN "}@w9={" WAV_FIFTH "}@w10={" WAV_BASSR "} "
      "@8 t76 v11 q8 me0 mp3,30,500 "
      /*A*/ "o5 d2 f2 | o5 g2 a+2 | o6 c+2 e2 | o6 d1 "
      /*B*/ "o6 a+2 g2 | o6 g2 d+2 | o6 e2 c+2 | o6 d2 o5 a2 "
      /*C*/ "@10 v10 q6 me3 mp5,20,200 o6 f4 e4 d2 | o6 e4 c4 o5 a2 | o6 d4 a+4 f2 | "
            "o6 e4 c4 g2 ",
    "@10 t76 v10 q8 me0 "
      /*A*/ "o2 d1 | g1 | a1 | d1 "
      /*B*/ "o2 g1 | d+1 | a1 | d1 "
      /*C*/ "o2 d2 d2 | a2 a2 | a+2 a+2 | c2 c2 ",
    "@9 t76 v6 q8 me0,600 mv2,20 k-11 "
      /*A*/ "o5 f1 | a+1 | c+1 | f1 "
      /*B*/ "o4 a+1 | g1 | c+1 | f1 "
      /*C*/ "o5 f2 r4 f4 | c2 r4 c4 | d2 r4 d4 | e2 r4 e4 ",
    "@9 t76 v6 q8 me0,600 mv2,20 k0 "
      /*A*/ "o4 a1 | o5 d1 | e1 | a1 "
      /*B*/ "o5 d1 | o4 a+1 | e1 | a1 "
      /*C*/ "o4 a2 r4 a4 | e2 r4 e4 | f2 r4 f4 | g2 r4 g4 ",
    "@9 t76 v6 q8 me0,600 mv2,20 k11 "
      /*A*/ "o5 d1 | g1 | a1 | d1 "
      /*B*/ "o4 g1 | d+1 | a1 | o5 d1 "
      /*C*/ "o5 d2 r4 d4 | o4 a2 r4 a4 | a+2 r4 a+4 | o5 c2 r4 c4 ",
    "@n t76 q3 "
      /*A*/ "[ r1 ]4 "
      /*B*/ "[ v13 o1 a16 r8 a16 r2 a16 r8 a16 ]4 "
      /*C*/ "[ v13 o1 a16 r2 r8. v12 o4 d+16 r8. ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r4. r16 "
  } },
  // GLACIER -- C minor. A i-bVI-bIII-bVII, B i-bVII-bVI-V, C a suspended drone that resolves
  { "GLACIER", {
    "@w8={" WAV_GLASS "}@w9={" WAV_SINE "}@w10={" WAV_TRI "}@w11={" WAV_BELL "} "
      "@8 t92 v11 q8 me0 mp2,25,400 "
      /*A*/ "o6 c2 g2 | o6 g+2 d+2 | o6 a+2 g2 | o6 d2 f2 "
      /*B*/ "o6 d+4 c4 g2 | o6 d4 a+4 f2 | o6 c4 g+4 d+2 | o6 d4 g4 o5 b2 "
      /*C*/ "@11 v10 q6 me4 mp6,20,150 o6 f2 d+2 | o6 d+2 c2 | o6 f2 d+2 | o6 d2 a+2 ",
    "@10 t92 v10 q7 me2 "
      /*A*/ "o2 c1 | g+1 | d+1 | a+1 "
      /*B*/ "o2 c2 c2 | a+2 a+2 | g+2 g+2 | g2 g2 "
      /*C*/ "o2 c2 r4 c4 | c2 r4 c4 | a+2 r4 a+4 | a+2 r4 a+4 ",
    "@9 t92 v6 q8 me0,500 mv2,28 k-12 "
      /*A*/ "o5 d+1 | c1 | o4 g1 | d1 "
      /*B*/ "o5 d+2 d+2 | d2 d2 | c2 c2 | o4 b2 b2 "
      /*C*/ "o5 f1 | d+1 | d+1 | d1 ",
    "@9 t92 v6 q8 me0,500 mv2,28 k0 "
      /*A*/ "o4 g1 | d+1 | a+1 | f1 "
      /*B*/ "o4 g2 g2 | f2 f2 | d+2 d+2 | d2 d2 "
      /*C*/ "o4 g1 | g1 | f1 | f1 ",
    "@9 t92 v6 q8 me0,500 mv2,28 k12 "
      /*A*/ "o5 c1 | o4 g+1 | d+1 | a+1 "
      /*B*/ "o5 c2 c2 | o4 a+2 a+2 | g+2 g+2 | g2 g2 "
      /*C*/ "o5 c1 | c1 | o4 a+1 | a+1 ",
    "@n t92 q3 "
      /*A*/ "[ [ v9 o6 d+16 r16 d+16 r16 ]4 ]4 "
      /*B*/ "[ [ v9 o6 d+16 [d+16]3 ]4 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ v13 o1 a16 r2 r8. v12 o4 d+16 r8. ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r4. r16 "
  } },
  // NEBULA -- F# lydian. A a I-II vamp, B I-bVII-IV, C the Imaj7-IIImaj7 chromatic mediant
  { "NEBULA", {
    "@w8={" WAV_GLASS "}@w9={" WAV_SINE "}@w10={" WAV_TRI "}@w11={" WAV_BELL "} "
      "@8 t84 v11 q8 me0 mp2,30,500 "
      /*A*/ "o6 c+2 a+2 | o6 d+2 c2 | o6 f+1 | o6 g+2 d+2 "
      /*B*/ "o6 f+4 a+4 o7 c+2 | o6 b4 g+4 e2 | o6 b2 f+2 | o6 a+2 f+2 "
      /*C*/ "@11 v10 q6 me3 mp5,25,200 o6 f+2 f2 | o6 a+2 a2 | o6 c+1 | o6 d2 f2 ",
    "@10 t84 v10 q8 me0 "
      /*A*/ "o2 f+1 | g+1 | f+1 | g+1 "
      /*B*/ "o2 f+2 f+2 | e2 e2 | b2 b2 | f+2 f+2 "
      /*C*/ "o2 f+2 r4 f+4 | a+2 r4 a+4 | f+2 r4 f+4 | a+2 r4 a+4 ",
    "@9 t84 v6 q8 me0,600 mv2,24 k-12 "
      /*A*/ "o4 a+1 | o5 c1 | o4 a+1 | o5 c1 "
      /*B*/ "o4 a+1 | g+1 | d+1 | a+1 "
      /*C*/ "o4 a+2 r4 a+4 | o5 d2 r4 d4 | o4 a+2 r4 a+4 | o5 d2 r4 d4 ",
    "@9 t84 v6 q8 me0,600 mv2,24 k0 "
      /*A*/ "o5 c+1 | d+1 | c+1 | d+1 "
      /*B*/ "o5 c+1 | o4 b1 | f+1 | c+1 "
      /*C*/ "o5 c+2 r4 c+4 | f2 r4 f4 | c+2 r4 c+4 | f2 r4 f4 ",
    "@9 t84 v6 q8 me0,600 mv2,24 k12 "
      /*A*/ "o4 f+1 | g+1 | f+1 | g+1 "
      /*B*/ "o4 f+1 | e1 | b1 | f+1 "
      /*C*/ "o5 f2 r4 f4 | a2 r4 a4 | f2 r4 f4 | a2 r4 a4 ",
    "@n t84 q3 "
      /*A*/ "[ r1 ]4 "
      /*B*/ "[ [ v9 o6 d+16 r16 d+16 r16 ]4 ]4 "
      /*C*/ "[ v13 o1 a16 r8 a16 r2 a16 r8 a16 ]3 | "
            "a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r4. r16 "
  } },
  // DRIFT -- A major. A the Imaj7-vi7-ii7-V7 turnaround, B IVmaj7-iii7-vi7-V, C I-bIII-IV-iv
  { "DRIFT", {
    "@w8={" WAV_SINE "}@w9={" WAV_STRING "}@w10={" WAV_TRI "}@w11={" WAV_VOX "} "
      "@8 t100 v11 q8 me1 mp4,28,300 "
      /*A*/ "o6 a4 e4 c+2 | o6 e4 c+4 a2 | o6 d4 f+4 a2 | o6 b4 g+4 e2 "
      /*B*/ "o6 d4 f+4 a2 | o6 e4 g+4 b2 | o6 f+4 a4 o7 c+2 | o6 b4 g+4 e2 "
      /*C*/ "@11 v10 q6 me4 mp6,20,150 o6 a2 e2 | o6 g2 e2 | o6 f+2 d2 | o6 f2 d2 ",
    "@10 t100 v10 q6 me2 "
      /*A*/ "o2 a2 a2 | f+2 f+2 | b2 b2 | e2 e2 "
      /*B*/ "o2 d8 a8 o3 d8 o2 a8 d8 a8 o3 d8 o2 a8 | "
            "c+8 g+8 o3 c+8 o2 g+8 c+8 g+8 o3 c+8 o2 g+8 | "
            "f+8 o3 c+8 f+8 c+8 o2 f+8 o3 c+8 f+8 c+8 | "
            "o2 e8 b8 o3 e8 o2 b8 e8 b8 o3 e8 o2 b8 "
      /*C*/ "o2 a1 | c1 | d1 | d1 ",
    "@9 t100 v6 q8 me0,400 mv3,25 k-9 "
      /*A*/ "o5 c+1 | o4 a1 | o5 d1 | o4 g+1 "
      /*B*/ "o4 f+2 f+2 | e2 e2 | a2 a2 | g+2 g+2 "
      /*C*/ "o5 c+2 r4 c+4 | e2 r4 e4 | f+2 r4 f+4 | f2 r4 f4 ",
    "@9 t100 v6 q8 me0,400 mv3,25 k0 "
      /*A*/ "o5 e1 | c+1 | f+1 | b1 "
      /*B*/ "o4 a2 a2 | g+2 g+2 | o5 c+2 c+2 | o4 b2 b2 "
      /*C*/ "o5 e2 r4 e4 | g2 r4 g4 | a2 r4 a4 | a2 r4 a4 ",
    "@9 t100 v6 q8 me0,400 mv3,25 k9 "
      /*A*/ "o4 g+1 | e1 | a1 | o5 d1 "
      /*B*/ "o5 c+2 c+2 | o4 b2 b2 | o5 e2 e2 | e2 e2 "
      /*C*/ "o4 a2 r4 a4 | o5 c2 r4 c4 | d2 r4 d4 | d2 r4 d4 ",
    "@n t100 q3 "
      /*A*/ "[ v13 o1 a16 r4 r16 v8 o4 d+16 r16 v12 d+16 r4 r16 v8 d+16 r16 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ [ v9 o6 d+16 a16 d+16 a16 ]4 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ v13 o1 a16 r4. r16 v12 o4 d+16 r4 r16 v9 o6 d+16 r16 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r4. r16 "
  } },
  // AURORA -- E major. A Imaj7-iii7-IVmaj7-V, B vi-IV-ii-V, C the bVI-bVII-I lift
  { "AURORA", {
    "@w8={" WAV_BELL "}@w9={" WAV_GLASS "}@w10={" WAV_SINE "}@w11={" WAV_ORGAN "} "
      "@8 t108 v11 q7 me2 mp5,25,250 "
      /*A*/ "o6 b4 g+4 e2 | o6 d+4 b4 g+2 | o6 c+4 e4 a2 | o6 f+4 d+4 b2 "
      /*B*/ "o6 g+4 e4 c+2 | o6 e4 c+4 a2 | o6 c+4 a4 f+2 | o6 d+4 b4 f+2 "
      /*C*/ "@11 v11 q8 me0 mp3,40,300 o6 c2 g2 | o6 d2 a2 | o6 e2 b2 | o7 e2 o6 b2 ",
    "@10 t108 v10 q7 me2 "
      /*A*/ "o2 e2 e2 | g+2 g+2 | a2 a2 | b2 b2 "
      /*B*/ "o2 c+8 g+8 o3 c+8 o2 g+8 c+8 g+8 o3 c+8 o2 g+8 | "
            "a8 o3 e8 a8 e8 o2 a8 o3 e8 a8 e8 | "
            "o2 f+8 o3 c+8 f+8 c+8 o2 f+8 o3 c+8 f+8 c+8 | "
            "o2 b8 o3 f+8 b8 f+8 o2 b8 o3 f+8 b8 f+8 "
      /*C*/ "o2 c4 c4 g4 c4 | d4 d4 a4 d4 | e4 e4 b4 e4 | e4 e4 b4 e4 ",
    "@9 t108 v6 q8 me0,350 mv4,22 k-10 "
      /*A*/ "o4 g+1 | b1 | o5 c+1 | d+1 "
      /*B*/ "o5 e2 e2 | c+2 c+2 | o4 a2 a2 | d+2 d+2 "
      /*C*/ "o5 e4. e4. e4 | f+4. f+4. f+4 | g+4. g+4. g+4 | g+4. g+4. g+4 ",
    "@9 t108 v6 q8 me0,350 mv4,22 k0 "
      /*A*/ "o4 b1 | o5 d+1 | e1 | f+1 "
      /*B*/ "o4 g+2 g+2 | e2 e2 | c+2 c+2 | f+2 f+2 "
      /*C*/ "o4 g4. g4. g4 | a4. a4. a4 | b4. b4. b4 | b4. b4. b4 ",
    "@9 t108 v6 q8 me0,350 mv4,22 k10 "
      /*A*/ "o5 d+1 | f+1 | g+1 | b1 "
      /*B*/ "o5 c+2 c+2 | o4 a2 a2 | f+2 f+2 | b2 b2 "
      /*C*/ "o5 c4. c4. c4 | d4. d4. d4 | e4. e4. e4 | e4. e4. e4 ",
    "@n t108 q3 "
      /*A*/ "[ [ v9 o6 d+16 r16 d+16 r16 ]4 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ [ v9 o6 d+16 a16 d+16 a16 ]4 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ [ v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 ]2 ]3 | "
            "v13 o5 c16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 "
  } },
  // TIDE -- G dorian. A the i-IV vamp that makes the mode, B bIII-bVII-i-v, C i-bVI-bIII-iv
  { "TIDE", {
    "@w8={" WAV_VOX "}@w9={" WAV_SINE "}@w10={" WAV_BASSR "}@w11={" WAV_GLASS "} "
      "@8 t102 v11 q7 me2 mp4,30,250 "
      /*A*/ "o6 g4 a+4 d2 | o6 e4 c4 g2 | o6 d4 f4 g2 | o6 g4 e4 c2 "
      /*B*/ "o6 a+4 d4 f2 | o6 a4 c4 f2 | o6 g4 a+4 d2 | o6 f4 d4 a2 "
      /*C*/ "@11 v10 q8 me0 mp2,20,400 o6 d2 g2 | o6 d+2 a+2 | o6 d2 f2 | o6 d+2 g2 ",
    "@10 t102 v11 q6 me3 "
      /*A*/ "o2 g8 o3 d8 g8 d8 o2 f8 o3 d8 o2 a+8 o3 d8 | "
            "o2 c8 g8 o3 c8 o2 g8 a+8 g8 e8 g8 | g8 o3 d8 g8 d8 o2 f8 o3 d8 o2 a+8 o3 d8 | "
            "o2 c8 g8 o3 c8 o2 g8 a+8 g8 e8 g8 "
      /*B*/ "o2 [a+4]4 | [f4]4 | [g4]4 | [d4]4 "
      /*C*/ "o2 g1 | d+1 | a+1 | c1 ",
    "@9 t102 v6 q8 me0,300 mv3,28 k-9 "
      /*A*/ "r8 o4 a+8 r8 a+8 r8 a+8 r8 a+8 | r8 e8 r8 e8 r8 e8 r8 e8 | "
            "r8 a+8 r8 a+8 r8 a+8 r8 a+8 | r8 e8 r8 e8 r8 e8 r8 e8 "
      /*B*/ "o5 d2 d2 | o4 a2 a2 | a+2 a+2 | f2 f2 "
      /*C*/ "o4 a+1 | g1 | d1 | d+1 ",
    "@9 t102 v6 q8 me0,300 mv3,28 k0 "
      /*A*/ "r8 o5 d8 r8 d8 r8 d8 r8 d8 | r8 g8 r8 g8 r8 g8 r8 g8 | "
            "r8 d8 r8 d8 r8 d8 r8 d8 | r8 g8 r8 g8 r8 g8 r8 g8 "
      /*B*/ "o5 f2 f2 | c2 c2 | d2 d2 | o4 a2 a2 "
      /*C*/ "o5 d1 | o4 a+1 | f1 | g1 ",
    "@9 t102 v6 q8 me0,300 mv3,28 k9 "
      /*A*/ "r8 o4 g8 r8 g8 r8 g8 r8 g8 | r8 o5 c8 r8 c8 r8 c8 r8 c8 | "
            "r8 o4 g8 r8 g8 r8 g8 r8 g8 | r8 o5 c8 r8 c8 r8 c8 r8 c8 "
      /*B*/ "o4 a+2 a+2 | f2 f2 | g2 g2 | d2 d2 "
      /*C*/ "o4 g1 | d+1 | a+1 | o5 c1 ",
    "@n t102 q3 "
      /*A*/ "[ [ v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ [ v13 o1 a16 r8 v9 o6 d+16 v12 o4 d+16 r8 v9 o6 d+16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ v13 o1 a16 r4. r16 v12 o4 d+16 r4 r16 v9 o6 d+16 r16 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r4. r16 "
  } },
  // MONOLITH -- E minor, power chords. A i5-bVI5-bVII5, B the tritone i-bV-bVI, C i-iv-bII-i
  { "MONOLITH", {
    "@w8={" WAV_SYNC "}@w9={" WAV_METAL "}@w10={" WAV_SOFTSAW "}@w11={" WAV_HOLLOW "} "
      "@8 t80 v11 q8 me0 mp3,40,400 "
      /*A*/ "o5 e1 | o5 g2 c2 | o5 a2 d2 | o5 b2 e2 "
      /*B*/ "o5 e2 b2 | o5 f2 a+2 | o5 g2 o6 c2 | o6 c2 o5 g2 "
      /*C*/ "@11 v11 q6 me3 mp5,25,150 o5 b4 g4 e2 | o5 e4 a4 o6 c2 | o6 c4 a4 f2 | "
            "o6 b4 g4 e2 ",
    "@10 t80 v11 q8 me0 "
      /*A*/ "o2 e1 | c1 | d1 | e1 "
      /*B*/ "o2 e2 e2 | a+2 a+2 | c2 c2 | c2 c2 "
      /*C*/ "o2 e4 e4 b4 e4 | a4 a4 o3 e4 o2 a4 | f4 f4 o3 c4 o2 f4 | e4 e4 b4 e4 ",
    "@9 t80 v5 q8 me0,500 mv2,30 k-12 "
      /*A*/ "o4 b1 | g1 | a1 | b1 "
      /*B*/ "o4 g1 | d1 | e1 | e1 "
      /*C*/ "o4 [g4]4 | o5 [c4]4 | o4 [a4]4 | [g4]4 ",
    "@9 t80 v5 q8 me0,500 mv2,30 k0 "
      /*A*/ "o5 e1 | c1 | d1 | e1 "
      /*B*/ "o4 b1 | f1 | g1 | g1 "
      /*C*/ "o4 [b4]4 | o5 [e4]4 | [c4]4 | o4 [b4]4 ",
    "@9 t80 v5 q8 me0,500 mv2,30 k12 "
      /*A*/ "o4 b1 | g1 | a1 | b1 "
      /*B*/ "o5 e1 | o4 a+1 | o5 c1 | c1 "
      /*C*/ "o5 [e4]4 | [a4]4 | [f4]4 | [e4]4 ",
    "@n t80 q3 "
      /*A*/ "[ v13 o1 a16 r8 a16 r2 a16 r8 a16 ]4 "
      /*B*/ "[ v13 o1 a16 r4. r16 v12 o4 d+16 r4. r16 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 ]3 | "
            "v13 o5 c16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 "
  } },
  // LULLABY -- C major. A the doo-wop I-vi-IV-V, B I-I7-IV-iv, C ii-V-I-vi
  { "LULLABY", {
    "@w8={" WAV_SINE "}@w9={" WAV_TRI "}@w10={" WAV_BASSR "}@w11={" WAV_BELL "} "
      "@8 t72 v11 q8 me1 mp3,22,400 "
      /*A*/ "o6 c2 e2 | o6 a2 e2 | o6 f2 a2 | o6 g2 d2 "
      /*B*/ "o6 e2 g2 | o6 a+2 g2 | o6 a2 f2 | o6 g+2 f2 "
      /*C*/ "@11 v10 q6 me4 mp5,18,200 o6 d4 f4 a2 | o6 g4 d4 o5 b2 | o6 c4 e4 g2 | "
            "o6 a4 e4 c2 ",
    "@10 t72 v10 q6 me3 "
      /*A*/ "o2 c1 | a1 | f1 | g1 "
      /*B*/ "o2 c2 c2 | c2 c2 | f2 f2 | f2 f2 "
      /*C*/ "o2 d8 a8 o3 d8 o2 a8 d8 a8 o3 d8 o2 a8 | g8 o3 d8 g8 d8 o2 g8 o3 d8 g8 d8 | "
            "o2 c8 g8 o3 c8 o2 g8 c8 g8 o3 c8 o2 g8 | a8 o3 e8 a8 e8 o2 a8 o3 e8 a8 e8 ",
    "@9 t72 v6 q8 me0,500 mv2,20 k-8 "
      /*A*/ "o5 e1 | c1 | o4 a1 | b1 "
      /*B*/ "o5 e1 | e1 | a1 | g+1 "
      /*C*/ "o5 f2 r4 f4 | o4 b2 r4 b4 | o5 e2 r4 e4 | c2 r4 c4 ",
    "@9 t72 v6 q8 me0,500 mv2,20 k0 "
      /*A*/ "o4 g1 | e1 | c1 | d1 "
      /*B*/ "o4 g1 | g1 | o5 c1 | c1 "
      /*C*/ "o4 a2 r4 a4 | o5 d2 r4 d4 | g2 r4 g4 | e2 r4 e4 ",
    "@9 t72 v6 q8 me0,500 mv2,20 k8 "
      /*A*/ "o5 c1 | o4 a1 | f1 | g1 "
      /*B*/ "o5 c1 | o4 a+1 | f1 | f1 "
      /*C*/ "o5 d2 r4 d4 | g2 r4 g4 | o6 c2 r4 c4 | o5 a2 r4 a4 ",
    "@n t72 q3 "
      /*A*/ "[ r1 ]4 "
      /*B*/ "[ v13 o1 a16 r8 a16 r2 a16 r8 a16 ]4 "
      /*C*/ "[ v13 o1 a16 r2 r8. v12 o4 d+16 r8. ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r4. r16 "
  } },
  // CHASE -- A minor. A i-bVII-i-bVI in running eighths, B i-iv-bVII-bIII, C the harmonic i-V
  { "CHASE", {
    "@w8={" WAV_PULSE25 "}@w9={" WAV_SOFTSAW "}@w10={" WAV_BASSR "}@w11={" WAV_SYNC "} "
      "@8 t168 v11 q6 me5 mp6,20,120 "
      /*A*/ "o6 a8 e8 a8 o7 c8 o6 b8 a8 e8 c8 | o6 g8 d8 g8 b8 o7 d8 o6 b8 g8 d8 | "
            "o6 a8 e8 a8 o7 c8 o6 e8 a8 o7 c8 e8 | o6 a8 f8 c8 f8 a8 o7 c8 o6 a8 f8 "
      /*B*/ "o6 a4 o7 c4 e4 o6 a4 | o6 d4 f4 a4 d4 | o6 g4 b4 o7 d4 o6 g4 | "
            "o7 c4 o6 g4 e4 c4 "
      /*C*/ "@11 v11 q8 me2 mp8,35,60 o6 a8 o7 c8 o6 b8 a8 e2 | o6 g+8 b8 o7 e8 o6 b8 g+2 | "
            "o6 a8 e8 c8 e8 a2 | o6 b8 g+8 e8 g+8 b2 ",
    "@10 t168 v11 q5 me5 "
      /*A*/ "o2 [a8]8 | [g8]8 | [a8]8 | [f8]8 "
      /*B*/ "o2 a8 o3 a8 o2 a8 o3 a8 o2 a8 o3 a8 o2 a8 o3 a8 | "
            "o2 d8 o3 d8 o2 d8 o3 d8 o2 d8 o3 d8 o2 d8 o3 d8 | "
            "o2 g8 o3 g8 o2 g8 o3 g8 o2 g8 o3 g8 o2 g8 o3 g8 | "
            "o2 c8 o3 c8 o2 c8 o3 c8 o2 c8 o3 c8 o2 c8 o3 c8 "
      /*C*/ "o2 a8 a16 a16 a8 a16 a16 a8 a16 a16 o3 e8 o2 a16 a16 | "
            "e8 e16 e16 e8 e16 e16 e8 e16 e16 b8 e16 e16 | "
            "a8 a16 a16 a8 a16 a16 a8 a16 a16 o3 e8 o2 a16 a16 | "
            "e8 e16 e16 e8 e16 e16 e8 e16 e16 b8 e16 e16 ",
    "@9 t168 v6 q8 me0 mv5,30 k-8 "
      /*A*/ "o5 c8 r8 c8 r8 c8 r8 c8 r8 | o4 b8 r8 b8 r8 b8 r8 b8 r8 | "
            "o5 c8 r8 c8 r8 c8 r8 c8 r8 | o4 a8 r8 a8 r8 a8 r8 a8 r8 "
      /*B*/ "r8 o5 c8 r8 c8 r8 c8 r8 c8 | r8 f8 r8 f8 r8 f8 r8 f8 | "
            "r8 o4 b8 r8 b8 r8 b8 r8 b8 | r8 o5 e8 r8 e8 r8 e8 r8 e8 "
      /*C*/ "o5 [c4]4 | o4 [g+4]4 | o5 [c4]4 | o4 [g+4]4 ",
    "@9 t168 v6 q8 me0 mv5,30 k0 "
      /*A*/ "o5 e8 r8 e8 r8 e8 r8 e8 r8 | d8 r8 d8 r8 d8 r8 d8 r8 | "
            "e8 r8 e8 r8 e8 r8 e8 r8 | c8 r8 c8 r8 c8 r8 c8 r8 "
      /*B*/ "r8 o5 e8 r8 e8 r8 e8 r8 e8 | r8 a8 r8 a8 r8 a8 r8 a8 | "
            "r8 d8 r8 d8 r8 d8 r8 d8 | r8 g8 r8 g8 r8 g8 r8 g8 "
      /*C*/ "o5 [e4]4 | o4 [b4]4 | o5 [e4]4 | o4 [b4]4 ",
    "@9 t168 v6 q8 me0 mv5,30 k8 "
      /*A*/ "o4 a8 r8 a8 r8 a8 r8 a8 r8 | g8 r8 g8 r8 g8 r8 g8 r8 | "
            "a8 r8 a8 r8 a8 r8 a8 r8 | f8 r8 f8 r8 f8 r8 f8 r8 "
      /*B*/ "r8 o4 a8 r8 a8 r8 a8 r8 a8 | r8 o5 d8 r8 d8 r8 d8 r8 d8 | "
            "r8 g8 r8 g8 r8 g8 r8 g8 | r8 o6 c8 r8 c8 r8 c8 r8 c8 "
      /*C*/ "o4 [a4]4 | [e4]4 | [a4]4 | [e4]4 ",
    "@n t168 q3 "
      /*A*/ "[ [ v13 o1 a16 v9 o6 d+16 d+16 d+16 v12 o4 d+16 v9 o6 d+16 d+16 d+16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ v13 o1 a16 v9 o6 d+16 d+16 d+16 v12 o4 d+16 v9 o6 d+16 d+16 d+16 v13 o1 a16 v9 o6 d+16 d+16 v13 o1 a16 v12 o4 d+16 v9 o6 d+16 d+16 d+16 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ [ v13 o1 a16 v9 o6 d+16 v13 o1 a16 v9 o6 d+16 v12 o4 d+16 r16 v9 o6 d+16 r16 ]2 ]3 | "
            "v13 o5 c16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 "
  } },
  // BOSS RUSH -- D minor. A the phrygian-dominant i-bII, B i-bVI-bVII-i, C i-v-bII-V
  { "BOSS RUSH", {
    "@w8={" WAV_METAL "}@w9={" WAV_SYNC "}@w10={" WAV_SOFTSAW "}@w11={" WAV_REED "} "
      "@8 t144 v11 q6 me4 mp7,30,100 "
      /*A*/ "o5 d4 f4 d4 a4 | o5 d+4 g4 d+4 a+4 | o6 d4 c4 a4 f4 | o6 d+4 d4 o5 a+4 g4 "
      /*B*/ "o5 d2 a2 | o5 a+2 f2 | o6 c2 g2 | o6 d2 a2 "
      /*C*/ "@11 v11 q8 me0 mp4,45,200 o6 d8 e8 f8 g8 a4 d4 | o6 c8 o5 b8 a8 g8 a4 e4 | "
            "o6 d+8 d8 c8 o5 a+8 g4 d+4 | o6 c+8 e8 a8 e8 c+4 a4 ",
    "@10 t144 v11 q5 me5 "
      /*A*/ "o2 [d16]16 | [d+16]16 | [d16]16 | [d+16]16 "
      /*B*/ "o2 d4 d4 a4 d4 | a+4 a+4 o3 f4 o2 a+4 | c4 c4 g4 c4 | d4 d4 a4 d4 "
      /*C*/ "o2 d8 d16 d16 d8 d16 d16 d8 d16 d16 a8 d16 d16 | "
            "a8 a16 a16 a8 a16 a16 a8 a16 a16 o3 e8 o2 a16 a16 | "
            "d+8 d+16 d+16 d+8 d+16 d+16 d+8 d+16 d+16 a+8 d+16 d+16 | "
            "a8 a16 a16 a8 a16 a16 a8 a16 a16 o3 e8 o2 a16 a16 ",
    "@9 t144 v6 q8 me0 mv6,35 k-11 "
      /*A*/ "o5 f8 r8 f8 r8 f8 r8 f8 r8 | g8 r8 g8 r8 g8 r8 g8 r8 | "
            "f8 r8 f8 r8 f8 r8 f8 r8 | g8 r8 g8 r8 g8 r8 g8 r8 "
      /*B*/ "o5 f1 | d1 | e1 | f1 "
      /*C*/ "r8 o5 f8 r8 f8 r8 f8 r8 f8 | r8 c8 r8 c8 r8 c8 r8 c8 | "
            "r8 o4 g8 r8 g8 r8 g8 r8 g8 | r8 c+8 r8 c+8 r8 c+8 r8 c+8 ",
    "@9 t144 v6 q8 me0 mv6,35 k0 "
      /*A*/ "o4 a8 r8 a8 r8 a8 r8 a8 r8 | a+8 r8 a+8 r8 a+8 r8 a+8 r8 | "
            "a8 r8 a8 r8 a8 r8 a8 r8 | a+8 r8 a+8 r8 a+8 r8 a+8 r8 "
      /*B*/ "o4 a1 | f1 | g1 | a1 "
      /*C*/ "r8 o4 a8 r8 a8 r8 a8 r8 a8 | r8 e8 r8 e8 r8 e8 r8 e8 | "
            "r8 a+8 r8 a+8 r8 a+8 r8 a+8 | r8 e8 r8 e8 r8 e8 r8 e8 ",
    "@9 t144 v6 q8 me0 mv6,35 k11 "
      /*A*/ "o5 d8 r8 d8 r8 d8 r8 d8 r8 | d+8 r8 d+8 r8 d+8 r8 d+8 r8 | "
            "d8 r8 d8 r8 d8 r8 d8 r8 | d+8 r8 d+8 r8 d+8 r8 d+8 r8 "
      /*B*/ "o5 d1 | o4 a+1 | o5 c1 | d1 "
      /*C*/ "r8 o5 d8 r8 d8 r8 d8 r8 d8 | r8 o4 a8 r8 a8 r8 a8 r8 a8 | "
            "r8 d+8 r8 d+8 r8 d+8 r8 d+8 | r8 a8 r8 a8 r8 a8 r8 a8 ",
    "@n t144 q3 "
      /*A*/ "[ v13 o1 a16 v9 o6 d+16 d+16 d+16 v12 o4 d+16 v9 o6 d+16 d+16 d+16 v13 o1 a16 v9 o6 d+16 d+16 v13 o1 a16 v12 o4 d+16 v9 o6 d+16 d+16 d+16 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ [ v13 o1 a16 v9 o6 d+16 v13 o1 a16 v9 o6 d+16 v12 o4 d+16 r16 v9 o6 d+16 r16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 v12 o4 d+16 v8 d+16 v12 d+16 v8 d+16 v12 d+16 v8 d+16 v12 d+16 v8 d+16 "
  } },
  // OVERDRIVE -- E minor. A i-bVII-bVI-bVII, B the minor plagal i-iv-v-i, C bIII-bVII-iv-i
  { "OVERDRIVE", {
    "@w8={" WAV_SOFTSAW "}@w9={" WAV_PULSE25 "}@w10={" WAV_BASSR "}@w11={" WAV_SYNC "} "
      "@8 t172 v11 q6 me4 mp6,25,120 "
      /*A*/ "o6 e8 g8 b8 e8 b4 g4 | o6 d8 f+8 a8 d8 a4 f+4 | o6 c8 e8 g8 c8 g4 e4 | "
            "o6 d8 a8 f+8 d8 f+4 a4 "
      /*B*/ "o6 b2 e2 | o6 a2 e2 | o6 b2 f+2 | o7 e2 o6 b2 "
      /*C*/ "@11 v11 q8 me1 mp7,40,80 o6 g4 b4 o7 d2 | o6 f+4 a4 o7 d2 | o6 e4 a4 o7 c2 | "
            "o6 b4 o7 e4 g2 ",
    "@10 t172 v11 q5 me6 "
      /*A*/ "o2 [e8]8 | [d8]8 | [c8]8 | [d8]8 "
      /*B*/ "o2 e4 e4 b4 e4 | a4 a4 o3 e4 o2 a4 | b4 b4 o3 f+4 o2 b4 | e4 e4 b4 e4 "
      /*C*/ "o2 g8 o3 g8 o2 g8 o3 g8 o2 g8 o3 g8 o2 g8 o3 g8 | "
            "o2 d8 o3 d8 o2 d8 o3 d8 o2 d8 o3 d8 o2 d8 o3 d8 | "
            "o2 a8 o3 a8 o2 a8 o3 a8 o2 a8 o3 a8 o2 a8 o3 a8 | "
            "o2 e8 o3 e8 o2 e8 o3 e8 o2 e8 o3 e8 o2 e8 o3 e8 ",
    "@9 t172 v6 q8 me0 mv5,28 k-9 "
      /*A*/ "o4 g8 r8 g8 r8 g8 r8 g8 r8 | f+8 r8 f+8 r8 f+8 r8 f+8 r8 | "
            "e8 r8 e8 r8 e8 r8 e8 r8 | f+8 r8 f+8 r8 f+8 r8 f+8 r8 "
      /*B*/ "o4 g1 | o5 c1 | d1 | g1 "
      /*C*/ "r8 o4 b8 r8 b8 r8 b8 r8 b8 | r8 f+8 r8 f+8 r8 f+8 r8 f+8 | "
            "r8 c8 r8 c8 r8 c8 r8 c8 | r8 g8 r8 g8 r8 g8 r8 g8 ",
    "@9 t172 v6 q8 me0 mv5,28 k0 "
      /*A*/ "o4 b8 r8 b8 r8 b8 r8 b8 r8 | a8 r8 a8 r8 a8 r8 a8 r8 | "
            "g8 r8 g8 r8 g8 r8 g8 r8 | a8 r8 a8 r8 a8 r8 a8 r8 "
      /*B*/ "o4 b1 | o5 e1 | f+1 | b1 "
      /*C*/ "r8 o5 d8 r8 d8 r8 d8 r8 d8 | r8 o4 a8 r8 a8 r8 a8 r8 a8 | "
            "r8 e8 r8 e8 r8 e8 r8 e8 | r8 b8 r8 b8 r8 b8 r8 b8 ",
    "@9 t172 v6 q8 me0 mv5,28 k9 "
      /*A*/ "o5 e8 r8 e8 r8 e8 r8 e8 r8 | d8 r8 d8 r8 d8 r8 d8 r8 | "
            "c8 r8 c8 r8 c8 r8 c8 r8 | d8 r8 d8 r8 d8 r8 d8 r8 "
      /*B*/ "o5 e1 | a1 | b1 | e1 "
      /*C*/ "r8 o4 g8 r8 g8 r8 g8 r8 g8 | r8 d8 r8 d8 r8 d8 r8 d8 | "
            "r8 a8 r8 a8 r8 a8 r8 a8 | r8 e8 r8 e8 r8 e8 r8 e8 ",
    "@n t172 q3 "
      /*A*/ "[ [ v13 o1 a16 v9 o6 d+16 d+16 d+16 v12 o4 d+16 v9 o6 d+16 d+16 d+16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ v13 o1 a16 v9 o6 d+16 d+16 d+16 v12 o4 d+16 v9 o6 d+16 d+16 d+16 v13 o1 a16 v9 o6 d+16 d+16 v13 o1 a16 v12 o4 d+16 v9 o6 d+16 d+16 d+16 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ [ v13 o1 a16 v8 o4 d+16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 v13 o1 a16 ]2 ]3 | "
            "o5 c16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 "
  } },
  // IRONWORKS -- F# minor. A a i5-bVII5-bVI5 riff, B iv-i-bVII-V, C the tritone i-bV-i-bVI
  { "IRONWORKS", {
    "@w8={" WAV_METAL "}@w9={" WAV_HOLLOW "}@w10={" WAV_SOFTSAW "}@w11={" WAV_PULSE12 "} "
      "@8 t128 v11 q5 me6 mp4,15,150 "
      /*A*/ "o5 f+8 r8 f+8 o6 c+8 o5 f+4 a4 | o5 f+8 r8 f+8 a8 o6 c+4 o5 f+4 | "
            "o5 e8 r8 e8 b8 o6 e4 o5 b4 | o5 d8 r8 d8 a8 o6 d4 o5 a4 "
      /*B*/ "o6 f+4 d4 o5 b2 | o6 c+4 a4 f+2 | o6 b4 g+4 e2 | o6 g+4 f4 c+2 "
      /*C*/ "@11 v11 q8 me0 mp3,50,250 o6 f+2 c+2 | o6 g2 c2 | o6 a2 f+2 | o6 f+2 d2 ",
    "@10 t128 v11 q5 me6 "
      /*A*/ "o2 [f+16]16 | [f+16]16 | [e16]16 | [d16]16 "
      /*B*/ "o2 [b4]4 | [f+4]4 | [e4]4 | [c+4]4 "
      /*C*/ "o2 f+2 o3 f+2 | o2 c2 o3 c2 | o2 f+2 o3 f+2 | o2 d2 o3 d2 ",
    "@9 t128 v5 q8 me0 mv4,32 k-11 "
      /*A*/ "o5 c+8 r8 c+8 r8 c+8 r8 c+8 r8 | c+8 r8 c+8 r8 c+8 r8 c+8 r8 | "
            "o4 b8 r8 b8 r8 b8 r8 b8 r8 | a8 r8 a8 r8 a8 r8 a8 r8 "
      /*B*/ "o5 d1 | o4 a1 | g+1 | f1 "
      /*C*/ "o4 a2 a2 | e2 e2 | a2 a2 | f+2 f+2 ",
    "@9 t128 v5 q8 me0 mv4,32 k0 "
      /*A*/ "o4 f+8 r8 f+8 r8 f+8 r8 f+8 r8 | f+8 r8 f+8 r8 f+8 r8 f+8 r8 | "
            "e8 r8 e8 r8 e8 r8 e8 r8 | d8 r8 d8 r8 d8 r8 d8 r8 "
      /*B*/ "o4 f+1 | c+1 | b1 | g+1 "
      /*C*/ "o5 c+2 c+2 | o4 g2 g2 | c+2 c+2 | a2 a2 ",
    "@9 t128 v5 q8 me0 mv4,32 k11 "
      /*A*/ "o5 c+8 r8 c+8 r8 c+8 r8 c+8 r8 | c+8 r8 c+8 r8 c+8 r8 c+8 r8 | "
            "o4 b8 r8 b8 r8 b8 r8 b8 r8 | a8 r8 a8 r8 a8 r8 a8 r8 "
      /*B*/ "o4 b1 | f+1 | e1 | c+1 "
      /*C*/ "o4 f+2 f+2 | c2 c2 | f+2 f+2 | d2 d2 ",
    "@n t128 q3 "
      /*A*/ "[ [ v13 o1 a16 v8 o4 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ [ v11 o3 a16 r16 o2 d+16 r16 ]4 ]3 | "
            "v13 o5 c16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 "
  } },
  // RAMPAGE -- C minor. A i-bVI-bVII-v, B i-bIII-iv-bVI, C i-V-bVI-IV
  { "RAMPAGE", {
    "@w8={" WAV_SYNC "}@w9={" WAV_SOFTSAW "}@w10={" WAV_BASSR "}@w11={" WAV_METAL "} "
      "@8 t160 v11 q6 me4 mp6,28,120 "
      /*A*/ "o6 c4 d+4 g4 d+4 | o6 c4 d+4 g+4 d+4 | o6 d4 f4 a+4 f4 | o6 d4 g4 a+4 g4 "
      /*B*/ "o6 g2 c2 | o6 a+2 d+2 | o7 c2 o6 f2 | o6 d+2 g+2 "
      /*C*/ "@11 v11 q8 me2 mp8,30,80 o6 c8 g8 d+8 c8 g4 d+4 | o6 d8 b8 g8 d8 b4 g4 | "
            "o6 c8 g+8 d+8 c8 g+4 d+4 | o6 c8 a8 f8 c8 a4 f4 ",
    "@10 t160 v11 q5 me5 "
      /*A*/ "o2 [c8]8 | [g+8]8 | [a+8]8 | [g8]8 "
      /*B*/ "o2 c4 c4 g4 c4 | d+4 d+4 a+4 d+4 | f4 f4 o3 c4 o2 f4 | g+4 g+4 o3 d+4 o2 g+4 "
      /*C*/ "o2 c8 c16 c16 c8 c16 c16 c8 c16 c16 g8 c16 c16 | "
            "g8 g16 g16 g8 g16 g16 g8 g16 g16 o3 d8 o2 g16 g16 | "
            "g+8 g+16 g+16 g+8 g+16 g+16 g+8 g+16 g+16 o3 d+8 o2 g+16 g+16 | "
            "f8 f16 f16 f8 f16 f16 f8 f16 f16 o3 c8 o2 f16 f16 ",
    "@9 t160 v6 q8 me0 mv5,30 k-10 "
      /*A*/ "o5 d+8 r8 d+8 r8 d+8 r8 d+8 r8 | c8 r8 c8 r8 c8 r8 c8 r8 | "
            "d8 r8 d8 r8 d8 r8 d8 r8 | o4 a+8 r8 a+8 r8 a+8 r8 a+8 r8 "
      /*B*/ "o5 d+1 | g1 | g+1 | o6 c1 "
      /*C*/ "r8 o5 d+8 r8 d+8 r8 d+8 r8 d+8 | r8 o4 b8 r8 b8 r8 b8 r8 b8 | "
            "r8 o5 c8 r8 c8 r8 c8 r8 c8 | r8 o4 a8 r8 a8 r8 a8 r8 a8 ",
    "@9 t160 v6 q8 me0 mv5,30 k0 "
      /*A*/ "o4 g8 r8 g8 r8 g8 r8 g8 r8 | d+8 r8 d+8 r8 d+8 r8 d+8 r8 | "
            "f8 r8 f8 r8 f8 r8 f8 r8 | d8 r8 d8 r8 d8 r8 d8 r8 "
      /*B*/ "o4 g1 | a+1 | o5 c1 | d+1 "
      /*C*/ "r8 o4 g8 r8 g8 r8 g8 r8 g8 | r8 d8 r8 d8 r8 d8 r8 d8 | "
            "r8 d+8 r8 d+8 r8 d+8 r8 d+8 | r8 c8 r8 c8 r8 c8 r8 c8 ",
    "@9 t160 v6 q8 me0 mv5,30 k10 "
      /*A*/ "o5 c8 r8 c8 r8 c8 r8 c8 r8 | o4 g+8 r8 g+8 r8 g+8 r8 g+8 r8 | "
            "a+8 r8 a+8 r8 a+8 r8 a+8 r8 | g8 r8 g8 r8 g8 r8 g8 r8 "
      /*B*/ "o5 c1 | d+1 | f1 | g+1 "
      /*C*/ "r8 o5 c8 r8 c8 r8 c8 r8 c8 | r8 o4 g8 r8 g8 r8 g8 r8 g8 | "
            "r8 g+8 r8 g+8 r8 g+8 r8 g+8 | r8 f8 r8 f8 r8 f8 r8 f8 ",
    "@n t160 q3 "
      /*A*/ "[ [ v13 o1 a16 v9 o6 d+16 d+16 d+16 v12 o4 d+16 v9 o6 d+16 d+16 d+16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ v13 o1 a16 v9 o6 d+16 d+16 d+16 v12 o4 d+16 v9 o6 d+16 d+16 d+16 v13 o1 a16 v9 o6 d+16 d+16 v13 o1 a16 v12 o4 d+16 v9 o6 d+16 d+16 d+16 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ [ v13 o1 a16 v9 o6 d+16 v13 o1 a16 v9 o6 d+16 v12 o4 d+16 r16 v9 o6 d+16 r16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 v12 o4 d+16 v8 d+16 v12 d+16 v8 d+16 v12 d+16 v8 d+16 v12 d+16 v8 d+16 "
  } },
  // LAST STAND -- B minor. A i-V-bVI-III, B iv-bVII-bIII-bVI, C i-bII-bVII-i
  { "LAST STAND", {
    "@w8={" WAV_STRING "}@w9={" WAV_REED "}@w10={" WAV_SOFTSAW "}@w11={" WAV_FIFTH "} "
      "@8 t126 v11 q8 me1 mp5,35,250 "
      /*A*/ "o6 b2 f+2 | o6 a+2 f+2 | o6 b2 g2 | o6 a2 f+2 "
      /*B*/ "o6 e4 g4 b2 | o6 e4 c+4 a2 | o6 f+4 a4 o7 d2 | o6 g4 b4 o7 d2 "
      /*C*/ "@11 v11 q6 me3 mp7,25,120 o6 f+4 b4 o7 d2 | o7 e4 c4 o6 g2 | o7 c+4 o6 a4 e2 | "
            "o6 b1 ",
    "@10 t126 v11 q6 me3 "
      /*A*/ "o2 b2 b2 | f+2 f+2 | g2 g2 | d2 d2 "
      /*B*/ "o2 [e4]4 | [a4]4 | [d4]4 | [g4]4 "
      /*C*/ "o2 b4 b4 o3 f+4 o2 b4 | c4 c4 g4 c4 | a4 a4 o3 e4 o2 a4 | b4 b4 o3 f+4 o2 b4 ",
    "@9 t126 v6 q8 me0,300 mv3,25 k-10 "
      /*A*/ "o5 d1 | o4 a+1 | b1 | f+1 "
      /*B*/ "o4 [g4]4 | [c+4]4 | [f+4]4 | [b4]4 "
      /*C*/ "o5 d4. d4. d4 | e4. e4. e4 | c+4. c+4. c+4 | d4. d4. d4 ",
    "@9 t126 v6 q8 me0,300 mv3,25 k0 "
      /*A*/ "o4 f+1 | c+1 | d1 | a1 "
      /*B*/ "o4 [b4]4 | o5 [e4]4 | [a4]4 | [d4]4 "
      /*C*/ "o4 f+4. f+4. f+4 | g4. g4. g4 | e4. e4. e4 | f+4. f+4. f+4 ",
    "@9 t126 v6 q8 me0,300 mv3,25 k10 "
      /*A*/ "o4 b1 | f+1 | g1 | d1 "
      /*B*/ "o5 [e4]4 | [a4]4 | [d4]4 | [g4]4 "
      /*C*/ "o4 b4. b4. b4 | o5 c4. c4. c4 | o4 a4. a4. a4 | b4. b4. b4 ",
    "@n t126 q3 "
      /*A*/ "[ v13 o1 a16 r4. r16 v12 o4 d+16 r4. r16 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ [ v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 ]4 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 v12 o4 d+16 v8 d+16 v12 d+16 v8 d+16 v12 d+16 v8 d+16 v12 d+16 v8 d+16 "
  } },
  // NEON CITY -- A minor. A a i7-iv7 funk vamp, B bVImaj7-bVIImaj7-i7, C iim7b5-V7-i7
  { "NEON CITY", {
    "@w8={" WAV_VOX "}@w9={" WAV_PULSE25 "}@w10={" WAV_BASSR "}@w11={" WAV_REED "} "
      "@8 t112 v11 q6 me4 mp5,25,150 "
      /*A*/ "o6 a8 g8 e8 c8 e4 a4 | o6 d8 c8 a8 f8 a4 d4 | o6 c8 e8 g8 a8 g4 e4 | "
            "o6 f8 a8 o7 c8 o6 a8 f4 d4 "
      /*B*/ "o6 e4 c4 a2 | o6 f+4 d4 b2 | o6 g4 e4 c2 | o6 a1 "
      /*C*/ "@11 v11 q8 me1 mp6,35,150 o6 b4 a4 f4 d4 | o6 b4 g+4 e2 | o6 a8 g8 e8 c8 a2 | "
            "o6 e2 a2 ",
    "@10 t112 v11 q5 me6 "
      /*A*/ "o2 a8 r8 a8 a8 r8 a8 o3 e8 r8 | o2 d8 r8 d8 d8 r8 d8 a8 r8 | "
            "a8 r8 a8 a8 r8 a8 o3 e8 r8 | o2 d8 r8 d8 d8 r8 d8 a8 r8 "
      /*B*/ "o2 f4 a4 o3 c4 o2 f+4 | g4 b4 o3 d4 o2 g+4 | a4 o3 c4 e4 o2 g+4 | "
            "a4 o3 c4 e4 o2 f+4 "
      /*C*/ "o2 b8 o3 f8 o2 a8 o3 b8 o2 a8 o3 f8 d8 f8 | "
            "o2 e8 b8 d8 o3 e8 o2 d8 b8 g+8 b8 | a8 o3 e8 o2 g8 o3 a8 o2 g8 o3 e8 c8 e8 | "
            "o2 a8 o3 e8 o2 g8 o3 a8 o2 g8 o3 e8 c8 e8 ",
    "@9 t112 v6 q8 me0 mv4,28 k-9 "
      /*A*/ "r8 o5 c8 r8 c8 r8 c8 r8 c8 | r8 f8 r8 f8 r8 f8 r8 f8 | "
            "r8 c8 r8 c8 r8 c8 r8 c8 | r8 f8 r8 f8 r8 f8 r8 f8 "
      /*B*/ "o4 a2 a2 | b2 b2 | o5 c2 c2 | c2 c2 "
      /*C*/ "o5 d8 r8 d8 r8 d8 r8 d8 r8 | o4 g+8 r8 g+8 r8 g+8 r8 g+8 r8 | "
            "o5 c8 r8 c8 r8 c8 r8 c8 r8 | c8 r8 c8 r8 c8 r8 c8 r8 ",
    "@9 t112 v6 q8 me0 mv4,28 k0 "
      /*A*/ "r8 o5 e8 r8 e8 r8 e8 r8 e8 | r8 a8 r8 a8 r8 a8 r8 a8 | "
            "r8 e8 r8 e8 r8 e8 r8 e8 | r8 a8 r8 a8 r8 a8 r8 a8 "
      /*B*/ "o5 c2 c2 | d2 d2 | e2 e2 | e2 e2 "
      /*C*/ "o5 f8 r8 f8 r8 f8 r8 f8 r8 | o4 b8 r8 b8 r8 b8 r8 b8 r8 | "
            "o5 e8 r8 e8 r8 e8 r8 e8 r8 | e8 r8 e8 r8 e8 r8 e8 r8 ",
    "@9 t112 v6 q8 me0 mv4,28 k9 "
      /*A*/ "r8 o4 g8 r8 g8 r8 g8 r8 g8 | r8 o5 c8 r8 c8 r8 c8 r8 c8 | "
            "r8 o4 g8 r8 g8 r8 g8 r8 g8 | r8 o5 c8 r8 c8 r8 c8 r8 c8 "
      /*B*/ "o5 e2 e2 | f+2 f+2 | g2 g2 | g2 g2 "
      /*C*/ "o4 a8 r8 a8 r8 a8 r8 a8 r8 | o5 d8 r8 d8 r8 d8 r8 d8 r8 | "
            "g8 r8 g8 r8 g8 r8 g8 r8 | g8 r8 g8 r8 g8 r8 g8 r8 ",
    "@n t112 q3 "
      /*A*/ "[ [ v13 o1 a16 v8 o4 d+16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 v13 o1 a16 ]2 ]3 | "
            "a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ [ v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 v13 o1 a16 a16 v9 o6 d+16 r16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 ]3 | "
            "v13 o5 c16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 "
  } },
  // BLUE ROOM -- F blues. A the I7-IV7-I7-V7 first four bars, B the IV7 turn, C the V7-IV7-I7 fall
  { "BLUE ROOM", {
    "@w8={" WAV_REED "}@w9={" WAV_ORGAN "}@w10={" WAV_BASSR "}@w11={" WAV_VOX "} "
      "@8 t96 v11 q6 me4 mp5,40,150 "
      /*A*/ "o6 c4 o5 a+8 a8 f4 g+4 | o6 d4 c8 o5 a+8 f4 g+4 | o6 f4 d+8 c8 a+4 g+4 | "
            "o6 g4 e8 d8 c4 o5 a+4 "
      /*B*/ "o6 a+2 d4 f4 | o6 d4 f4 a+2 | o6 c2 a4 f4 | o6 f2 d+4 c4 "
      /*C*/ "@11 v11 q8 me2 mp7,30,100 o6 g4 a+4 g4 e4 | o6 f4 g+4 f4 d4 | "
            "o6 c4 d+4 c4 o5 a4 | o6 e4 g4 a+4 g4 ",
    "@10 t96 v11 q5 me5 "
      /*A*/ "o2 f8. o3 c16 o2 f8. o3 c16 o2 f8. o3 c16 o2 f8. o3 c16 | "
            "o2 a+8. o3 f16 o2 a+8. o3 f16 o2 a+8. o3 f16 o2 a+8. o3 f16 | "
            "o2 f8. o3 c16 o2 f8. o3 c16 o2 f8. o3 c16 o2 f8. o3 c16 | "
            "o2 c8. g16 c8. g16 c8. g16 c8. g16 "
      /*B*/ "o2 a+8. o3 f16 o2 a+8. o3 f16 o2 a+8. o3 f16 o2 a+8. o3 f16 | "
            "o2 a+8. o3 f16 o2 a+8. o3 f16 o2 a+8. o3 f16 o2 a+8. o3 f16 | "
            "o2 f8. o3 c16 o2 f8. o3 c16 o2 f8. o3 c16 o2 f8. o3 c16 | "
            "o2 f8. o3 c16 o2 f8. o3 c16 o2 f8. o3 c16 o2 f8. o3 c16 "
      /*C*/ "o2 c4 e4 g4 a4 | a+4 o3 d4 f4 o2 f+4 | f4 a4 o3 c4 o2 c+4 | c4 e4 g4 o1 b4 ",
    "@9 t96 v6 q8 me0 mv3,25 k-10 "
      /*A*/ "o4 a8 r8 a8 r8 a8 r8 a8 r8 | o5 d8 r8 d8 r8 d8 r8 d8 r8 | "
            "o4 a8 r8 a8 r8 a8 r8 a8 r8 | e8 r8 e8 r8 e8 r8 e8 r8 "
      /*B*/ "o5 d2 d2 | d2 d2 | o4 a2 a2 | a2 a2 "
      /*C*/ "o5 [e4]4 | [d4]4 | o4 [a4]4 | [e4]4 ",
    "@9 t96 v6 q8 me0 mv3,25 k0 "
      /*A*/ "o5 c8 r8 c8 r8 c8 r8 c8 r8 | f8 r8 f8 r8 f8 r8 f8 r8 | "
            "c8 r8 c8 r8 c8 r8 c8 r8 | o4 g8 r8 g8 r8 g8 r8 g8 r8 "
      /*B*/ "o5 f2 f2 | f2 f2 | c2 c2 | c2 c2 "
      /*C*/ "o4 [g4]4 | [f4]4 | [c4]4 | [g4]4 ",
    "@9 t96 v6 q8 me0 mv3,25 k10 "
      /*A*/ "o5 d+8 r8 d+8 r8 d+8 r8 d+8 r8 | g+8 r8 g+8 r8 g+8 r8 g+8 r8 | "
            "d+8 r8 d+8 r8 d+8 r8 d+8 r8 | o4 a+8 r8 a+8 r8 a+8 r8 a+8 r8 "
      /*B*/ "o4 g+2 g+2 | g+2 g+2 | d+2 d+2 | d+2 d+2 "
      /*C*/ "o4 [a+4]4 | [g+4]4 | [d+4]4 | [a+4]4 ",
    "@n t96 q3 "
      /*A*/ "[ [ v13 o1 a16 r8 v9 o6 d+16 v12 o4 d+16 r8 v9 o6 d+16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ [ v13 o1 a16 r8 v9 o6 d+16 v12 o4 d+16 r8 v9 o6 d+16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ [ v13 o1 a16 v8 o4 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 v12 o4 d+16 v8 d+16 v12 d+16 v8 d+16 v12 d+16 v8 d+16 v12 d+16 v8 d+16 "
  } },
  // CAFE -- C major bossa. A Imaj7-VI7-iim7-V7, B iiim7-VI7-iim7-V7, C IVmaj7-iv6-Imaj7-V7
  { "CAFE", {
    "@w8={" WAV_SINE "}@w9={" WAV_STRING "}@w10={" WAV_BASSR "}@w11={" WAV_PIANO "} "
      "@8 t118 v11 q7 me3 mp4,22,250 "
      /*A*/ "o6 e4 g4 b2 | o6 c+4 e4 g2 | o6 d4 f4 a2 | o6 b4 f4 d2 "
      /*B*/ "o6 b4 g4 e2 | o6 g4 e4 c+2 | o6 a4 f4 d2 | o6 f4 d4 o5 b2 "
      /*C*/ "@11 v11 q6 me6 mp0 o6 a4 e4 c2 | o6 g+4 d4 c2 | o6 e4 b4 g2 | o6 g4 f4 d2 ",
    "@10 t118 v11 q6 me4 "
      /*A*/ "o2 c8 r8 c8 c8 r8 c8 g8 r8 | a8 r8 a8 a8 r8 a8 o3 e8 r8 | "
            "o2 d8 r8 d8 d8 r8 d8 a8 r8 | g8 r8 g8 g8 r8 g8 o3 d8 r8 "
      /*B*/ "o2 e2 b2 | a2 o3 e2 | o2 d2 a2 | g2 o3 d2 "
      /*C*/ "o2 f8 o3 c8 o2 e8 o3 f8 o2 e8 o3 c8 o2 a8 o3 c8 | "
            "o2 f8 o3 c8 o2 d8 o3 f8 o2 d8 o3 c8 o2 g+8 o3 c8 | "
            "o2 c8 g8 b8 o3 c8 o2 b8 g8 e8 g8 | "
            "g8 o3 d8 o2 f8 o3 g8 o2 f8 o3 d8 o2 b8 o3 d8 ",
    "@9 t118 v6 q8 me0,250 mv3,22 k-8 "
      /*A*/ "r8 o5 e8 r8 e8 r8 e8 r8 e8 | r8 c+8 r8 c+8 r8 c+8 r8 c+8 | "
            "r8 f8 r8 f8 r8 f8 r8 f8 | r8 o4 b8 r8 b8 r8 b8 r8 b8 "
      /*B*/ "o4 g8 r8 g8 r8 g8 r8 g8 r8 | c+8 r8 c+8 r8 c+8 r8 c+8 r8 | "
            "f8 r8 f8 r8 f8 r8 f8 r8 | b8 r8 b8 r8 b8 r8 b8 r8 "
      /*C*/ "o4 a2 a2 | g+2 g+2 | e2 e2 | b2 b2 ",
    "@9 t118 v6 q8 me0,250 mv3,22 k0 "
      /*A*/ "r8 o4 g8 r8 g8 r8 g8 r8 g8 | r8 e8 r8 e8 r8 e8 r8 e8 | "
            "r8 a8 r8 a8 r8 a8 r8 a8 | r8 o5 d8 r8 d8 r8 d8 r8 d8 "
      /*B*/ "o4 b8 r8 b8 r8 b8 r8 b8 r8 | o5 e8 r8 e8 r8 e8 r8 e8 r8 | "
            "a8 r8 a8 r8 a8 r8 a8 r8 | d8 r8 d8 r8 d8 r8 d8 r8 "
      /*C*/ "o5 c2 c2 | c2 c2 | o4 g2 g2 | d2 d2 ",
    "@9 t118 v6 q8 me0,250 mv3,22 k8 "
      /*A*/ "r8 o4 b8 r8 b8 r8 b8 r8 b8 | r8 g8 r8 g8 r8 g8 r8 g8 | "
            "r8 o5 c8 r8 c8 r8 c8 r8 c8 | r8 f8 r8 f8 r8 f8 r8 f8 "
      /*B*/ "o5 d8 r8 d8 r8 d8 r8 d8 r8 | g8 r8 g8 r8 g8 r8 g8 r8 | "
            "o6 c8 r8 c8 r8 c8 r8 c8 r8 | o5 f8 r8 f8 r8 f8 r8 f8 r8 "
      /*C*/ "o5 e2 e2 | d2 d2 | o4 b2 b2 | f2 f2 ",
    "@n t118 q3 "
      /*A*/ "[ [ v9 o6 d+16 a16 d+16 a16 ]4 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ [ v9 o6 a16 r16 a16 r16 v12 o4 d+16 r16 v9 o6 a16 r16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ [ v9 o6 d+16 a16 d+16 a16 ]4 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r4. r16 "
  } },
  // SLOW JAM -- Eb major. A Imaj7-iiim7-vi7-V7, B IVmaj7-V7-iiim7-vi7, C iim7-V7-Imaj7-VI7
  { "SLOW JAM", {
    "@w8={" WAV_VOX "}@w9={" WAV_STRING "}@w10={" WAV_BASSR "}@w11={" WAV_SINE "} "
      "@8 t84 v11 q8 me1 mp4,35,300 "
      /*A*/ "o6 g2 a+2 | o6 a+2 d2 | o6 g2 d+2 | o6 f2 d2 "
      /*B*/ "o6 c4 g4 d+2 | o6 d4 f4 a+2 | o6 f4 d4 a+2 | o6 d+4 g4 a+2 "
      /*C*/ "@11 v10 q6 me5 mp6,20,150 o6 g+4 f4 c2 | o6 a+4 g+4 f2 | o6 g4 d4 a+2 | "
            "o6 e4 a+4 g2 ",
    "@10 t84 v11 q6 me3 "
      /*A*/ "o2 d+2 d+2 | g2 g2 | c2 c2 | a+2 a+2 "
      /*B*/ "o2 g+8 o3 d+8 o2 g8 o3 g+8 o2 g8 o3 d+8 c8 d+8 | "
            "o2 a+8 o3 f8 o2 g+8 o3 a+8 o2 g+8 o3 f8 d8 f8 | "
            "o2 g8 o3 d8 o2 f8 o3 g8 o2 f8 o3 d8 o2 a+8 o3 d8 | "
            "o2 c8 g8 a+8 o3 c8 o2 a+8 g8 d+8 g8 "
      /*C*/ "o2 f4 g+4 o3 c4 o2 a4 | a+4 o3 d4 f4 o2 e4 | d+4 g4 a+4 c+4 | c4 e4 g4 e4 ",
    "@9 t84 v6 q8 me0,400 mv3,26 k-9 "
      /*A*/ "o4 g1 | a+1 | o5 d+1 | d1 "
      /*B*/ "o5 c2 c2 | d2 d2 | o4 a+2 a+2 | o5 d+2 d+2 "
      /*C*/ "o4 g+2 r4 g+4 | d2 r4 d4 | g2 r4 g4 | e2 r4 e4 ",
    "@9 t84 v6 q8 me0,400 mv3,26 k0 "
      /*A*/ "o4 a+1 | o5 d1 | g1 | f1 "
      /*B*/ "o5 d+2 d+2 | f2 f2 | d2 d2 | g2 g2 "
      /*C*/ "o5 c2 r4 c4 | f2 r4 f4 | a+2 r4 a+4 | g2 r4 g4 ",
    "@9 t84 v6 q8 me0,400 mv3,26 k9 "
      /*A*/ "o5 d1 | f1 | a+1 | g+1 "
      /*B*/ "o4 g2 g2 | g+2 g+2 | f2 f2 | a+2 a+2 "
      /*C*/ "o5 d+2 r4 d+4 | g+2 r4 g+4 | d2 r4 d4 | o4 a+2 r4 a+4 ",
    "@n t84 q3 "
      /*A*/ "[ v13 o1 a16 r4 r16 v8 o4 d+16 r16 v12 d+16 r4 r16 v8 d+16 r16 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ v13 o1 a16 r4. r16 v12 o4 d+16 r4 r16 v9 o6 d+16 r16 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ [ v9 o6 a16 r16 a16 r16 v12 o4 d+16 r16 v9 o6 a16 r16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r4. r16 "
  } },
  // RAINY DAY -- A minor. A im7-iv7-bVII7-bIIImaj7, B iim7b5-V7-i, C bVImaj7-V7-i
  { "RAINY DAY", {
    "@w8={" WAV_PIANO "}@w9={" WAV_SINE "}@w10={" WAV_BASSR "}@w11={" WAV_VOX "} "
      "@8 t92 v11 q7 me5 mp3,20,250 "
      /*A*/ "o6 c4 e4 g2 | o6 d4 f4 a2 | o6 f4 d4 o5 b2 | o6 e4 g4 b2 "
      /*B*/ "o6 b4 f4 d2 | o6 g+4 d4 o5 b2 | o6 c4 a4 e2 | o6 a2 g2 "
      /*C*/ "@11 v10 q8 me0 mp5,30,200 o6 a4 e4 c2 | o6 g+4 b4 d2 | o6 e4 c4 a2 | o6 a1 ",
    "@10 t92 v10 q6 me4 "
      /*A*/ "o2 a2 a2 | d2 d2 | g2 g2 | c2 c2 "
      /*B*/ "o2 b4 o3 d4 f4 o2 f4 | e4 g+4 b4 g+4 | a4 o3 c4 e4 o2 g+4 | "
            "a4 o3 c4 e4 o2 a+4 "
      /*C*/ "o2 f8 o3 c8 o2 e8 o3 f8 o2 e8 o3 c8 o2 a8 o3 c8 | "
            "o2 e8 b8 d8 o3 e8 o2 d8 b8 g+8 b8 | a8 o3 e8 o2 g8 o3 a8 o2 g8 o3 e8 c8 e8 | "
            "o2 a8 o3 e8 o2 g8 o3 a8 o2 g8 o3 e8 c8 e8 ",
    "@9 t92 v6 q8 me0,350 mv2,24 k-8 "
      /*A*/ "o5 c1 | f1 | o4 b1 | o5 e1 "
      /*B*/ "o5 d2 d2 | o4 g+2 g+2 | o5 c2 c2 | c2 c2 "
      /*C*/ "o4 a2 r4 a4 | g+2 r4 g+4 | o5 c2 r4 c4 | c2 r4 c4 ",
    "@9 t92 v6 q8 me0,350 mv2,24 k0 "
      /*A*/ "o5 e1 | a1 | d1 | g1 "
      /*B*/ "o5 f2 f2 | o4 b2 b2 | o5 e2 e2 | e2 e2 "
      /*C*/ "o5 c2 r4 c4 | o4 b2 r4 b4 | o5 e2 r4 e4 | e2 r4 e4 ",
    "@9 t92 v6 q8 me0,350 mv2,24 k8 "
      /*A*/ "o4 g1 | o5 c1 | f1 | o4 b1 "
      /*B*/ "o4 a2 a2 | o5 d2 d2 | g2 g2 | g2 g2 "
      /*C*/ "o5 e2 r4 e4 | d2 r4 d4 | g2 r4 g4 | g2 r4 g4 ",
    "@n t92 q3 "
      /*A*/ "[ v13 o1 a16 r4 r16 v8 o4 d+16 r16 v12 d+16 r4 r16 v8 d+16 r16 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ [ v9 o6 a16 r16 a16 r16 v12 o4 d+16 r16 v9 o6 a16 r16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ v13 o1 a16 r2 r8. v12 o4 d+16 r8. ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r4. r16 "
  } },
  // LATE NIGHT -- D minor. A a im9-iv9 vamp, B bVImaj7-bVIImaj7-im9, C iim7b5-V7-im9-iv9
  { "LATE NIGHT", {
    "@w8={" WAV_VOX "}@w9={" WAV_GLASS "}@w10={" WAV_BASSR "}@w11={" WAV_SINE "} "
      "@8 t88 v11 q7 me3 mp4,28,250 "
      /*A*/ "o6 e4 c4 a2 | o6 a4 f4 d2 | o6 f4 e4 c2 | o6 d4 a+4 g2 "
      /*B*/ "o6 d4 f4 a2 | o6 e4 g4 b2 | o6 a4 f4 d2 | o6 d2 e2 "
      /*C*/ "@11 v10 q8 me0 mp3,45,300 o6 g4 a+4 d2 | o6 c+4 e4 g2 | o6 f4 a4 o7 c2 | "
            "o6 a+4 g4 d2 ",
    "@10 t88 v11 q6 me4 "
      /*A*/ "o2 d8 r8 d8 d8 r8 d8 a8 r8 | g8 r8 g8 g8 r8 g8 o3 d8 r8 | "
            "o2 d8 r8 d8 d8 r8 d8 a8 r8 | g8 r8 g8 g8 r8 g8 o3 d8 r8 "
      /*B*/ "o2 a+2 o3 f2 | o2 c2 g2 | d2 a2 | d2 a2 "
      /*C*/ "o2 e4 g4 a+4 g+4 | a4 o3 c+4 e4 o2 d+4 | d4 f4 a4 f+4 | g4 a+4 o3 d4 o2 f4 ",
    "@9 t88 v6 q8 me0,300 mv3,25 k-10 "
      /*A*/ "r8 o5 f8 r8 f8 r8 f8 r8 f8 | r8 a+8 r8 a+8 r8 a+8 r8 a+8 | "
            "r8 f8 r8 f8 r8 f8 r8 f8 | r8 a+8 r8 a+8 r8 a+8 r8 a+8 "
      /*B*/ "o5 d2 d2 | e2 e2 | f2 f2 | f2 f2 "
      /*C*/ "o4 g2 r4 g4 | c+2 r4 c+4 | f2 r4 f4 | a+2 r4 a+4 ",
    "@9 t88 v6 q8 me0,300 mv3,25 k0 "
      /*A*/ "r8 o4 a8 r8 a8 r8 a8 r8 a8 | r8 o5 d8 r8 d8 r8 d8 r8 d8 | "
            "r8 o4 a8 r8 a8 r8 a8 r8 a8 | r8 o5 d8 r8 d8 r8 d8 r8 d8 "
      /*B*/ "o5 f2 f2 | g2 g2 | a2 a2 | a2 a2 "
      /*C*/ "o4 a+2 r4 a+4 | e2 r4 e4 | a2 r4 a4 | o5 d2 r4 d4 ",
    "@9 t88 v6 q8 me0,300 mv3,25 k10 "
      /*A*/ "r8 o5 c8 r8 c8 r8 c8 r8 c8 | r8 f8 r8 f8 r8 f8 r8 f8 | "
            "r8 c8 r8 c8 r8 c8 r8 c8 | r8 f8 r8 f8 r8 f8 r8 f8 "
      /*B*/ "o4 a2 a2 | b2 b2 | o5 c2 c2 | c2 c2 "
      /*C*/ "o5 d2 r4 d4 | g2 r4 g4 | o6 c2 r4 c4 | o5 f2 r4 f4 ",
    "@n t88 q3 "
      /*A*/ "[ [ v9 o6 a16 r16 a16 r16 v12 o4 d+16 r16 v9 o6 a16 r16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ [ v9 o6 d+16 a16 d+16 a16 ]4 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ v13 o1 a16 r4 r16 v8 o4 d+16 r16 v12 d+16 r4 r16 v8 d+16 r16 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r4. r16 "
  } },
  // CLOCKWORK -- G major. A the Pachelbel descent two chords to a bar, B vi-iii-IV-I, C I-IV-ii-V
  { "CLOCKWORK", {
    "@w8={" WAV_PULSE12 "}@w9={" WAV_TRI "}@w10={" WAV_BASSR "}@w11={" WAV_GLASS "} "
      "@8 t120 v11 q6 me6 mp3,15,200 "
      /*A*/ "o6 g4 f+4 e4 d4 | o6 b4 a4 g4 f+4 | o6 e4 g4 b4 g4 | o6 e4 c4 d4 f+4 "
      /*B*/ "o6 e8 g8 b8 g8 e4 b4 | o6 b8 f+8 d8 f+8 b4 f+4 | o6 c8 e8 g8 e8 c4 g4 | "
            "o6 d8 g8 b8 g8 d4 b4 "
      /*C*/ "@11 v11 q8 me0 mp5,35,250 o6 g2 b2 | o7 c2 o6 e2 | o6 e2 a2 | o6 f+2 d2 ",
    "@10 t120 v11 q5 me6 "
      /*A*/ "o2 g2 o3 d2 | o2 e2 b2 | c2 g2 | c2 d2 "
      /*B*/ "o2 [e8]8 | [b8]8 | [c8]8 | [g8]8 "
      /*C*/ "o2 [g4]4 | [c4]4 | [a4]4 | [d4]4 ",
    "@9 t120 v6 q8 me0 mv4,22 k-8 "
      /*A*/ "o4 b2 f+2 | g2 d2 | e2 b2 | o5 e2 f+2 "
      /*B*/ "o4 g8 r8 g8 r8 g8 r8 g8 r8 | d8 r8 d8 r8 d8 r8 d8 r8 | "
            "e8 r8 e8 r8 e8 r8 e8 r8 | b8 r8 b8 r8 b8 r8 b8 r8 "
      /*C*/ "o4 b1 | o5 e1 | c1 | o4 f+1 ",
    "@9 t120 v6 q8 me0 mv4,22 k0 "
      /*A*/ "o5 d2 o4 a2 | b2 f+2 | g2 d2 | g2 a2 "
      /*B*/ "o4 b8 r8 b8 r8 b8 r8 b8 r8 | f+8 r8 f+8 r8 f+8 r8 f+8 r8 | "
            "g8 r8 g8 r8 g8 r8 g8 r8 | d8 r8 d8 r8 d8 r8 d8 r8 "
      /*C*/ "o5 d1 | g1 | e1 | a1 ",
    "@9 t120 v6 q8 me0 mv4,22 k8 "
      /*A*/ "o4 g2 d2 | e2 b2 | o5 c2 o4 g2 | o5 c2 d2 "
      /*B*/ "o5 e8 r8 e8 r8 e8 r8 e8 r8 | o4 b8 r8 b8 r8 b8 r8 b8 r8 | "
            "o5 c8 r8 c8 r8 c8 r8 c8 r8 | o4 g8 r8 g8 r8 g8 r8 g8 r8 "
      /*C*/ "o4 g1 | o5 c1 | o4 a1 | o5 d1 ",
    "@n t120 q3 "
      /*A*/ "[ [ v9 o6 d+16 [d+16]3 ]4 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ [ v13 o1 a16 v9 o6 d+16 d+16 d+16 v12 o4 d+16 v9 o6 d+16 d+16 d+16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 ]3 | "
            "v13 o5 c16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 "
  } },
  // PUZZLE BOX -- E major. A the I-bIII-IV-bVI chromatic mediants, B a lydian I-II-iii-IV rise, C vi-bVI-V-bV
  { "PUZZLE BOX", {
    "@w8={" WAV_PULSE25 "}@w9={" WAV_BELL "}@w10={" WAV_TRI "}@w11={" WAV_HOLLOW "} "
      "@8 t126 v11 q6 me5 mp4,18,180 "
      /*A*/ "o6 e8 g+8 b8 g+8 e4 b4 | o6 g8 b8 o7 d8 o6 b8 g4 o7 d4 | "
            "o6 a8 o7 c+8 e8 c+8 o6 a4 e4 | o6 c8 e8 g8 e8 c4 g4 "
      /*B*/ "o6 e4 f+4 g+2 | o6 f+4 g+4 a+2 | o6 g+4 a+4 b2 | o6 a4 b4 o7 c+2 "
      /*C*/ "@11 v11 q8 me1 mp6,40,150 o6 c+2 g+2 | o6 c2 g2 | o6 b2 f+2 | o6 a+2 f2 ",
    "@10 t126 v11 q5 me5 "
      /*A*/ "o2 [e8]8 | [g8]8 | [a8]8 | [c8]8 "
      /*B*/ "o2 [e4]4 | [f+4]4 | [g+4]4 | [a4]4 "
      /*C*/ "o2 c+2 o3 c+2 | o2 c2 o3 c2 | o2 b2 o3 b2 | o2 a+2 o3 a+2 ",
    "@9 t126 v6 q8 me0 mv4,26 k-9 "
      /*A*/ "o4 g+8 r8 g+8 r8 g+8 r8 g+8 r8 | b8 r8 b8 r8 b8 r8 b8 r8 | "
            "o5 c+8 r8 c+8 r8 c+8 r8 c+8 r8 | e8 r8 e8 r8 e8 r8 e8 r8 "
      /*B*/ "o4 g+2 g+2 | a+2 a+2 | b2 b2 | o5 c+2 c+2 "
      /*C*/ "o5 e1 | e1 | d+1 | d1 ",
    "@9 t126 v6 q8 me0 mv4,26 k0 "
      /*A*/ "o4 b8 r8 b8 r8 b8 r8 b8 r8 | o5 d8 r8 d8 r8 d8 r8 d8 r8 | "
            "e8 r8 e8 r8 e8 r8 e8 r8 | g8 r8 g8 r8 g8 r8 g8 r8 "
      /*B*/ "o4 b2 b2 | o5 c+2 c+2 | d+2 d+2 | e2 e2 "
      /*C*/ "o4 g+1 | g1 | f+1 | f1 ",
    "@9 t126 v6 q8 me0 mv4,26 k9 "
      /*A*/ "o5 e8 r8 e8 r8 e8 r8 e8 r8 | g8 r8 g8 r8 g8 r8 g8 r8 | "
            "a8 r8 a8 r8 a8 r8 a8 r8 | o6 c8 r8 c8 r8 c8 r8 c8 r8 "
      /*B*/ "o5 e2 e2 | f+2 f+2 | g+2 g+2 | a2 a2 "
      /*C*/ "o5 c+1 | c1 | o4 b1 | a+1 ",
    "@n t126 q3 "
      /*A*/ "[ [ v9 o6 d+16 r16 d+16 r16 ]4 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ [ v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ [ v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 ]2 ]3 | "
            "v13 o5 c16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 "
  } },
  // TOY BOX -- C major. A I-vi-ii-V, B IV-V-I-vi, C I-VI7-ii-V with a secondary dominant
  { "TOY BOX", {
    "@w8={" WAV_PULSE12 "}@w9={" WAV_BELL "}@w10={" WAV_TRI "}@w11={" WAV_PULSE25 "} "
      "@8 t140 v11 q6 me6 mp5,20,150 "
      /*A*/ "o6 c8 e8 g8 e8 c4 g4 | o6 e8 a8 o7 c8 o6 a8 e4 a4 | o6 d8 f8 a8 f8 d4 a4 | "
            "o6 g8 b8 o7 d8 o6 b8 g4 d4 "
      /*B*/ "o6 f4 a4 o7 c2 | o6 g4 b4 o7 d2 | o7 c4 o6 g4 e2 | o6 a4 e4 c2 "
      /*C*/ "@11 v11 q8 me2 mp7,30,120 o6 e4 c4 g2 | o6 e4 c+4 a2 | o6 f4 d4 a2 | "
            "o6 g4 d4 o5 b2 ",
    "@10 t140 v11 q5 me6 "
      /*A*/ "o2 c8 o3 c8 o2 c8 o3 c8 o2 c8 o3 c8 o2 c8 o3 c8 | "
            "o2 a8 o3 a8 o2 a8 o3 a8 o2 a8 o3 a8 o2 a8 o3 a8 | "
            "o2 d8 o3 d8 o2 d8 o3 d8 o2 d8 o3 d8 o2 d8 o3 d8 | "
            "o2 g8 o3 g8 o2 g8 o3 g8 o2 g8 o3 g8 o2 g8 o3 g8 "
      /*B*/ "o2 [f4]4 | [g4]4 | [c4]4 | [a4]4 "
      /*C*/ "o2 c8 e8 g8 o3 c8 o2 g8 e8 c8 e8 | a8 o3 c+8 e8 a8 e8 c+8 o2 a8 o3 c+8 | "
            "o2 d8 f8 a8 o3 d8 o2 a8 f8 d8 f8 | g8 b8 o3 d8 g8 d8 o2 b8 g8 b8 ",
    "@9 t140 v6 q8 me0 mv5,25 k-8 "
      /*A*/ "o5 e8 r8 e8 r8 e8 r8 e8 r8 | c8 r8 c8 r8 c8 r8 c8 r8 | "
            "f8 r8 f8 r8 f8 r8 f8 r8 | o4 b8 r8 b8 r8 b8 r8 b8 r8 "
      /*B*/ "o4 a2 a2 | b2 b2 | o5 e2 e2 | c2 c2 "
      /*C*/ "r8 o5 e8 r8 e8 r8 e8 r8 e8 | r8 c+8 r8 c+8 r8 c+8 r8 c+8 | "
            "r8 f8 r8 f8 r8 f8 r8 f8 | r8 o4 b8 r8 b8 r8 b8 r8 b8 ",
    "@9 t140 v6 q8 me0 mv5,25 k0 "
      /*A*/ "o4 g8 r8 g8 r8 g8 r8 g8 r8 | e8 r8 e8 r8 e8 r8 e8 r8 | "
            "a8 r8 a8 r8 a8 r8 a8 r8 | o5 d8 r8 d8 r8 d8 r8 d8 r8 "
      /*B*/ "o5 c2 c2 | d2 d2 | g2 g2 | e2 e2 "
      /*C*/ "r8 o4 g8 r8 g8 r8 g8 r8 g8 | r8 e8 r8 e8 r8 e8 r8 e8 | "
            "r8 a8 r8 a8 r8 a8 r8 a8 | r8 o5 d8 r8 d8 r8 d8 r8 d8 ",
    "@9 t140 v6 q8 me0 mv5,25 k8 "
      /*A*/ "o5 c8 r8 c8 r8 c8 r8 c8 r8 | o4 a8 r8 a8 r8 a8 r8 a8 r8 | "
            "o5 d8 r8 d8 r8 d8 r8 d8 r8 | g8 r8 g8 r8 g8 r8 g8 r8 "
      /*B*/ "o5 f2 f2 | g2 g2 | o6 c2 c2 | o5 a2 a2 "
      /*C*/ "r8 o5 c8 r8 c8 r8 c8 r8 c8 | r8 o4 g8 r8 g8 r8 g8 r8 g8 | "
            "r8 d8 r8 d8 r8 d8 r8 d8 | r8 g8 r8 g8 r8 g8 r8 g8 ",
    "@n t140 q3 "
      /*A*/ "[ [ v13 o1 a16 v9 o6 d+16 d+16 d+16 v12 o4 d+16 v9 o6 d+16 d+16 d+16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ [ v13 o1 a16 r16 v9 o6 d+16 r16 ]4 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 ]3 | "
            "v13 o5 c16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 "
  } },
  // BUBBLE POP -- D major. A I-V-IV-V, B vi-ii-V-I, C an augmented I-I+-IV-iv line
  { "BUBBLE POP", {
    "@w8={" WAV_SINE "}@w9={" WAV_PULSE25 "}@w10={" WAV_TRI "}@w11={" WAV_BELL "} "
      "@8 t134 v11 q6 me5 mp5,22,150 "
      /*A*/ "o6 d8 f+8 a8 f+8 d4 a4 | o6 e8 a8 o7 c+8 o6 a8 e4 a4 | "
            "o6 g8 b8 o7 d8 o6 b8 g4 d4 | o6 a8 o7 c+8 e8 c+8 o6 a4 e4 "
      /*B*/ "o6 b4 f+4 d2 | o6 e4 b4 g2 | o6 c+4 e4 a2 | o6 d4 a4 f+2 "
      /*C*/ "@11 v11 q8 me1 mp6,35,150 o6 a2 f+2 | o6 a+2 f+2 | o6 b2 g2 | o6 a+2 g2 ",
    "@10 t134 v11 q5 me6 "
      /*A*/ "o2 d8 o3 d8 o2 d8 o3 d8 o2 d8 o3 d8 o2 d8 o3 d8 | "
            "o2 a8 o3 a8 o2 a8 o3 a8 o2 a8 o3 a8 o2 a8 o3 a8 | "
            "o2 g8 o3 g8 o2 g8 o3 g8 o2 g8 o3 g8 o2 g8 o3 g8 | "
            "o2 a8 o3 a8 o2 a8 o3 a8 o2 a8 o3 a8 o2 a8 o3 a8 "
      /*B*/ "o2 [b4]4 | [e4]4 | [a4]4 | [d4]4 "
      /*C*/ "o2 d2 o3 d2 | o2 d2 o3 d2 | o2 g2 o3 g2 | o2 g2 o3 g2 ",
    "@9 t134 v6 q8 me0 mv5,28 k-8 "
      /*A*/ "o4 f+8 r8 f+8 r8 f+8 r8 f+8 r8 | c+8 r8 c+8 r8 c+8 r8 c+8 r8 | "
            "b8 r8 b8 r8 b8 r8 b8 r8 | o5 c+8 r8 c+8 r8 c+8 r8 c+8 r8 "
      /*B*/ "o5 d2 d2 | g2 g2 | c+2 c+2 | f+2 f+2 "
      /*C*/ "o4 f+1 | f+1 | b1 | a+1 ",
    "@9 t134 v6 q8 me0 mv5,28 k0 "
      /*A*/ "o4 a8 r8 a8 r8 a8 r8 a8 r8 | e8 r8 e8 r8 e8 r8 e8 r8 | "
            "d8 r8 d8 r8 d8 r8 d8 r8 | e8 r8 e8 r8 e8 r8 e8 r8 "
      /*B*/ "o4 f+2 f+2 | b2 b2 | o5 e2 e2 | a2 a2 "
      /*C*/ "o4 a1 | a+1 | o5 d1 | d1 ",
    "@9 t134 v6 q8 me0 mv5,28 k8 "
      /*A*/ "o5 d8 r8 d8 r8 d8 r8 d8 r8 | o4 a8 r8 a8 r8 a8 r8 a8 r8 | "
            "g8 r8 g8 r8 g8 r8 g8 r8 | a8 r8 a8 r8 a8 r8 a8 r8 "
      /*B*/ "o4 b2 b2 | o5 e2 e2 | a2 a2 | d2 d2 "
      /*C*/ "o5 d1 | d1 | g1 | g1 ",
    "@n t134 q3 "
      /*A*/ "[ [ v13 o1 a16 r16 v9 o6 d+16 r16 ]4 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ [ v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ [ v13 o1 a16 r16 v10 o5 g16 r16 v12 o4 d+16 r16 v10 o5 g16 r16 ]2 ]3 | "
            "v13 c16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 "
  } },
  // CANDY LANE -- F major. A I-IV-I-V, B ii-V-iii-vi, C I-III7-vi-IV
  { "CANDY LANE", {
    "@w8={" WAV_BELL "}@w9={" WAV_SINE "}@w10={" WAV_TRI "}@w11={" WAV_PULSE25 "} "
      "@8 t128 v11 q6 me4 mp4,25,200 "
      /*A*/ "o6 f8 a8 o7 c8 o6 a8 f4 c4 | o6 a+8 o7 d8 f8 d8 o6 a+4 f4 | "
            "o6 c8 f8 a8 f8 c4 a4 | o6 g8 o7 c8 e8 c8 o6 g4 e4 "
      /*B*/ "o6 g4 a+4 d2 | o6 g4 e4 c2 | o6 a4 e4 c2 | o6 d4 f4 a2 "
      /*C*/ "@11 v11 q8 me2 mp6,30,150 o6 c2 a2 | o6 c+2 e2 | o6 d2 f2 | o6 d2 a+2 ",
    "@10 t128 v11 q6 me5 "
      /*A*/ "o2 f8 o3 f8 o2 f8 o3 f8 o2 f8 o3 f8 o2 f8 o3 f8 | "
            "o2 a+8 o3 a+8 o2 a+8 o3 a+8 o2 a+8 o3 a+8 o2 a+8 o3 a+8 | "
            "o2 f8 o3 f8 o2 f8 o3 f8 o2 f8 o3 f8 o2 f8 o3 f8 | "
            "o2 c8 o3 c8 o2 c8 o3 c8 o2 c8 o3 c8 o2 c8 o3 c8 "
      /*B*/ "o2 [g4]4 | [c4]4 | [a4]4 | [d4]4 "
      /*C*/ "o2 f8 o3 c8 f8 c8 o2 f8 o3 c8 f8 c8 | o2 a8 o3 e8 a8 e8 o2 a8 o3 e8 a8 e8 | "
            "o2 d8 a8 o3 d8 o2 a8 d8 a8 o3 d8 o2 a8 | "
            "a+8 o3 f8 a+8 f8 o2 a+8 o3 f8 a+8 f8 ",
    "@9 t128 v6 q8 me0 mv4,24 k-8 "
      /*A*/ "o4 a8 r8 a8 r8 a8 r8 a8 r8 | o5 d8 r8 d8 r8 d8 r8 d8 r8 | "
            "o4 a8 r8 a8 r8 a8 r8 a8 r8 | e8 r8 e8 r8 e8 r8 e8 r8 "
      /*B*/ "o4 a+2 a+2 | e2 e2 | c2 c2 | f2 f2 "
      /*C*/ "o4 a1 | o5 c+1 | f1 | d1 ",
    "@9 t128 v6 q8 me0 mv4,24 k0 "
      /*A*/ "o5 c8 r8 c8 r8 c8 r8 c8 r8 | f8 r8 f8 r8 f8 r8 f8 r8 | "
            "c8 r8 c8 r8 c8 r8 c8 r8 | o4 g8 r8 g8 r8 g8 r8 g8 r8 "
      /*B*/ "o5 d2 d2 | g2 g2 | e2 e2 | a2 a2 "
      /*C*/ "o5 c1 | e1 | a1 | f1 ",
    "@9 t128 v6 q8 me0 mv4,24 k8 "
      /*A*/ "o5 f8 r8 f8 r8 f8 r8 f8 r8 | a+8 r8 a+8 r8 a+8 r8 a+8 r8 | "
            "f8 r8 f8 r8 f8 r8 f8 r8 | c8 r8 c8 r8 c8 r8 c8 r8 "
      /*B*/ "o4 g2 g2 | o5 c2 c2 | o4 a2 a2 | o5 d2 d2 "
      /*C*/ "o5 f1 | g1 | d1 | o4 a+1 ",
    "@n t128 q3 "
      /*A*/ "[ [ v13 o1 a16 r16 v9 o6 d+16 r16 ]4 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ [ v9 o6 d+16 a16 d+16 a16 ]4 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ [ v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 ]2 ]3 | "
            "v13 o5 c16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 "
  } },
  // MARCH -- Bb major. A I-V-I-IV, B IV-I-ii-V, C I-V7/ii-ii-V
  { "MARCH", {
    "@w8={" WAV_REED "}@w9={" WAV_FIFTH "}@w10={" WAV_SOFTSAW "}@w11={" WAV_PIANO "} "
      "@8 t112 v11 q6 me3 mp5,20,250 "
      /*A*/ "o6 a+4 d4 f4 a+4 | o6 c4 a4 f4 c4 | o6 d4 f4 a+4 f4 | o6 d+4 g4 a+4 g4 "
      /*B*/ "o6 g4 a+4 o7 d+2 | o6 f4 d4 a+2 | o6 g4 d+4 c2 | o6 a4 c4 f2 "
      /*C*/ "@11 v11 q8 me1 mp6,35,150 o6 a+2 f2 | o6 b2 g2 | o6 d+2 g2 | o6 c2 a2 ",
    "@10 t112 v11 q6 me4 "
      /*A*/ "o2 a+2 a+2 | f2 f2 | a+2 a+2 | d+2 d+2 "
      /*B*/ "o2 [d+4]4 | [a+4]4 | [c4]4 | [f4]4 "
      /*C*/ "o2 a+4 a+4 o3 f4 o2 a+4 | g4 g4 o3 d4 o2 g4 | c4 c4 g4 c4 | "
            "f4 f4 o3 c4 o2 f4 ",
    "@9 t112 v6 q8 me0 mv3,20 k-7 "
      /*A*/ "o5 d2 d2 | o4 a2 a2 | o5 d2 d2 | g2 g2 "
      /*B*/ "o4 [g4]4 | [d4]4 | [d+4]4 | [a4]4 "
      /*C*/ "o5 d1 | o4 b1 | o5 d+1 | o4 a1 ",
    "@9 t112 v6 q8 me0 mv3,20 k0 "
      /*A*/ "o5 f2 f2 | c2 c2 | f2 f2 | a+2 a+2 "
      /*B*/ "o4 [a+4]4 | [f4]4 | [g4]4 | o5 [c4]4 "
      /*C*/ "o5 f1 | d1 | g1 | o6 c1 ",
    "@9 t112 v6 q8 me0 mv3,20 k7 "
      /*A*/ "o4 a+2 a+2 | f2 f2 | a+2 a+2 | o5 d+2 d+2 "
      /*B*/ "o5 [d+4]4 | o4 [a+4]4 | o5 [c4]4 | [f4]4 "
      /*C*/ "o4 a+1 | f1 | c1 | f1 ",
    "@n t112 q3 "
      /*A*/ "[ v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ [ v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 ]4 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ [ v13 o1 a16 v8 o4 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 v12 o4 d+16 v8 d+16 v12 d+16 v8 d+16 v12 d+16 v8 d+16 v12 d+16 v8 d+16 "
  } },
  // CATACOMB -- C# minor. A the phrygian i-bII-bVII-i, B iv-bVI-i-V, C i-bIII-bVI-bII
  { "CATACOMB", {
    "@w8={" WAV_HOLLOW "}@w9={" WAV_METAL "}@w10={" WAV_BASSR "}@w11={" WAV_GLASS "} "
      "@8 t84 v11 q8 me0 mp3,40,400 "
      /*A*/ "o5 c+1 | o5 d2 f+2 | o5 b2 f+2 | o6 c+2 g+2 "
      /*B*/ "o6 f+2 c+2 | o6 e2 a2 | o6 g+2 e2 | o6 d+2 c2 "
      /*C*/ "@11 v10 q6 me4 mp5,25,200 o6 c+4 e4 g+2 | o6 e4 g+4 b2 | o6 a4 e4 c+2 | "
            "o6 f+4 d4 a2 ",
    "@10 t84 v11 q7 me1 "
      /*A*/ "o2 c+1 | d1 | b1 | c+1 "
      /*B*/ "o2 f+2 f+2 | a2 a2 | c+2 c+2 | g+2 g+2 "
      /*C*/ "o2 [c+4]4 | [e4]4 | [a4]4 | [d4]4 ",
    "@9 t84 v5 q8 me0,500 mv2,32 k-12 "
      /*A*/ "o5 e1 | f+1 | d+1 | e1 "
      /*B*/ "o4 a1 | o5 c+1 | e1 | c1 "
      /*C*/ "o5 e2 r4 e4 | g+2 r4 g+4 | c+2 r4 c+4 | f+2 r4 f+4 ",
    "@9 t84 v5 q8 me0,500 mv2,32 k0 "
      /*A*/ "o4 g+1 | a1 | f+1 | g+1 "
      /*B*/ "o5 c+1 | e1 | g+1 | d+1 "
      /*C*/ "o4 g+2 r4 g+4 | b2 r4 b4 | o5 e2 r4 e4 | a2 r4 a4 ",
    "@9 t84 v5 q8 me0,500 mv2,32 k12 "
      /*A*/ "o5 c+1 | d1 | o4 b1 | o5 c+1 "
      /*B*/ "o4 f+1 | a1 | o5 c+1 | o4 g+1 "
      /*C*/ "o5 c+2 r4 c+4 | e2 r4 e4 | a2 r4 a4 | d2 r4 d4 ",
    "@n t84 q3 "
      /*A*/ "[ v13 o1 a16 r8 a16 r2 a16 r8 a16 ]4 "
      /*B*/ "[ v13 o1 a16 r2 r8. v12 o4 d+16 r8. ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ v13 o1 a16 r4. r16 v12 o4 d+16 r4. r16 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r4. r16 "
  } },
  // OMEN -- G minor. A the harmonic i-bVI-V-i, B a bII-i tritone rock, C i-iv-bII-V
  { "OMEN", {
    "@w8={" WAV_METAL "}@w9={" WAV_HOLLOW "}@w10={" WAV_SOFTSAW "}@w11={" WAV_FIFTH "} "
      "@8 t80 v11 q8 me0 mp2,35,450 "
      /*A*/ "o5 g2 a+2 | o6 d+2 g2 | o6 d2 f+2 | o6 g2 d2 "
      /*B*/ "o6 g+2 d+2 | o6 g2 d2 | o6 c2 g+2 | o6 a+2 g2 "
      /*C*/ "@11 v11 q6 me3 mp6,20,150 o6 d4 g4 a+2 | o6 d+4 c4 g2 | o6 g+4 d+4 c2 | "
            "o6 a4 f+4 d2 ",
    "@10 t80 v11 q7 me1 "
      /*A*/ "o2 g1 | d+1 | d1 | g1 "
      /*B*/ "o2 g+2 g+2 | g2 g2 | g+2 g+2 | g2 g2 "
      /*C*/ "o2 [g4]4 | [c4]4 | [g+4]4 | [d4]4 ",
    "@9 t80 v5 q8 me0,600 mv2,30 k-11 "
      /*A*/ "o4 a+1 | g1 | f+1 | a+1 "
      /*B*/ "o5 c1 | o4 a+1 | o5 c1 | o4 a+1 "
      /*C*/ "o4 a+2 r4 a+4 | o5 d+2 r4 d+4 | c2 r4 c4 | o4 f+2 r4 f+4 ",
    "@9 t80 v5 q8 me0,600 mv2,30 k0 "
      /*A*/ "o5 d1 | o4 a+1 | a1 | o5 d1 "
      /*B*/ "o5 d+1 | d1 | d+1 | d1 "
      /*C*/ "o5 d2 r4 d4 | g2 r4 g4 | d+2 r4 d+4 | o4 a2 r4 a4 ",
    "@9 t80 v5 q8 me0,600 mv2,30 k11 "
      /*A*/ "o4 g1 | d+1 | d1 | g1 "
      /*B*/ "o4 g+1 | g1 | g+1 | g1 "
      /*C*/ "o4 g2 r4 g4 | o5 c2 r4 c4 | o4 g+2 r4 g+4 | d2 r4 d4 ",
    "@n t80 q3 "
      /*A*/ "[ r1 ]4 "
      /*B*/ "[ v13 o1 a16 r8 a16 r2 a16 r8 a16 ]4 "
      /*C*/ "[ v13 o1 a16 r4. r16 v12 o4 d+16 r4. r16 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r4. r16 "
  } },
  // WHISPER -- A minor. A a modal i-bVII drone, B iv-i-bVII-bVI, C i-V-i-bVI
  { "WHISPER", {
    "@w8={" WAV_SINE "}@w9={" WAV_GLASS "}@w10={" WAV_TRI "}@w11={" WAV_VOX "} "
      "@8 t70 v10 q8 me0 mp2,20,600 "
      /*A*/ "o6 a1 | o6 g1 | o6 e2 c2 | o6 d2 o5 b2 "
      /*B*/ "o6 d2 f2 | o6 e2 a2 | o6 d2 g2 | o6 c2 f2 "
      /*C*/ "@11 v10 q6 me3 mp5,30,250 o6 a2 e2 | o6 g+2 b2 | o6 c2 a2 | o6 f2 a2 ",
    "@10 t70 v9 q8 me0 "
      /*A*/ "o2 a1 | g1 | a1 | g1 "
      /*B*/ "o2 d1 | a1 | g1 | f1 "
      /*C*/ "o2 a2 r4 a4 | e2 r4 e4 | a2 r4 a4 | f2 r4 f4 ",
    "@9 t70 v5 q8 me0,700 mv2,26 k-12 "
      /*A*/ "o5 c1 | o4 b1 | o5 c1 | o4 b1 "
      /*B*/ "o5 f1 | c1 | o4 b1 | a1 "
      /*C*/ "o5 c2 r4 c4 | o4 g+2 r4 g+4 | o5 c2 r4 c4 | o4 a2 r4 a4 ",
    "@9 t70 v5 q8 me0,700 mv2,26 k0 "
      /*A*/ "o5 e1 | d1 | e1 | d1 "
      /*B*/ "o4 a1 | e1 | d1 | c1 "
      /*C*/ "o5 e2 r4 e4 | o4 b2 r4 b4 | o5 e2 r4 e4 | c2 r4 c4 ",
    "@9 t70 v5 q8 me0,700 mv2,26 k12 "
      /*A*/ "o4 a1 | g1 | a1 | g1 "
      /*B*/ "o5 d1 | o4 a1 | g1 | f1 "
      /*C*/ "o4 a2 r4 a4 | e2 r4 e4 | a2 r4 a4 | f2 r4 f4 ",
    "@n t70 q3 "
      /*A*/ "[ r1 ]4 "
      /*B*/ "[ r1 ]4 "
      /*C*/ "[ v13 o1 a16 r2 r8. v12 o4 d+16 r8. ]4 "
  } },
  // FOG -- E minor. A a suspended isus4-i-IVsus2-iv, B bVI-bVII-i, C iv-bVII-bIII-i
  { "FOG", {
    "@w8={" WAV_GLASS "}@w9={" WAV_SINE "}@w10={" WAV_TRI "}@w11={" WAV_HOLLOW "} "
      "@8 t68 v10 q8 me0 mp2,25,600 "
      /*A*/ "o6 b2 a2 | o6 g2 e2 | o6 b2 e2 | o6 c2 a2 "
      /*B*/ "o6 g2 e2 | o6 a2 f+2 | o6 b2 g2 | o6 e1 "
      /*C*/ "@11 v10 q6 me4 mp4,35,300 o6 c2 a2 | o6 d2 a2 | o6 b2 g2 | o6 e2 b2 ",
    "@10 t68 v9 q8 me0 "
      /*A*/ "o2 e1 | e1 | a1 | a1 "
      /*B*/ "o2 c1 | d1 | e1 | e1 "
      /*C*/ "o2 a2 r4 a4 | d2 r4 d4 | g2 r4 g4 | e2 r4 e4 ",
    "@9 t68 v5 q8 me0,700 mv2,30 k-12 "
      /*A*/ "o4 a1 | g1 | b1 | o5 c1 "
      /*B*/ "o5 e1 | f+1 | g1 | g1 "
      /*C*/ "o5 c2 r4 c4 | o4 f+2 r4 f+4 | b2 r4 b4 | g2 r4 g4 ",
    "@9 t68 v5 q8 me0,700 mv2,30 k0 "
      /*A*/ "o4 b1 | b1 | o5 e1 | e1 "
      /*B*/ "o4 g1 | a1 | b1 | b1 "
      /*C*/ "o5 e2 r4 e4 | a2 r4 a4 | d2 r4 d4 | o4 b2 r4 b4 ",
    "@9 t68 v5 q8 me0,700 mv2,30 k12 "
      /*A*/ "o5 e1 | e1 | a1 | a1 "
      /*B*/ "o5 c1 | d1 | e1 | e1 "
      /*C*/ "o4 a2 r4 a4 | o5 d2 r4 d4 | g2 r4 g4 | e2 r4 e4 ",
    "@n t68 q3 "
      /*A*/ "[ r1 ]4 "
      /*B*/ "[ [ v9 o6 d+16 r16 d+16 r16 ]4 ]4 "
      /*C*/ "[ v13 o1 a16 r8 a16 r2 a16 r8 a16 ]3 | "
            "a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r4. r16 "
  } },
  // RITUAL -- D dorian. A i-iv-bVII-i, B bIII-bVII-iv-i, C a i-bII see-saw
  { "RITUAL", {
    "@w8={" WAV_REED "}@w9={" WAV_VOX "}@w10={" WAV_BASSR "}@w11={" WAV_METAL "} "
      "@8 t96 v11 q6 me4 mp4,25,200 "
      /*A*/ "o5 d8 f8 a8 f8 d4 a4 | o5 g8 a+8 o6 d8 o5 a+8 g4 o6 d4 | "
            "o6 c8 e8 g8 e8 c4 g4 | o6 d8 a8 f8 a8 d4 f4 "
      /*B*/ "o6 f4 a4 o7 c2 | o6 g4 e4 c2 | o6 a+4 g4 d2 | o6 a4 f4 d2 "
      /*C*/ "@11 v11 q8 me0 mp6,45,150 o6 d2 f2 | o6 d+2 g2 | o6 a2 f2 | o6 a+2 g2 ",
    "@10 t96 v11 q6 me4 "
      /*A*/ "o2 [d8]8 | [g8]8 | [c8]8 | [d8]8 "
      /*B*/ "o2 [f4]4 | [c4]4 | [g4]4 | [d4]4 "
      /*C*/ "o2 d2 o3 d2 | o2 d+2 o3 d+2 | o2 d2 o3 d2 | o2 d+2 o3 d+2 ",
    "@9 t96 v6 q8 me0,300 mv3,30 k-10 "
      /*A*/ "o5 f8 r8 f8 r8 f8 r8 f8 r8 | a+8 r8 a+8 r8 a+8 r8 a+8 r8 | "
            "e8 r8 e8 r8 e8 r8 e8 r8 | f8 r8 f8 r8 f8 r8 f8 r8 "
      /*B*/ "o4 a2 a2 | e2 e2 | a+2 a+2 | f2 f2 "
      /*C*/ "o5 f1 | g1 | f1 | g1 ",
    "@9 t96 v6 q8 me0,300 mv3,30 k0 "
      /*A*/ "o4 a8 r8 a8 r8 a8 r8 a8 r8 | o5 d8 r8 d8 r8 d8 r8 d8 r8 | "
            "g8 r8 g8 r8 g8 r8 g8 r8 | a8 r8 a8 r8 a8 r8 a8 r8 "
      /*B*/ "o5 c2 c2 | o4 g2 g2 | d2 d2 | a2 a2 "
      /*C*/ "o4 a1 | a+1 | a1 | a+1 ",
    "@9 t96 v6 q8 me0,300 mv3,30 k10 "
      /*A*/ "o5 d8 r8 d8 r8 d8 r8 d8 r8 | g8 r8 g8 r8 g8 r8 g8 r8 | "
            "o6 c8 r8 c8 r8 c8 r8 c8 r8 | o5 d8 r8 d8 r8 d8 r8 d8 r8 "
      /*B*/ "o5 f2 f2 | c2 c2 | o4 g2 g2 | d2 d2 "
      /*C*/ "o5 d1 | d+1 | d1 | d+1 ",
    "@n t96 q3 "
      /*A*/ "[ [ v11 o3 a16 r16 o2 d+16 r16 ]4 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 v12 o4 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ v13 o1 a16 r4. r16 v12 o4 d+16 r4. r16 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 v12 o4 d+16 v8 d+16 v12 d+16 v8 d+16 v12 d+16 v8 d+16 v12 d+16 v8 d+16 "
  } },
  // VOID -- F minor. A i-bVI-iv-bII, B i-v-bVI-bIII, C a chromatic i-i+-i6-i7 line
  { "VOID", {
    "@w8={" WAV_SYNC "}@w9={" WAV_GLASS "}@w10={" WAV_SOFTSAW "}@w11={" WAV_HOLLOW "} "
      "@8 t76 v10 q8 me0 mp2,30,500 "
      /*A*/ "o6 f2 c2 | o6 f2 g+2 | o6 a+2 f2 | o6 a+2 f+2 "
      /*B*/ "o6 c2 f2 | o6 d+2 g2 | o6 g+2 f2 | o6 c2 d+2 "
      /*C*/ "@11 v10 q6 me3 mp5,40,250 o6 f2 g+2 | o6 a2 c+2 | o6 d2 c2 | o6 d+2 c2 ",
    "@10 t76 v10 q8 me0 "
      /*A*/ "o2 f1 | c+1 | a+1 | f+1 "
      /*B*/ "o2 f2 f2 | c2 c2 | c+2 c+2 | g+2 g+2 "
      /*C*/ "o2 f2 r4 f4 | f2 r4 f4 | f2 r4 f4 | f2 r4 f4 ",
    "@9 t76 v5 q8 me0,650 mv2,28 k-12 "
      /*A*/ "o4 g+1 | f1 | c+1 | a+1 "
      /*B*/ "o4 g+1 | d+1 | f1 | c1 "
      /*C*/ "o4 g+2 r4 g+4 | a2 r4 a4 | g+2 r4 g+4 | g+2 r4 g+4 ",
    "@9 t76 v5 q8 me0,650 mv2,28 k0 "
      /*A*/ "o5 c1 | o4 g+1 | f1 | c+1 "
      /*B*/ "o5 c1 | o4 g1 | g+1 | d+1 "
      /*C*/ "o5 c2 r4 c4 | c+2 r4 c+4 | c2 r4 c4 | c2 r4 c4 ",
    "@9 t76 v5 q8 me0,650 mv2,28 k12 "
      /*A*/ "o5 f1 | c+1 | o4 a+1 | f+1 "
      /*B*/ "o5 f1 | c1 | c+1 | o4 g+1 "
      /*C*/ "o5 f2 r4 f4 | f2 r4 f4 | d2 r4 d4 | d+2 r4 d+4 ",
    "@n t76 q3 "
      /*A*/ "[ r1 ]4 "
      /*B*/ "[ v13 o1 a16 r8 a16 r2 a16 r8 a16 ]4 "
      /*C*/ "[ v13 o1 a16 r2 r8. v12 o4 d+16 r8. ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r4. r16 "
  } },
  // STARFIELD -- Bb major. A I-IVmaj7-I-V, B iii-vi-ii-V, C the backdoor bVII-IV-I
  { "STARFIELD", {
    "@w8={" WAV_BELL "}@w9={" WAV_SINE "}@w10={" WAV_TRI "}@w11={" WAV_GLASS "} "
      "@8 t100 v11 q7 me3 mp4,25,300 "
      /*A*/ "o6 d2 f2 | o6 d2 g2 | o6 f2 a+2 | o6 c2 a2 "
      /*B*/ "o6 d4 f4 a2 | o6 g4 a+4 d2 | o6 d+4 g4 c2 | o6 a4 f4 c2 "
      /*C*/ "@11 v11 q8 me0 mp3,45,300 o6 c2 d+2 | o6 g2 a+2 | o6 d2 f2 | o6 a+1 ",
    "@10 t100 v10 q7 me3 "
      /*A*/ "o2 a+2 a+2 | d+2 d+2 | a+2 a+2 | f2 f2 "
      /*B*/ "o2 d8 a8 o3 d8 o2 a8 d8 a8 o3 d8 o2 a8 | g8 o3 d8 g8 d8 o2 g8 o3 d8 g8 d8 | "
            "o2 c8 g8 o3 c8 o2 g8 c8 g8 o3 c8 o2 g8 | f8 o3 c8 f8 c8 o2 f8 o3 c8 f8 c8 "
      /*C*/ "o2 g+4 g+4 o3 d+4 o2 g+4 | d+4 d+4 a+4 d+4 | a+4 a+4 o3 f4 o2 a+4 | "
            "a+4 a+4 o3 f4 o2 a+4 ",
    "@9 t100 v6 q8 me0,400 mv3,24 k-10 "
      /*A*/ "o5 d1 | g1 | d1 | o4 a1 "
      /*B*/ "o5 f2 f2 | a+2 a+2 | d+2 d+2 | o4 a2 a2 "
      /*C*/ "o5 c4. c4. c4 | o4 g4. g4. g4 | d4. d4. d4 | d4. d4. d4 ",
    "@9 t100 v6 q8 me0,400 mv3,24 k0 "
      /*A*/ "o5 f1 | a+1 | f1 | c1 "
      /*B*/ "o4 a2 a2 | o5 d2 d2 | g2 g2 | o6 c2 c2 "
      /*C*/ "o5 d+4. d+4. d+4 | o4 a+4. a+4. a+4 | f4. f4. f4 | f4. f4. f4 ",
    "@9 t100 v6 q8 me0,400 mv3,24 k10 "
      /*A*/ "o4 a+1 | o5 d1 | o4 a+1 | f1 "
      /*B*/ "o5 d2 d2 | g2 g2 | o6 c2 c2 | o5 f2 f2 "
      /*C*/ "o4 g+4. g+4. g+4 | d+4. d+4. d+4 | a+4. a+4. a+4 | a+4. a+4. a+4 ",
    "@n t100 q3 "
      /*A*/ "[ [ v9 o6 d+16 r16 d+16 r16 ]4 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ [ v9 o6 d+16 a16 d+16 a16 ]4 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ [ v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 ]2 ]3 | "
            "v13 o5 c16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 "
  } },
  // HYPERJUMP -- E minor. A i-bIII-bVII-IV, B i-bVI-bVII-bIII, C i-V-bVI-bVII
  { "HYPERJUMP", {
    "@w8={" WAV_PULSE12 "}@w9={" WAV_SYNC "}@w10={" WAV_BASSR "}@w11={" WAV_SOFTSAW "} "
      "@8 t176 v11 q6 me5 mp6,20,100 "
      /*A*/ "o6 e8 g8 b8 o7 e8 o6 b4 g4 | o6 g8 b8 o7 d8 g8 o6 d4 b4 | "
            "o6 d8 f+8 a8 o7 d8 o6 a4 f+4 | o6 a8 o7 c+8 e8 a8 o6 e4 c+4 "
      /*B*/ "o6 b4 e4 g2 | o7 c4 o6 g4 e2 | o6 d4 a4 f+2 | o6 g4 o7 d4 o6 b2 "
      /*C*/ "@11 v11 q8 me1 mp8,40,80 o6 e2 g2 | o6 f+2 d+2 | o6 g2 e2 | o6 a2 f+2 ",
    "@10 t176 v11 q5 me6 "
      /*A*/ "o2 [e16]16 | [g16]16 | [d16]16 | [a16]16 "
      /*B*/ "o2 e8 o3 e8 o2 e8 o3 e8 o2 e8 o3 e8 o2 e8 o3 e8 | "
            "o2 c8 o3 c8 o2 c8 o3 c8 o2 c8 o3 c8 o2 c8 o3 c8 | "
            "o2 d8 o3 d8 o2 d8 o3 d8 o2 d8 o3 d8 o2 d8 o3 d8 | "
            "o2 g8 o3 g8 o2 g8 o3 g8 o2 g8 o3 g8 o2 g8 o3 g8 "
      /*C*/ "o2 e8 e16 e16 e8 e16 e16 e8 e16 e16 b8 e16 e16 | "
            "b8 b16 b16 b8 b16 b16 b8 b16 b16 o3 f+8 o2 b16 b16 | "
            "c8 c16 c16 c8 c16 c16 c8 c16 c16 g8 c16 c16 | "
            "d8 d16 d16 d8 d16 d16 d8 d16 d16 a8 d16 d16 ",
    "@9 t176 v6 q8 me0 mv6,30 k-9 "
      /*A*/ "o4 g8 r8 g8 r8 g8 r8 g8 r8 | b8 r8 b8 r8 b8 r8 b8 r8 | "
            "f+8 r8 f+8 r8 f+8 r8 f+8 r8 | c+8 r8 c+8 r8 c+8 r8 c+8 r8 "
      /*B*/ "r8 o4 g8 r8 g8 r8 g8 r8 g8 | r8 e8 r8 e8 r8 e8 r8 e8 | "
            "r8 f+8 r8 f+8 r8 f+8 r8 f+8 | r8 b8 r8 b8 r8 b8 r8 b8 "
      /*C*/ "o4 g1 | d+1 | e1 | f+1 ",
    "@9 t176 v6 q8 me0 mv6,30 k0 "
      /*A*/ "o4 b8 r8 b8 r8 b8 r8 b8 r8 | o5 d8 r8 d8 r8 d8 r8 d8 r8 | "
            "o4 a8 r8 a8 r8 a8 r8 a8 r8 | e8 r8 e8 r8 e8 r8 e8 r8 "
      /*B*/ "r8 o4 b8 r8 b8 r8 b8 r8 b8 | r8 g8 r8 g8 r8 g8 r8 g8 | "
            "r8 a8 r8 a8 r8 a8 r8 a8 | r8 o5 d8 r8 d8 r8 d8 r8 d8 "
      /*C*/ "o4 b1 | f+1 | g1 | a1 ",
    "@9 t176 v6 q8 me0 mv6,30 k9 "
      /*A*/ "o5 e8 r8 e8 r8 e8 r8 e8 r8 | g8 r8 g8 r8 g8 r8 g8 r8 | "
            "d8 r8 d8 r8 d8 r8 d8 r8 | o4 a8 r8 a8 r8 a8 r8 a8 r8 "
      /*B*/ "r8 o5 e8 r8 e8 r8 e8 r8 e8 | r8 c8 r8 c8 r8 c8 r8 c8 | "
            "r8 d8 r8 d8 r8 d8 r8 d8 | r8 g8 r8 g8 r8 g8 r8 g8 "
      /*C*/ "o5 e1 | o4 b1 | o5 c1 | d1 ",
    "@n t176 q3 "
      /*A*/ "[ [ v13 o1 a16 v9 o6 d+16 d+16 d+16 v12 o4 d+16 v9 o6 d+16 d+16 d+16 ]2 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 v13 o1 a16 v8 o4 d+16 v12 d+16 v8 d+16 "
      /*B*/ "[ v13 o1 a16 v9 o6 d+16 d+16 d+16 v12 o4 d+16 v9 o6 d+16 d+16 d+16 v13 o1 a16 v9 o6 d+16 d+16 v13 o1 a16 v12 o4 d+16 v9 o6 d+16 d+16 d+16 ]3 | "
            "v13 o1 a16 r16 v9 o6 d+16 v13 o1 a16 v12 o4 d+16 r16 v9 o6 d+16 r16 v11 o3 a16 o2 d+16 o3 a16 o2 d+16 v12 o4 d+16 v8 d+16 v12 d+16 v13 o5 c16 "
      /*C*/ "[ [ v13 o1 a16 v9 o6 d+16 v13 o1 a16 v9 o6 d+16 v12 o4 d+16 r16 v9 o6 d+16 r16 ]2 ]3 | "
            "v13 o5 c16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 v13 o1 a16 r16 v9 o6 d+16 r16 v12 o4 d+16 r16 v9 o6 d+16 r16 "
  } },
  // VIBRATO -- mp: the same held notes flat, then wavering, then trilling
  { "VIBRATO", {
    "@w8={" WAV_TRI "} "
      "@8 t110 v11 q8 me1 mp0 "
      /*A*/ "mp0 o5 e1 | o5 a1 "
      /*B*/ "mp5,35,150 o5 f1 | o5 g1 "
      /*C*/ "mp9,90,0,2 o6 c1 | o5 b1 ",
    "@8 t110 v10 q8 me0 "
      /*A*/ "o2 c1 | a1 "
      /*B*/ "o2 f1 | g1 "
      /*C*/ "o2 c1 | g1 ",
    "@8 t110 v6 q8 me0 k-7 "
      /*A*/ "o5 e1 | c1 "
      /*B*/ "o4 a1 | b1 "
      /*C*/ "o5 e1 | o4 b1 ",
    "@8 t110 v6 q8 me0 k0 "
      /*A*/ "o4 g1 | e1 "
      /*B*/ "o5 c1 | d1 "
      /*C*/ "o4 g1 | d1 ",
    "@8 t110 v6 q8 me0 k7 "
      /*A*/ "o5 c1 | o4 a1 "
      /*B*/ "o5 f1 | g1 "
      /*C*/ "o5 c1 | o4 g1 ",
    0
  } },
  // PLUCK -- me: one arpeggio held flat, then decaying, then plucked hard
  { "PLUCK", {
    "@w8={" WAV_PULSE25 "} "
      "@8 t150 v12 q8 me0 "
      /*A*/ "me0 o5 [ c16 e16 g16 o6 c16 o5 g16 e16 c16 e16 ]2 | "
            "o5 [ f16 a16 o6 c16 f16 c16 o5 a16 f16 a16 ]2 "
      /*B*/ "me6 o5 [ a16 o6 c16 e16 a16 e16 o5 c16 a16 o6 c16 ]2 | "
            "o5 [ g16 b16 o6 d16 g16 d16 o5 b16 g16 b16 ]2 "
      /*C*/ "me13 o5 [ c16 e16 g16 o6 c16 o5 g16 e16 c16 e16 ]2 | "
            "o5 [ g16 b16 o6 d16 g16 d16 o5 b16 g16 b16 ]2 ",
    "@8 t150 v11 q6 me2 "
      /*A*/ "o2 [c4]4 | [f4]4 "
      /*B*/ "o2 [a4]4 | [g4]4 "
      /*C*/ "o2 [c4]4 | [g4]4 ",
    "@8 t150 v6 q8 me0 k-8 "
      /*A*/ "o5 e1 | a1 "
      /*B*/ "o5 c1 | o4 b1 "
      /*C*/ "o5 e1 | o4 b1 ",
    "@8 t150 v6 q8 me0 k0 "
      /*A*/ "o4 g1 | o5 c1 "
      /*B*/ "o5 e1 | d1 "
      /*C*/ "o4 g1 | d1 ",
    "@8 t150 v6 q8 me0 k8 "
      /*A*/ "o5 c1 | f1 "
      /*B*/ "o4 a1 | g1 "
      /*C*/ "o5 c1 | o4 g1 ",
    "@n t150 q3 "
      /*A*/ "[ [ v9 o6 d+16 r16 d+16 r16 ]4 ]2 "
      /*B*/ "[ [ v9 o6 d+16 r16 d+16 r16 ]4 ]2 "
      /*C*/ "[ [ v9 o6 d+16 r16 d+16 r16 ]4 ]2 "
  } },
  // SLIDE -- mg and ms: leaps struck cleanly, then slid into, then swept away
  { "SLIDE", {
    "@w8={" WAV_SOFTSAW "} "
      "@8 t120 v11 q8 me0 mg0 ms0 "
      /*A*/ "mg0 ms0 o5 c4 o6 g4 o5 e4 o6 c4 | o5 g4 o6 d4 o5 b4 o6 g4 "
      /*B*/ "mg150 ms0 o5 c4 o6 g4 o5 e4 o6 c4 | o5 g4 o6 d4 o5 b4 o6 g4 "
      /*C*/ "mg0 ms-900 o5 c4 o6 g4 o5 e4 o6 c4 | o5 g4 o6 d4 o5 b4 o6 g4 ",
    "@8 t120 v10 q7 me3 "
      /*A*/ "o2 c2 c2 | g2 g2 "
      /*B*/ "o2 c2 c2 | g2 g2 "
      /*C*/ "o2 c2 c2 | g2 g2 ",
    "@8 t120 v6 q8 me0 k-8 "
      /*A*/ "o5 e1 | o4 b1 "
      /*B*/ "o5 e1 | o4 b1 "
      /*C*/ "o5 e1 | o4 b1 ",
    "@8 t120 v6 q8 me0 k0 "
      /*A*/ "o4 g1 | d1 "
      /*B*/ "o4 g1 | d1 "
      /*C*/ "o4 g1 | d1 ",
    "@8 t120 v6 q8 me0 k8 "
      /*A*/ "o5 c1 | o4 g1 "
      /*B*/ "o5 c1 | o4 g1 "
      /*C*/ "o5 c1 | o4 g1 ",
    0
  } },
  // THICK -- k: one chord with its three voices in tune, then a few cents apart, then wide
  { "THICK", {
    "@w8={" WAV_STRING "} "
      "@8 t100 v10 q8 me0 mp3,20,300 "
      /*A*/ "o6 a1 | o6 f1 "
      /*B*/ "o6 a1 | o6 f1 "
      /*C*/ "o6 a1 | o6 f1 ",
    "@8 t100 v11 q8 me1 "
      /*A*/ "o2 a1 | f1 "
      /*B*/ "o2 a1 | f1 "
      /*C*/ "o2 a1 | f1 ",
    "@8 t100 v7 q8 me0 k0 "
      /*A*/ "k0 o5 c1 | o4 a1 "
      /*B*/ "k-8 o5 c1 | o4 a1 "
      /*C*/ "k-25 o5 c1 | o4 a1 ",
    "@8 t100 v7 q8 me0 k0 "
      /*A*/ "k0 o5 e1 | c1 "
      /*B*/ "k0 o5 e1 | c1 "
      /*C*/ "k0 o5 e1 | c1 ",
    "@8 t100 v7 q8 me0 k0 "
      /*A*/ "k0 o4 a1 | f1 "
      /*B*/ "k8 o4 a1 | f1 "
      /*C*/ "k25 o4 a1 | f1 ",
    0
  } },
  // WAVETABLE -- the same four bars through eight waveforms of its own, then four factory tones
  { "WAVETABLE", {
    "@w8={" WAV_TRI "}@w9={" WAV_SINE "}@w10={" WAV_PULSE25 "}@w11={" WAV_PULSE12 "}@w12={" WAV_ORGAN "}@w13={" WAV_BELL "}@w14={" WAV_VOX "}@w15={" WAV_METAL "} "
      "@8 t120 v11 q7 me2 mp0 "
      /*A*/ "@8 o5 c8 e8 g8 o6 c8 o5 g8 e8 c8 e8 | @9 o5 a8 o6 c8 e8 a8 e8 o5 c8 a8 o6 c8 | "
            "@10 o5 f8 a8 o6 c8 f8 c8 o5 a8 f8 a8 | @11 o5 g8 b8 o6 d8 g8 d8 o5 b8 g8 b8 "
      /*B*/ "@12 o5 c8 e8 g8 o6 c8 o5 g8 e8 c8 e8 | "
            "@13 o5 a8 o6 c8 e8 a8 e8 o5 c8 a8 o6 c8 | "
            "@14 o5 f8 a8 o6 c8 f8 c8 o5 a8 f8 a8 | @15 o5 g8 b8 o6 d8 g8 d8 o5 b8 g8 b8 "
      /*C*/ "@0 o5 c8 e8 g8 o6 c8 o5 g8 e8 c8 e8 | @1 o5 a8 o6 c8 e8 a8 e8 o5 c8 a8 o6 c8 | "
            "@2 o5 f8 a8 o6 c8 f8 c8 o5 a8 f8 a8 | @3 o5 g8 b8 o6 d8 g8 d8 o5 b8 g8 b8 ",
    "@1 t120 v10 q6 me4 "
      /*A*/ "o2 [c4]4 | [a4]4 | [f4]4 | [g4]4 "
      /*B*/ "o2 [c4]4 | [a4]4 | [f4]4 | [g4]4 "
      /*C*/ "o2 [c4]4 | [a4]4 | [f4]4 | [g4]4 ",
    "@3 t120 v6 q8 me0 mv3,20 k-8 "
      /*A*/ "o5 e1 | c1 | o4 a1 | b1 "
      /*B*/ "o5 e1 | c1 | o4 a1 | b1 "
      /*C*/ "o5 e1 | c1 | o4 a1 | b1 ",
    "@3 t120 v6 q8 me0 mv3,20 k0 "
      /*A*/ "o4 g1 | e1 | c1 | d1 "
      /*B*/ "o4 g1 | e1 | c1 | d1 "
      /*C*/ "o4 g1 | e1 | c1 | d1 ",
    "@3 t120 v6 q8 me0 mv3,20 k8 "
      /*A*/ "o5 c1 | o4 a1 | f1 | g1 "
      /*B*/ "o5 c1 | o4 a1 | f1 | g1 "
      /*C*/ "o5 c1 | o4 a1 | f1 | g1 ",
    "@n t120 q3 "
      /*A*/ "[ [ v9 o6 d+16 r16 d+16 r16 ]4 ]4 "
      /*B*/ "[ [ v9 o6 d+16 r16 d+16 r16 ]4 ]4 "
      /*C*/ "[ [ v9 o6 d+16 r16 d+16 r16 ]4 ]4 "
  } },
};
static const int BGM_COUNT = (int)( sizeof(BGM_DEFS) / sizeof(BGM_DEFS[0]) );
