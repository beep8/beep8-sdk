// Sound helper audition tool: 48 six-track MML pieces and all 27 SFX presets,
// side by side, so both halves of <sound.h> can be listened to and tuned by ear.
//
// The music is what the six BGM channels are for: every piece runs a melody, a
// bass and a three-voice chord detuned a few cents apart (see 'k'), so the pad
// beats gently against itself instead of sitting still, plus a drum track on the
// noise generator. Nearly all of them lean on the modulation layer -- 'mp'
// vibrato on the held leads, a slow 'mv' tremolo across the pad, 'me0' with a
// fade-in where the chord should swell rather than pluck.
//
// The tracks are generated on a 16-step bar grid, which is what keeps them
// phase-locked: every track of a piece is exactly the same number of whole notes
// long, and each loops back to its own start independently (see sound.cpp's
// track_advance), so a mismatch would drift them apart within a few bars.
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

struct BgmDef {
  const char* name;
  const char* t[6];      // melody / bass / three pad voices / drums
};

static const BgmDef BGM_DEFS[] = {
  { "SUNRISE", {
    "t112 @5 v11 q8 me1 mp5,30,250 o6 c2 f2 | "
      "o5 g2 o6 c4 o5 g4 | "
      "o6 a2 f4 o5 a4 | "
      "o7 d2. o6 a+4",
    "t112 @1 v11 q6 me3 o2 f2 f2 | "
      "o3 c2 c2 | "
      "d2 d2 | "
      "o2 a+2 a+2",
    "t112 @3 v6 q8 me0,400 mv3,22 k-8 o4 f1 | "
      "c1 | "
      "d1 | "
      "a+1",
    "t112 @3 v6 q8 me0,400 mv3,22 o4 a1 | "
      "e1 | "
      "f1 | "
      "o5 d1",
    "t112 @3 v6 q8 me0,400 mv3,22 k8 o5 c1 | "
      "o4 g1 | "
      "a1 | "
      "o5 f1",
    "t112 @3 v5 q8 me0,400 mv3,22 k-8 o6 f1 | "
      "c1 | "
      "d1 | "
      "a+1" } },
  { "MEADOW", {
    "t126 @0 v11 q6 me4 o5 d8 f+8 a8 o6 d8 f+8 d8 o5 a8 f+8 | "
      "o6 d4 o5 b4 g2 | "
      "d8 f+8 a8 o6 d8 f+8 d8 o5 a8 f+8 | "
      "o6 b2. g4",
    "t126 @1 v11 q6 me4 o2 d4 o3 d4 o2 d4 o3 d4 | "
      "o2 g4 o3 g4 o2 g4 o3 g4 | "
      "o2 d4 o3 d4 o2 d4 o3 d4 | "
      "o2 g4 o3 g4 o2 g4 o3 g4",
    "t126 @3 v6 q8 me0 mv4,25 k-7 o4 d1 | "
      "g1 | "
      "d1 | "
      "g1",
    "t126 @3 v6 q8 me0 mv4,25 o4 f+1 | "
      "b1 | "
      "f+1 | "
      "b1",
    "t126 @3 v6 q8 me0 mv4,25 k7 o4 a1 | "
      "o5 d1 | "
      "o4 a1 | "
      "o5 d1",
    "@n t126 q3 v13 o1 a16 r8. v10 o6 d+16 r8. v12 o4 d+16 r8. v10 o6 d+16 r8. | "
      "v13 o1 a16 r8. v10 o6 d+16 r8. v12 o4 d+16 r8. v10 o6 d+16 r8. | "
      "v13 o1 a16 r8. v10 o6 d+16 r8. v12 o4 d+16 r8. v10 o6 d+16 r8. | "
      "v13 o1 a16 r8. v10 o6 d+16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 a16 r16 v12 o4 d+16 r16 v10 o6 d+16 r16" } },
  { "SKYLINE", {
    "t138 @4 v11 q6 me2 mp6,32,150 o6 e4 c+8 o5 g+8 o6 c+4 o5 e4 | "
      "a8. o6 c+8. e8 o7 c+4 o6 a4 | "
      "g+4 e8 o5 b8 o6 e4 o5 g+4 | "
      "f+4 d+4 o4 b2",
    "t138 @1 v11 q6 me4 o3 c+8 c+8 o4 c+8 o3 c+8 c+8 c+8 o4 c+8 o3 c+8 | "
      "o2 a8 a8 o3 a8 o2 a8 a8 a8 o3 a8 o2 a8 | "
      "e8 e8 o3 e8 o2 e8 e8 e8 o3 e8 o2 e8 | "
      "b8 b8 o3 b8 o2 b8 b8 b8 o3 b8 o2 b8",
    "t138 @5 v6 q8 me0 mv5,30 k-9 o4 c+2 c+2 | "
      "a2 a2 | "
      "e2 e2 | "
      "o3 b2 b2",
    "t138 @5 v6 q8 me0 mv5,30 o4 e2 e2 | "
      "o5 c+2 c+2 | "
      "o4 g+2 g+2 | "
      "d+2 d+2",
    "t138 @5 v6 q8 me0 mv5,30 k9 o4 g+2 g+2 | "
      "o5 e2 e2 | "
      "o4 b2 b2 | "
      "f+2 f+2",
    "@n t138 q3 v13 o1 a16 r8. v10 o6 d+16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 a16 r16 v12 o4 d+16 r16 v10 o6 d+16 r16 | "
      "v13 o1 a16 r8. v10 o6 d+16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 a16 r16 v12 o4 d+16 r16 v10 o6 d+16 r16 | "
      "v13 o1 a16 r8. v10 o6 d+16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 a16 r16 v12 o4 d+16 r16 v10 o6 d+16 r16 | "
      "v13 o1 a16 r8. v10 o6 d+16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 a16 r16 v12 o4 d+16 r16 v10 o6 d+16 r16" } },
  { "VICTORY", {
    "t150 @0 v11 q7 me2 mp7,25,200 o6 e4 e4 c8 o5 g8 e4 | "
      "b4 b4 g8 d8 o4 b4 | "
      "a16 o5 c16 e16 a16 o6 c16 e16 c16 o5 a16 e16 a16 o6 c16 e16 c16 o5 a16 e8 | "
      "o6 c4 o5 a4 f2",
    "t150 @1 v11 q6 me4 o2 c4 c4 c4 c4 | "
      "g4 g4 g4 g4 | "
      "a4 a4 a4 a4 | "
      "f4 f4 f4 f4",
    "t150 @3 v6 q8 me0 mv4,20 k-7 o4 e4 e4 e4 e4 | "
      "o3 b4 b4 b4 b4 | "
      "o4 c4 c4 c4 c4 | "
      "a4 a4 a4 a4",
    "t150 @3 v6 q8 me0 mv4,20 o4 g4 g4 g4 g4 | "
      "d4 d4 d4 d4 | "
      "e4 e4 e4 e4 | "
      "o5 c4 c4 c4 c4",
    "t150 @3 v6 q8 me0 mv4,20 k7 o5 c4 c4 c4 c4 | "
      "o4 g4 g4 g4 g4 | "
      "a4 a4 a4 a4 | "
      "o5 f4 f4 f4 f4",
    "@n t150 q3 v13 o1 a16 r8. v10 o6 d+16 r8. v12 o4 d+16 r8. v10 o6 d+16 r8. | "
      "v13 o1 a16 r8. v10 o6 d+16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 a16 r16 v12 o4 d+16 r16 v10 o6 d+16 r16 | "
      "v13 o1 a16 r8. v10 o6 d+16 r8. v12 o4 d+16 r8. v10 o6 d+16 r8. | "
      "v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16" } },
  { "PARADE", {
    "t132 @7 v11 q6 me3 mp5,20,300 o6 b4 b4 g8 d8 o5 b4 | "
      "o6 c4 o5 a4 f2 | "
      "o7 e4 e4 c8 o6 g8 e4 | "
      "o5 g16 b16 o6 d16 g16 b16 o7 d16 o6 b16 g16 d16 g16 b16 o7 d16 o6 b16 g16 d8",
    "t132 @1 v11 q6 me4 o2 g4 o3 g4 o2 g4 o3 g4 | "
      "f4 o4 f4 o3 f4 o4 f4 | "
      "o3 c4 o4 c4 o3 c4 o4 c4 | "
      "o2 g4 o3 g4 o2 g4 o3 g4",
    "t132 @3 v6 q8 me0 mv3,25 k-7 o4 g2 g2 | "
      "f2 f2 | "
      "o5 c2 c2 | "
      "o4 g2 g2",
    "t132 @3 v6 q8 me0 mv3,25 o4 b2 b2 | "
      "a2 a2 | "
      "o5 e2 e2 | "
      "o4 b2 b2",
    "t132 @3 v6 q8 me0 mv3,25 k7 o5 d2 d2 | "
      "c2 c2 | "
      "g2 g2 | "
      "d2 d2",
    "@n t132 q3 v13 o1 a16 r8. v10 o6 d+16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 a16 r16 v12 o4 d+16 r16 v10 o6 d+16 r16 | "
      "v13 o1 a16 r8. v10 o6 d+16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 a16 r16 v12 o4 d+16 r16 v10 o6 d+16 r16 | "
      "v13 o1 a16 r8. v10 o6 d+16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 a16 r16 v12 o4 d+16 r16 v10 o6 d+16 r16 | "
      "v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16" } },
  { "FANFARE", {
    "t150 @5 v11 q7 me1 mp6,40,120 o6 e4 r4 g4 r4 | "
      "o5 f16 a16 o6 c16 f16 a16 o7 c16 o6 a16 f16 c16 f16 a16 o7 c16 o6 a16 f16 c8 | "
      "e4 r4 g4 r4 | "
      "c4 o5 a4 f2",
    "t150 @1 v11 q6 me4 o2 c4 c4 c4 c4 | "
      "f4 f4 f4 f4 | "
      "c4 c4 c4 c4 | "
      "f4 f4 f4 f4",
    "t150 @3 v6 q8 me0,300 mv3,18 k-10 o4 c1 | "
      "f1 | "
      "c1 | "
      "f1",
    "t150 @3 v6 q8 me0,300 mv3,18 o4 g1 | "
      "o5 c1 | "
      "o4 g1 | "
      "o5 c1",
    "t150 @3 v6 q8 me0,300 mv3,18 k10 o5 e1 | "
      "a1 | "
      "e1 | "
      "a1",
    "@n t150 q3 v13 o1 a16 r8. v10 o6 d+16 r8. v12 o4 d+16 r8. v10 o6 d+16 r8. | "
      "v13 o1 a16 r8. v10 o6 d+16 r8. v12 o4 d+16 r8. v10 o6 d+16 r8. | "
      "v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 | "
      "v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16" } },
  { "TWILIGHT", {
    "t96 @5 v11 q8 me1 mp4,38,300 o6 c2. o5 a4 | "
      "c2 f4 c4 | "
      "o6 e2. c4 | "
      "d2 o5 b4 d4",
    "t96 @1 v11 q6 me2 o1 a1 | "
      "o2 f1 | "
      "c1 | "
      "g1",
    "t96 @3 v6 q8 me0 mv3,30 k-9 o3 a1 | "
      "f1 | "
      "o4 c1 | "
      "o3 g1",
    "t96 @3 v6 q8 me0 mv3,30 o4 c1 | "
      "o3 a1 | "
      "o4 e1 | "
      "o3 b1",
    "t96 @3 v6 q8 me0 mv3,30 k9 o4 e1 | "
      "c1 | "
      "g1 | "
      "d1",
    "t96 @3 v5 q8 me0 mv3,30 k-9 o5 a1 | "
      "f1 | "
      "o6 c1 | "
      "o5 g1" } },
  { "CAVERN", {
    "t88 @3 v11 q8 me0 mp3,45,400 o5 f1 | "
      "o6 f+2. d+4 | "
      "o5 c1 | "
      "f2 a+4 f4",
    "t88 @1 v11 q6 me1 o1 a+1 | "
      "o2 d+1 | "
      "f1 | "
      "o1 a+1",
    "t88 @5 v6 q8 me0,500 mv2,35 k-12 o3 a+1 | "
      "o4 d+1 | "
      "o3 f1 | "
      "a+1",
    "t88 @5 v6 q8 me0,500 mv2,35 o4 c+1 | "
      "f+1 | "
      "o3 g+1 | "
      "o4 c+1",
    "t88 @5 v6 q8 me0,500 mv2,35 k12 o4 f1 | "
      "a+1 | "
      "c1 | "
      "f1",
    "@n t88 q3 v13 o1 a16 r4. r16 v12 o4 d+16 r4. r16 | "
      "v13 o1 a16 r4. r16 v12 o4 d+16 r4. r16 | "
      "v13 o1 a16 r4. r16 v12 o4 d+16 r4. r16 | "
      "v13 o1 a16 r4. r16 v12 o4 d+16 r4. r16" } },
  { "NIGHTFALL", {
    "t104 @4 v11 q6 me2 mp5,35,220 o5 a2 o6 d4 o5 a4 | "
      "g4 e4 c2 | "
      "o6 d2. o5 a+4 | "
      "o6 e2 c+4 o5 e4",
    "t104 @1 v11 q6 me4 o2 d2 d2 | "
      "o3 c2 c2 | "
      "o2 a+2 a+2 | "
      "a2 a2",
    "t104 @3 v6 q8 me0 mv3,26 k-7 o4 d1 | "
      "c1 | "
      "o3 a+1 | "
      "a1",
    "t104 @3 v6 q8 me0 mv3,26 o4 f1 | "
      "e1 | "
      "d1 | "
      "c+1",
    "t104 @3 v6 q8 me0 mv3,26 k7 o4 a1 | "
      "g1 | "
      "f1 | "
      "e1",
    "@n t104 q3 v13 o1 a16 r16 a16 r4 r16 v12 o4 d+16 r4. r16 | "
      "v13 o1 a16 r16 a16 r4 r16 v12 o4 d+16 r4. r16 | "
      "v13 o1 a16 r16 a16 r4 r16 v12 o4 d+16 r4. r16 | "
      "v13 o1 a16 r16 a16 r4 r16 v12 o4 d+16 r4. r16" } },
  { "SHADOW", {
    "t100 @0 v11 q6 me6 o5 f+8 r8 f+8 r8 b8 r8 o6 d8 r8 | "
      "d8 r8 o5 f+4 r8 b8 r4 | "
      "d8 r8 d8 r8 g8 r8 b8 r8 | "
      "e4 c+4 o4 a2",
    "t100 @1 v11 q6 me4 o1 b8 b8 b8 b8 b8 b8 b8 b8 | "
      "b8 b8 b8 b8 b8 b8 b8 b8 | "
      "o2 g8 g8 g8 g8 g8 g8 g8 g8 | "
      "a8 a8 a8 a8 a8 a8 a8 a8",
    "t100 @5 v6 q8 me0 mv6,35 k-7 o3 b2 b2 | "
      "b2 b2 | "
      "g2 g2 | "
      "a2 a2",
    "t100 @5 v6 q8 me0 mv6,35 o4 d2 d2 | "
      "d2 d2 | "
      "o3 b2 b2 | "
      "o4 c+2 c+2",
    "t100 @5 v6 q8 me0 mv6,35 k7 o4 f+2 f+2 | "
      "f+2 f+2 | "
      "d2 d2 | "
      "e2 e2",
    "@n t100 q3 v13 o1 a16 r8. v10 o6 d+16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 a16 r16 v12 o4 d+16 r16 v10 o6 d+16 r16 | "
      "v13 o1 a16 r8. v10 o6 d+16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 a16 r16 v12 o4 d+16 r16 v10 o6 d+16 r16 | "
      "v13 o1 a16 r8. v10 o6 d+16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 a16 r16 v12 o4 d+16 r16 v10 o6 d+16 r16 | "
      "v13 o1 a16 r8 v10 o6 d+16 r8 v12 o4 d+16 r16 v13 o1 a16 r8 v10 o6 d+16 r8 v12 o4 d+16 r16" } },
  { "REQUIEM", {
    "t76 @5 v10 q8 me0 mp3,30,500 o5 e1 | "
      "o6 d2. o5 b4 | "
      "g1 | "
      "o4 b2 o5 e4 o4 b4",
    "t76 @3 v11 q6 me0 o1 a1 | "
      "b1 | "
      "o2 c1 | "
      "e1",
    "t76 @3 v6 q8 me0,600 mv2,28 k-6 o3 a1 | "
      "b1 | "
      "o4 c1 | "
      "o3 e1",
    "t76 @3 v6 q8 me0,600 mv2,28 o4 c1 | "
      "d1 | "
      "e1 | "
      "o3 g1",
    "t76 @3 v6 q8 me0,600 mv2,28 k6 o4 e1 | "
      "f1 | "
      "g1 | "
      "o3 b1",
    "t76 @3 v5 q8 me0,600 mv2,28 k-6 o5 a1 | "
      "b1 | "
      "o6 c1 | "
      "o5 e1" } },
  { "GLACIER", {
    "t84 @4 v11 q6 me1 mp4,25,350 o6 d+2. f4 | "
      "d+1 | "
      "d+2 o5 b4 f+4 | "
      "o6 c+1",
    "t84 @1 v11 q6 me4 o2 d+1 | "
      "g+1 | "
      "b1 | "
      "f+1",
    "t84 @5 v6 q8 me0,450 mv3,32 k-11 o4 f+1 | "
      "b1 | "
      "d+1 | "
      "a+1",
    "t84 @5 v6 q8 me0,450 mv3,32 o4 a+1 | "
      "o5 d+1 | "
      "o4 f+1 | "
      "o5 c+1",
    "t84 @5 v6 q8 me0,450 mv3,32 k11 o5 f1 | "
      "a+1 | "
      "c+1 | "
      "g+1",
    "@n t84 q3 v10 o6 d+16 r8. d+16 r8. d+16 r8. d+16 r8. | "
      "d+16 r8. d+16 r8. d+16 r8. d+16 r8. | "
      "d+16 r8. d+16 r8. d+16 r8. d+16 r8. | "
      "d+16 r8. d+16 r8. d+16 r8. d+16 r8." } },
  { "NEBULA", {
    "t72 @5 v10 q8 me0 mp3,40,400 o5 g1 | "
      "r1 | "
      "o6 c2 f2 | "
      "r1",
    "t72 @3 v10 q6 me0 o2 c1 | "
      "c1 | "
      "f1 | "
      "f1",
    "t72 @3 v6 q8 me0,700 mv2,30 k-13 o4 c1 | "
      "c1 | "
      "f1 | "
      "f1",
    "t72 @3 v6 q8 me0,700 mv2,30 o4 f1 | "
      "e1 | "
      "a+1 | "
      "a1",
    "t72 @3 v6 q8 me0,700 mv2,30 k13 o4 g1 | "
      "g1 | "
      "o5 c1 | "
      "c1",
    "t72 @3 v5 q8 me0,700 mv2,30 k-13 o6 c1 | "
      "c1 | "
      "f1 | "
      "f1" } },
  { "DRIFT", {
    "t68 @3 v10 q8 me0 mp2,35,600 mg200 o6 c1 | "
      "d+2 g+2 | "
      "f1 | "
      "g+2. d+4",
    "t68 @1 v11 q6 me0 o2 f1 | "
      "g+1 | "
      "a+1 | "
      "o3 d+1",
    "t68 @5 v6 q8 me0,800 mv2,25 k-10 o4 f1 | "
      "g+1 | "
      "a+1 | "
      "d+1",
    "t68 @5 v6 q8 me0,800 mv2,25 o4 a+1 | "
      "o5 c+1 | "
      "d+1 | "
      "o4 g+1",
    "t68 @5 v6 q8 me0,800 mv2,25 k10 o5 c1 | "
      "d+1 | "
      "f1 | "
      "o4 a+1",
    "t68 @5 v5 q8 me0,800 mv2,25 k-10 o6 f1 | "
      "g+1 | "
      "a+1 | "
      "d+1" } },
  { "AURORA", {
    "t80 @4 v11 q8 me1 mp4,28,300 o6 e2. d+4 | "
      "e2 c+4 o5 g+4 | "
      "o6 c+2 e4 c+4 | "
      "o5 f+1",
    "t80 @1 v11 q6 me1 o2 e2 e2 | "
      "o3 c+2 c+2 | "
      "o2 f+2 f+2 | "
      "b2 b2",
    "t80 @3 v6 q8 me0,500 mv3,28 k-7 o4 g+1 | "
      "e1 | "
      "a1 | "
      "d+1",
    "t80 @3 v6 q8 me0,500 mv3,28 o4 b1 | "
      "g+1 | "
      "o5 c+1 | "
      "o4 f+1",
    "t80 @3 v6 q8 me0,500 mv3,28 k7 o5 d+1 | "
      "o4 b1 | "
      "o5 e1 | "
      "o4 a1",
    "@n t80 q3 v10 o6 d+16 r8. d+16 r8. d+16 r8. d+16 r8. | "
      "d+16 r8. d+16 r8. d+16 r8. d+16 r8. | "
      "d+16 r8. d+16 r8. d+16 r8. d+16 r8. | "
      "d+16 r8. d+16 r8. d+16 r8. d+16 r8." } },
  { "TIDE", {
    "t74 @5 v10 q8 me0 mv4,30 mp3,20,400 o5 a2 o6 d2 | "
      "b2. g4 | "
      "o5 a2 o6 d2 | "
      "o7 d2 o6 b4 d4",
    "t74 @1 v11 q6 me0 o2 d1 | "
      "g1 | "
      "d1 | "
      "g1",
    "t74 @3 v6 q8 me0,600 mv2,35 k-7 o4 d1 | "
      "g1 | "
      "d1 | "
      "g1",
    "t74 @3 v6 q8 me0,600 mv2,35 o4 f+1 | "
      "b1 | "
      "f+1 | "
      "b1",
    "t74 @3 v6 q8 me0,600 mv2,35 k7 o4 a1 | "
      "o5 d1 | "
      "o4 a1 | "
      "o5 d1",
    "t74 @3 v5 q8 me0,600 mv2,35 k-7 o6 d1 | "
      "g1 | "
      "d1 | "
      "g1" } },
  { "MONOLITH", {
    "t66 @6 v9 q8 me0 mp2,50,500 o4 a+1 | "
      "r1 | "
      "o5 c+1 | "
      "r1",
    "t66 @1 v11 q6 me0 o1 a+1 | "
      "a+1 | "
      "o2 c+1 | "
      "d+1",
    "t66 @3 v7 q8 me0,900 mv2,40 k-15 o3 a+1 | "
      "a+1 | "
      "o4 c+1 | "
      "d+1",
    "t66 @3 v7 q8 me0,900 mv2,40 o4 f1 | "
      "f1 | "
      "g+1 | "
      "a+1",
    "t66 @3 v7 q8 me0,900 mv2,40 k15 o4 a+1 | "
      "a+1 | "
      "o5 c+1 | "
      "d+1",
    "@n t66 q3 v13 o1 a16 r4. r16 v12 o4 d+16 r4. r16 | "
      "v13 o1 a16 r4. r16 v12 o4 d+16 r4. r16 | "
      "v13 o1 a16 r4. r16 v12 o4 d+16 r4. r16 | "
      "v13 o1 a16 r4. r16 v12 o4 d+16 r4. r16" } },
  { "LULLABY", {
    "t90 @5 v10 q8 me2 mp5,25,250 o6 d2 g4 d4 | "
      "o5 a4. o6 d4. o5 f+4 | "
      "b2 o6 e4 o5 b4 | "
      "o7 e2. c4",
    "t90 @1 v10 q6 me4 o2 g2 g2 | "
      "o3 d2 d2 | "
      "e2 e2 | "
      "c2 c2",
    "t90 @3 v6 q8 me0 mv3,20 k-7 o4 g1 | "
      "d1 | "
      "e1 | "
      "o5 c1",
    "t90 @3 v6 q8 me0 mv3,20 o4 b1 | "
      "f+1 | "
      "g1 | "
      "o5 e1",
    "t90 @3 v6 q8 me0 mv3,20 k7 o5 d1 | "
      "o4 a1 | "
      "b1 | "
      "o5 g1",
    "t90 @3 v5 q8 me0 mv3,20 k-7 o6 g1 | "
      "d1 | "
      "e1 | "
      "o7 c1" } },
  { "CHASE", {
    "t168 @0 v11 q5 me7 o4 a16 o5 c16 e16 a16 o6 c16 o5 a16 e16 c16 o4 a16 o5 c16 e16 a16 o6 c16 o5 a16 e16 c16 | "
      "o4 g16 b16 o5 d16 g16 b16 o6 d16 o5 b16 g16 d16 g16 b16 o6 d16 o5 b16 g16 d8 | "
      "o4 f16 a16 o5 c16 f16 a16 f16 c16 o4 a16 f16 a16 o5 c16 f16 a16 f16 c16 o4 a16 | "
      "e8. g+8. b8 o5 g+4 e4",
    "t168 @1 v11 q6 me5 o1 a8 a8 o2 a8 o1 a8 a8 a8 o2 a8 o1 a8 | "
      "o2 g8 g8 o3 g8 o2 g8 g8 g8 o3 g8 o2 g8 | "
      "f8 f8 o3 f8 o2 f8 f8 f8 o3 f8 o2 f8 | "
      "e8 e8 o3 e8 o2 e8 e8 e8 o3 e8 o2 e8",
    "t168 @7 v6 q8 me0 mv7,30 k-7 o3 a4 a4 a4 a4 | "
      "g4 g4 g4 g4 | "
      "f4 f4 f4 f4 | "
      "e4 e4 e4 e4",
    "t168 @7 v6 q8 me0 mv7,30 o4 c4 c4 c4 c4 | "
      "o3 b4 b4 b4 b4 | "
      "a4 a4 a4 a4 | "
      "g+4 g+4 g+4 g+4",
    "t168 @7 v6 q8 me0 mv7,30 k7 o4 e4 e4 e4 e4 | "
      "d4 d4 d4 d4 | "
      "c4 c4 c4 c4 | "
      "o3 b4 b4 b4 b4",
    "@n t168 q3 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 | "
      "v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 | "
      "v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 | "
      "v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16" } },
  { "BOSS RUSH", {
    "t176 @6 v11 q6 me4 mp8,30,80 o6 d8 r8 o5 f+4 r8 b8 r4 | "
      "c16 e16 g16 o6 c16 e16 g16 e16 c16 o5 g16 o6 c16 e16 g16 e16 c16 o5 g8 | "
      "o6 d8 r8 o5 f+4 r8 b8 r4 | "
      "o4 a+8. o5 d8. f8 o6 d4 o5 a+4",
    "t176 @1 v11 q6 me4 o1 b16 o2 f+16 b16 f+16 o1 b16 o2 f+16 b16 f+16 o1 b16 o2 f+16 b16 f+16 o1 b16 o2 f+16 b16 f+16 | "
      "c16 g16 o3 c16 o2 g16 c16 g16 o3 c16 o2 g16 c16 g16 o3 c16 o2 g16 c16 g16 o3 c16 o2 g16 | "
      "o1 b16 o2 f+16 b16 f+16 o1 b16 o2 f+16 b16 f+16 o1 b16 o2 f+16 b16 f+16 o1 b16 o2 f+16 b16 f+16 | "
      "a+16 o3 f16 a+16 f16 o2 a+16 o3 f16 a+16 f16 o2 a+16 o3 f16 a+16 f16 o2 a+16 o3 f16 a+16 f16",
    "t176 @0 v6 q8 me0 mv8,40 k-7 o3 b8 b8 b8 b8 b8 b8 b8 b8 | "
      "o4 c8 c8 c8 c8 c8 c8 c8 c8 | "
      "o3 b8 b8 b8 b8 b8 b8 b8 b8 | "
      "a+8 a+8 a+8 a+8 a+8 a+8 a+8 a+8",
    "t176 @0 v6 q8 me0 mv8,40 o4 d8 d8 d8 d8 d8 d8 d8 d8 | "
      "e8 e8 e8 e8 e8 e8 e8 e8 | "
      "d8 d8 d8 d8 d8 d8 d8 d8 | "
      "d8 d8 d8 d8 d8 d8 d8 d8",
    "t176 @0 v6 q8 me0 mv8,40 k7 o4 f+8 f+8 f+8 f+8 f+8 f+8 f+8 f+8 | "
      "g8 g8 g8 g8 g8 g8 g8 g8 | "
      "f+8 f+8 f+8 f+8 f+8 f+8 f+8 f+8 | "
      "f8 f8 f8 f8 f8 f8 f8 f8",
    "@n t176 q3 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 | "
      "v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 | "
      "v13 o1 a16 r8. v10 o6 d+16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 a16 r16 v12 o4 d+16 r16 v10 o6 d+16 r16 | "
      "v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16" } },
  { "OVERDRIVE", {
    "t184 @1 v11 q5 me6 mp7,20,100 o5 d16 a16 o6 d16 d16 a16 o7 d16 o6 a16 d16 d16 d16 a16 o7 d16 o6 a16 d16 d8 | "
      "o5 d16 a16 o6 d16 d16 a16 d16 d16 o5 a16 d16 a16 o6 d16 d16 a16 d16 d16 o5 a16 | "
      "f16 o6 c16 f16 f16 o7 c16 f16 c16 o6 f16 f16 f16 o7 c16 f16 c16 o6 f16 f8 | "
      "g8 r8 g8 r8 g8 r8 o7 d8 r8",
    "t184 @1 v11 q6 me4 o2 d8 d8 o3 d8 o2 d8 d8 d8 o3 d8 o2 d8 | "
      "d8 d8 o3 d8 o2 d8 d8 d8 o3 d8 o2 d8 | "
      "f8 f8 o3 f8 o2 f8 f8 f8 o3 f8 o2 f8 | "
      "g8 g8 o3 g8 o2 g8 g8 g8 o3 g8 o2 g8",
    "t184 @7 v6 q8 me0 mv6,35 k-7 o4 d4 d4 d4 d4 | "
      "d4 d4 d4 d4 | "
      "f4 f4 f4 f4 | "
      "g4 g4 g4 g4",
    "t184 @7 v6 q8 me0 mv6,35 o4 a4 a4 a4 a4 | "
      "a4 a4 a4 a4 | "
      "o5 c4 c4 c4 c4 | "
      "d4 d4 d4 d4",
    "t184 @7 v6 q8 me0 mv6,35 k7 o5 d4 d4 d4 d4 | "
      "d4 d4 d4 d4 | "
      "f4 f4 f4 f4 | "
      "g4 g4 g4 g4",
    "@n t184 q3 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 | "
      "v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 | "
      "v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 | "
      "v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16" } },
  { "IRONWORKS", {
    "t156 @6 v11 q6 me5 o6 c+4 c+4 o5 a+8 f8 c+4 | "
      "o6 c+8 r8 o5 e4 r8 a+8 r4 | "
      "a+4 a+4 f+8 c+8 o4 a+4 | "
      "o5 c4 o4 a4 f2",
    "t156 @1 v11 q6 me4 o1 a+8. a+8. a+8 o2 a+4 o1 a+4 | "
      "a+8. a+8. a+8 o2 a+4 o1 a+4 | "
      "o2 f+8. f+8. f+8 o3 f+4 o2 f+4 | "
      "f8. f8. f8 o3 f4 o2 f4",
    "t156 @2 v6 q8 me0 mv5,30 k-7 o3 a+2 a+2 | "
      "a+2 a+2 | "
      "f+2 f+2 | "
      "f2 f2",
    "t156 @2 v6 q8 me0 mv5,30 o4 c+2 c+2 | "
      "c+2 c+2 | "
      "o3 a+2 a+2 | "
      "a2 a2",
    "t156 @2 v6 q8 me0 mv5,30 k7 o4 f2 f2 | "
      "e2 e2 | "
      "c+2 c+2 | "
      "c2 c2",
    "@n t156 q3 v13 o1 a16 r8. v10 o6 d+16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 a16 r16 v12 o4 d+16 r16 v10 o6 d+16 r16 | "
      "v13 o1 a16 r8 v10 o6 d+16 r8 v12 o4 d+16 r16 v13 o1 a16 r8 v10 o6 d+16 r8 v12 o4 d+16 r16 | "
      "v13 o1 a16 r8. v10 o6 d+16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 a16 r16 v12 o4 d+16 r16 v10 o6 d+16 r16 | "
      "v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16" } },
  { "RAMPAGE", {
    "t190 @0 v11 q5 me8 o5 c16 d+16 g16 o6 c16 d+16 c16 o5 g16 d+16 c16 d+16 g16 o6 c16 d+16 c16 o5 g16 d+16 | "
      "o6 d+8 r8 o5 g4 r8 o6 c8 r4 | "
      "o4 g+16 o5 c16 d+16 g+16 o6 c16 d+16 c16 o5 g+16 d+16 g+16 o6 c16 d+16 c16 o5 g+16 d+8 | "
      "o4 a+16 o5 d16 f16 a+16 o6 d16 o5 a+16 f16 d16 o4 a+16 o5 d16 f16 a+16 o6 d16 o5 a+16 f16 d16",
    "t190 @1 v11 q6 me6 o2 c16 g16 o3 c16 o2 g16 c16 g16 o3 c16 o2 g16 c16 g16 o3 c16 o2 g16 c16 g16 o3 c16 o2 g16 | "
      "c16 g16 o3 c16 o2 g16 c16 g16 o3 c16 o2 g16 c16 g16 o3 c16 o2 g16 c16 g16 o3 c16 o2 g16 | "
      "g+16 o3 d+16 g+16 d+16 o2 g+16 o3 d+16 g+16 d+16 o2 g+16 o3 d+16 g+16 d+16 o2 g+16 o3 d+16 g+16 d+16 | "
      "o2 a+16 o3 f16 a+16 f16 o2 a+16 o3 f16 a+16 f16 o2 a+16 o3 f16 a+16 f16 o2 a+16 o3 f16 a+16 f16",
    "t190 @1 v5 q8 me0 mv9,45 k-7 o4 c8 c8 c8 c8 c8 c8 c8 c8 | "
      "c8 c8 c8 c8 c8 c8 c8 c8 | "
      "o3 g+8 g+8 g+8 g+8 g+8 g+8 g+8 g+8 | "
      "a+8 a+8 a+8 a+8 a+8 a+8 a+8 a+8",
    "t190 @1 v5 q8 me0 mv9,45 o4 d+8 d+8 d+8 d+8 d+8 d+8 d+8 d+8 | "
      "d+8 d+8 d+8 d+8 d+8 d+8 d+8 d+8 | "
      "c8 c8 c8 c8 c8 c8 c8 c8 | "
      "d8 d8 d8 d8 d8 d8 d8 d8",
    "t190 @1 v5 q8 me0 mv9,45 k7 o4 g8 g8 g8 g8 g8 g8 g8 g8 | "
      "g8 g8 g8 g8 g8 g8 g8 g8 | "
      "d+8 d+8 d+8 d+8 d+8 d+8 d+8 d+8 | "
      "f8 f8 f8 f8 f8 f8 f8 f8",
    "@n t190 q3 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 | "
      "v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 | "
      "v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 | "
      "v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16" } },
  { "LAST STAND", {
    "t144 @4 v11 q7 me2 mp6,40,150 o6 c4 o5 a8 e8 a4 c4 | "
      "o6 c2 o5 a4 c4 | "
      "o6 e4 e4 c8 o5 g8 e4 | "
      "b2. g4",
    "t144 @1 v11 q6 me4 o1 a4 o2 a4 o1 a4 o2 a4 | "
      "f4 o3 f4 o2 f4 o3 f4 | "
      "o2 c4 o3 c4 o2 c4 o3 c4 | "
      "o2 g4 o3 g4 o2 g4 o3 g4",
    "t144 @3 v6 q8 me0 mv4,25 k-9 o3 a1 | "
      "f1 | "
      "o4 c1 | "
      "o3 g1",
    "t144 @3 v6 q8 me0 mv4,25 o4 e1 | "
      "c1 | "
      "g1 | "
      "d1",
    "t144 @3 v6 q8 me0 mv4,25 k9 o5 c1 | "
      "o4 a1 | "
      "o5 e1 | "
      "o4 b1",
    "@n t144 q3 v13 o1 a16 r8. v10 o6 d+16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 a16 r16 v12 o4 d+16 r16 v10 o6 d+16 r16 | "
      "v13 o1 a16 r8. v10 o6 d+16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 a16 r16 v12 o4 d+16 r16 v10 o6 d+16 r16 | "
      "v13 o1 a16 r8. v10 o6 d+16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 a16 r16 v12 o4 d+16 r16 v10 o6 d+16 r16 | "
      "v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16" } },
  { "NEON CITY", {
    "t118 @5 v11 q7 me3 mp5,25,220 o5 f8. g+8. o6 c8 f4 d+4 | "
      "o5 f4 d4 o4 a+2 | "
      "o6 d+4 d8 o5 a+8 o6 d4 o5 g4 | "
      "o6 d+2. d4",
    "t118 @1 v11 q6 me4 o2 f4 o3 c4 f4 c4 | "
      "o2 a+4 o3 f4 a+4 f4 | "
      "o2 d+4 a+4 o3 d+4 o2 a+4 | "
      "d+4 a+4 o3 d+4 o2 a+4",
    "t118 @3 v6 q8 me0 mv4,25 k-7 o4 g+2 g+2 | "
      "d2 d2 | "
      "g2 g2 | "
      "g2 g2",
    "t118 @3 v6 q8 me0 mv4,25 o5 c2 c2 | "
      "o4 f2 f2 | "
      "a+2 a+2 | "
      "a+2 a+2",
    "t118 @3 v6 q8 me0 mv4,25 k7 o5 d+2 d+2 | "
      "o4 g+2 g+2 | "
      "o5 d2 d2 | "
      "d2 d2",
    "@n t118 q3 v13 o1 a16 r8 v10 o6 d+16 r8 v12 o4 d+16 r16 v13 o1 a16 r8 v10 o6 d+16 r8 v12 o4 d+16 r16 | "
      "v13 o1 a16 r8 v10 o6 d+16 r8 v12 o4 d+16 r16 v13 o1 a16 r8 v10 o6 d+16 r8 v12 o4 d+16 r16 | "
      "v13 o1 a16 r8 v10 o6 d+16 r8 v12 o4 d+16 r16 v13 o1 a16 r8 v10 o6 d+16 r8 v12 o4 d+16 r16 | "
      "v13 o1 a16 r8 v10 o6 d+16 r8 v12 o4 d+16 r16 v13 o1 a16 r8 v10 o6 d+16 r8 v12 o4 d+16 r16" } },
  { "BLUE ROOM", {
    "t96 @4 v10 q8 me1 mp4,30,300 o6 c2 e4 c4 | "
      "d2. c4 | "
      "d4. f4. o5 a+4 | "
      "g4 e4 c2",
    "t96 @1 v10 q6 me4 o2 f4 o3 c4 f4 c4 | "
      "d4 a4 o4 d4 o3 a4 | "
      "o2 g4 o3 d4 g4 d4 | "
      "c4 g4 o4 c4 o3 g4",
    "t96 @5 v6 q8 me0 mv3,28 k-8 o4 a1 | "
      "f1 | "
      "a+1 | "
      "e1",
    "t96 @5 v6 q8 me0 mv3,28 o5 c1 | "
      "o4 a1 | "
      "o5 d1 | "
      "o4 g1",
    "t96 @5 v6 q8 me0 mv3,28 k8 o5 e1 | "
      "c1 | "
      "f1 | "
      "o4 a+1",
    "@n t96 q3 v13 o1 a16 r4. r16 v12 o4 d+16 r4. r16 | "
      "v13 o1 a16 r4. r16 v12 o4 d+16 r4. r16 | "
      "v13 o1 a16 r16 a16 r4 r16 v12 o4 d+16 r4. r16 | "
      "v13 o1 a16 r4. r16 v12 o4 d+16 r4. r16" } },
  { "CAFE", {
    "t110 @3 v10 q6 me4 o5 c8. e8. g8 o6 c4 o5 b4 | "
      "e4. g4. c4 | "
      "d8. f8. a8 o6 d4 c4 | "
      "o5 d4 o4 b4 g2",
    "t110 @1 v11 q6 me4 o2 c4 g4 o3 c4 o2 g4 | "
      "a4 o3 e4 a4 e4 | "
      "o2 d4 a4 o3 d4 o2 a4 | "
      "g4 o3 d4 g4 d4",
    "t110 @5 v6 q8 me0 mv3,22 k-7 o4 c2 c2 | "
      "o3 a2 a2 | "
      "o4 d2 d2 | "
      "o3 g2 g2",
    "t110 @5 v6 q8 me0 mv3,22 o4 e2 e2 | "
      "c2 c2 | "
      "f2 f2 | "
      "o3 b2 b2",
    "t110 @5 v6 q8 me0 mv3,22 k7 o4 g2 g2 | "
      "e2 e2 | "
      "a2 a2 | "
      "d2 d2",
    "@n t110 q3 v13 o1 a16 r8 v10 o6 d+16 r8 v12 o4 d+16 r16 v13 o1 a16 r8 v10 o6 d+16 r8 v12 o4 d+16 r16 | "
      "v13 o1 a16 r8 v10 o6 d+16 r8 v12 o4 d+16 r16 v13 o1 a16 r8 v10 o6 d+16 r8 v12 o4 d+16 r16 | "
      "v13 o1 a16 r8 v10 o6 d+16 r8 v12 o4 d+16 r16 v13 o1 a16 r8 v10 o6 d+16 r8 v12 o4 d+16 r16 | "
      "v13 o1 a16 r8 v10 o6 d+16 r8 v12 o4 d+16 r16 v13 o1 a16 r8 v10 o6 d+16 r8 v12 o4 d+16 r16" } },
  { "SLOW JAM", {
    "t84 @5 v11 q8 me1 mp4,35,250 mg90 o6 c2. o5 a+4 | "
      "c2 d+4 c4 | "
      "o6 d2 o5 a+4 f4 | "
      "f1",
    "t84 @1 v11 q6 me4 o2 c2 c2 | "
      "f2 f2 | "
      "o1 a+2 a+2 | "
      "a+2 a+2",
    "t84 @3 v6 q8 me0 mv3,30 k-7 o4 d+1 | "
      "o3 a1 | "
      "o4 d1 | "
      "d1",
    "t84 @3 v6 q8 me0 mv3,30 o4 g1 | "
      "c1 | "
      "f1 | "
      "f1",
    "t84 @3 v6 q8 me0 mv3,30 k7 o4 a+1 | "
      "d+1 | "
      "a1 | "
      "a1",
    "@n t84 q3 v13 o1 a16 r16 a16 r4 r16 v12 o4 d+16 r4. r16 | "
      "v13 o1 a16 r16 a16 r4 r16 v12 o4 d+16 r4. r16 | "
      "v13 o1 a16 r16 a16 r4 r16 v12 o4 d+16 r4. r16 | "
      "v13 o1 a16 r16 a16 r4 r16 v12 o4 d+16 r4. r16" } },
  { "RAINY DAY", {
    "t92 @4 v10 q7 me2 mp5,22,280 o5 a2 o6 d4 o5 a4 | "
      "b2 o6 e2 | "
      "a2. f+4 | "
      "d4. g4. o5 b4",
    "t92 @1 v11 q6 me4 o2 d2 d4 a4 | "
      "e2 e4 b4 | "
      "f+2 f+4 o3 c+4 | "
      "o2 g2 g4 o3 d4",
    "t92 @3 v6 q8 me0,400 mv3,26 k-7 o4 d1 | "
      "e1 | "
      "f+1 | "
      "g1",
    "t92 @3 v6 q8 me0,400 mv3,26 o4 f+1 | "
      "g1 | "
      "a1 | "
      "b1",
    "t92 @3 v6 q8 me0,400 mv3,26 k7 o4 a1 | "
      "b1 | "
      "o5 c+1 | "
      "d1",
    "@n t92 q3 v10 o6 d+16 r8. d+16 r8. d+16 r8. d+16 r8. | "
      "d+16 r8. d+16 r8. d+16 r8. d+16 r8. | "
      "d+16 r8. d+16 r8. d+16 r8. d+16 r8. | "
      "d+16 r8. d+16 r8. d+16 r8. d+16 r8." } },
  { "LATE NIGHT", {
    "t88 @5 v10 q8 me1 mp3,30,350 o5 e2 b2 | "
      "o6 d2. e4 | "
      "o5 c2 g4 c4 | "
      "o6 e2 c4 o5 g4",
    "t88 @1 v10 q6 me2 o1 a1 | "
      "o2 d1 | "
      "f1 | "
      "c1",
    "t88 @3 v7 q8 me0,500 mv2,30 k-9 o4 c1 | "
      "f1 | "
      "o3 a1 | "
      "o4 e1",
    "t88 @3 v7 q8 me0,500 mv2,30 o4 e1 | "
      "a1 | "
      "c1 | "
      "g1",
    "t88 @3 v7 q8 me0,500 mv2,30 k9 o4 b1 | "
      "o5 e1 | "
      "o4 g1 | "
      "o5 d1",
    "@n t88 q3 v13 o1 a16 r4. r16 v12 o4 d+16 r4. r16 | "
      "v13 o1 a16 r4. r16 v12 o4 d+16 r4. r16 | "
      "v13 o1 a16 r4. r16 v12 o4 d+16 r4. r16 | "
      "v13 o1 a16 r4. r16 v12 o4 d+16 r4. r16" } },
  { "CLOCKWORK", {
    "t140 @7 v11 q4 me8 o5 b8 r8 b8 r8 o6 e8 r8 g+8 r8 | "
      "o5 a8 o6 c+8 e8 a8 o7 c+8 o6 a8 e8 c+8 | "
      "o5 b8 r8 b8 r8 o6 e8 r8 g+8 r8 | "
      "o7 c+8 r8 o6 e4 r8 a8 r4",
    "t140 @1 v10 q6 me4 o2 e8 e8 e8 e8 e8 e8 e8 e8 | "
      "a8 a8 a8 a8 a8 a8 a8 a8 | "
      "e8 e8 e8 e8 e8 e8 e8 e8 | "
      "a8 a8 a8 a8 a8 a8 a8 a8",
    "t140 @0 v5 q8 me0 mv6,30 k-7 o4 e4 e4 e4 e4 | "
      "a4 a4 a4 a4 | "
      "e4 e4 e4 e4 | "
      "a4 a4 a4 a4",
    "t140 @0 v5 q8 me0 mv6,30 o4 g+4 g+4 g+4 g+4 | "
      "o5 c+4 c+4 c+4 c+4 | "
      "o4 g+4 g+4 g+4 g+4 | "
      "o5 c+4 c+4 c+4 c+4",
    "t140 @0 v5 q8 me0 mv6,30 k7 o4 b4 b4 b4 b4 | "
      "o5 e4 e4 e4 e4 | "
      "o4 b4 b4 b4 b4 | "
      "o5 e4 e4 e4 e4",
    "@n t140 q3 v10 o6 d+16 r8. d+16 r8. d+16 r8. d+16 r8. | "
      "d+16 r8. d+16 r8. d+16 r8. d+16 r8. | "
      "d+16 r8. d+16 r8. d+16 r8. d+16 r8. | "
      "v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16" } },
  { "PUZZLE BOX", {
    "t104 @4 v10 q6 me5 mp6,20,200 o5 g4 f4 c2 | "
      "g4. o6 c4. o5 e4 | "
      "o6 c4 o5 a+4 f2 | "
      "f8. a8. o6 c8 a4 f4",
    "t104 @1 v11 q6 me4 o2 c2 c2 | "
      "c2 c2 | "
      "f2 f2 | "
      "f2 f2",
    "t104 @5 v6 q8 me0 mv3,25 k-7 o4 c1 | "
      "c1 | "
      "f1 | "
      "f1",
    "t104 @5 v6 q8 me0 mv3,25 o4 f1 | "
      "e1 | "
      "a+1 | "
      "a1",
    "t104 @5 v6 q8 me0 mv3,25 k7 o4 g1 | "
      "g1 | "
      "o5 c1 | "
      "c1",
    "@n t104 q3 v10 o6 d+16 r8. d+16 r8. d+16 r8. d+16 r8. | "
      "d+16 r8. d+16 r8. d+16 r8. d+16 r8. | "
      "d+16 r8. d+16 r8. d+16 r8. d+16 r8. | "
      "d+16 r8. d+16 r8. d+16 r8. d+16 r8." } },
  { "TOY BOX", {
    "t128 @0 v11 q4 me9 o5 g8 b8 o6 d8 g8 b8 g8 d8 o5 b8 | "
      "d8. f+8. a8 o6 f+4 d4 | "
      "o5 e8 g8 b8 o6 e8 g8 e8 o5 b8 g8 | "
      "o6 g4 e4 c2",
    "t128 @1 v11 q6 me4 o2 g4 o3 g4 o2 g4 o3 g4 | "
      "d4 o4 d4 o3 d4 o4 d4 | "
      "o3 e4 o4 e4 o3 e4 o4 e4 | "
      "o3 c4 o4 c4 o3 c4 o4 c4",
    "t128 @7 v5 q8 me0 mv5,28 k-7 o4 g4 g4 g4 g4 | "
      "d4 d4 d4 d4 | "
      "e4 e4 e4 e4 | "
      "o5 c4 c4 c4 c4",
    "t128 @7 v5 q8 me0 mv5,28 o4 b4 b4 b4 b4 | "
      "f+4 f+4 f+4 f+4 | "
      "g4 g4 g4 g4 | "
      "o5 e4 e4 e4 e4",
    "t128 @7 v5 q8 me0 mv5,28 k7 o5 d4 d4 d4 d4 | "
      "o4 a4 a4 a4 a4 | "
      "b4 b4 b4 b4 | "
      "o5 g4 g4 g4 g4",
    "@n t128 q3 v13 o1 a16 r8. v10 o6 d+16 r8. v12 o4 d+16 r8. v10 o6 d+16 r8. | "
      "v13 o1 a16 r8. v10 o6 d+16 r8. v12 o4 d+16 r8. v10 o6 d+16 r8. | "
      "v13 o1 a16 r8. v10 o6 d+16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 a16 r16 v12 o4 d+16 r16 v10 o6 d+16 r16 | "
      "v13 o1 a16 r8. v10 o6 d+16 r8. v12 o4 d+16 r8. v10 o6 d+16 r8." } },
  { "BUBBLE POP", {
    "t134 @4 v11 q4 me10 mp8,25,60 o5 a8 r8 a8 r8 o6 d8 r8 f+8 r8 | "
      "o5 f+16 a+16 o6 c+16 f+16 a+16 f+16 c+16 o5 a+16 f+16 a+16 o6 c+16 f+16 a+16 f+16 c+16 o5 a+16 | "
      "o6 d8 r8 d8 r8 g8 r8 b8 r8 | "
      "o5 g16 a+16 o6 d16 g16 a+16 o7 d16 o6 a+16 g16 d16 g16 a+16 o7 d16 o6 a+16 g16 d8",
    "t134 @1 v10 q6 me4 o2 d8 d8 d8 d8 d8 d8 d8 d8 | "
      "f+8 f+8 f+8 f+8 f+8 f+8 f+8 f+8 | "
      "g8 g8 g8 g8 g8 g8 g8 g8 | "
      "g8 g8 g8 g8 g8 g8 g8 g8",
    "t134 @3 v6 q8 me0 mv6,30 k-7 o4 d2 d2 | "
      "f+2 f+2 | "
      "g2 g2 | "
      "g2 g2",
    "t134 @3 v6 q8 me0 mv6,30 o4 f+2 f+2 | "
      "a+2 a+2 | "
      "b2 b2 | "
      "a+2 a+2",
    "t134 @3 v6 q8 me0 mv6,30 k7 o4 a2 a2 | "
      "o5 c+2 c+2 | "
      "d2 d2 | "
      "d2 d2",
    "@n t134 q3 v10 o6 d+16 r16 d+16 r16 v11 o5 g+16 r16 v10 o6 d+16 r16 d+16 r16 d+16 r16 v11 o5 g+16 r16 v10 o6 d+16 r16 | "
      "d+16 r16 d+16 r16 v11 o5 g+16 r16 v10 o6 d+16 r16 d+16 r16 d+16 r16 v11 o5 g+16 r16 v10 o6 d+16 r16 | "
      "d+16 r16 d+16 r16 v11 o5 g+16 r16 v10 o6 d+16 r16 d+16 r16 d+16 r16 v11 o5 g+16 r16 v10 o6 d+16 r16 | "
      "d+16 r16 d+16 r16 v11 o5 g+16 r16 v10 o6 d+16 r16 d+16 r16 d+16 r16 v11 o5 g+16 r16 v10 o6 d+16 r16" } },
  { "CANDY LANE", {
    "t142 @5 v11 q5 me6 mp6,28,140 o5 f8. a8. o6 c8 a4 f4 | "
      "o5 c8 e8 g8 o6 c8 e8 c8 o5 g8 e8 | "
      "d16 f16 a16 o6 d16 f16 a16 f16 d16 o5 a16 o6 d16 f16 a16 f16 d16 o5 a8 | "
      "o6 f4 d4 o5 a+2",
    "t142 @1 v11 q6 me4 o2 f4 o3 f4 o2 f4 o3 f4 | "
      "c4 o4 c4 o3 c4 o4 c4 | "
      "o3 d4 o4 d4 o3 d4 o4 d4 | "
      "o2 a+4 o3 a+4 o2 a+4 o3 a+4",
    "t142 @3 v6 q8 me0 mv4,24 k-7 o4 f1 | "
      "c1 | "
      "d1 | "
      "a+1",
    "t142 @3 v6 q8 me0 mv4,24 o4 a1 | "
      "e1 | "
      "f1 | "
      "o5 d1",
    "t142 @3 v6 q8 me0 mv4,24 k7 o5 c1 | "
      "o4 g1 | "
      "a1 | "
      "o5 f1",
    "@n t142 q3 v13 o1 a16 r8. v10 o6 d+16 r8. v12 o4 d+16 r8. v10 o6 d+16 r8. | "
      "v13 o1 a16 r8. v10 o6 d+16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 a16 r16 v12 o4 d+16 r16 v10 o6 d+16 r16 | "
      "v13 o1 a16 r8. v10 o6 d+16 r8. v12 o4 d+16 r8. v10 o6 d+16 r8. | "
      "v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16" } },
  { "MARCH", {
    "t120 @0 v11 q6 me3 o6 g4 g4 d+8 o5 a+8 g4 | "
      "o6 f4 f4 c+8 o5 g+8 f4 | "
      "o6 d+4 c4 o5 g+2 | "
      "d+16 g16 a+16 o6 d+16 g16 a+16 g16 d+16 o5 a+16 o6 d+16 g16 a+16 g16 d+16 o5 a+8",
    "t120 @1 v11 q6 me4 o2 d+4 d+4 d+4 d+4 | "
      "o3 c+4 c+4 c+4 c+4 | "
      "o2 g+4 g+4 g+4 g+4 | "
      "d+4 d+4 d+4 d+4",
    "t120 @3 v6 q8 me0 mv3,20 k-7 o4 d+4 d+4 d+4 d+4 | "
      "c+4 c+4 c+4 c+4 | "
      "g+4 g+4 g+4 g+4 | "
      "d+4 d+4 d+4 d+4",
    "t120 @3 v6 q8 me0 mv3,20 o4 g4 g4 g4 g4 | "
      "f4 f4 f4 f4 | "
      "o5 c4 c4 c4 c4 | "
      "o4 g4 g4 g4 g4",
    "t120 @3 v6 q8 me0 mv3,20 k7 o4 a+4 a+4 a+4 a+4 | "
      "g+4 g+4 g+4 g+4 | "
      "o5 d+4 d+4 d+4 d+4 | "
      "o4 a+4 a+4 a+4 a+4",
    "@n t120 q3 v13 o1 a16 r8. v10 o6 d+16 r8. v12 o4 d+16 r8. v10 o6 d+16 r8. | "
      "v13 o1 a16 r8. v10 o6 d+16 r8. v12 o4 d+16 r8. v10 o6 d+16 r8. | "
      "v13 o1 a16 r8. v10 o6 d+16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 a16 r16 v12 o4 d+16 r16 v10 o6 d+16 r16 | "
      "v13 o1 a16 r8. v10 o6 d+16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 a16 r16 v12 o4 d+16 r16 v10 o6 d+16 r16" } },
  { "CATACOMB", {
    "t82 @6 v9 q8 me0 mp3,55,400 o5 f1 | "
      "o6 c+2. o5 a+4 | "
      "c+1 | "
      "o6 c2 o5 a4 c4",
    "t82 @3 v11 q6 me0 o1 a+1 | "
      "a+1 | "
      "o2 f+1 | "
      "f1",
    "t82 @5 v6 q8 me0,600 mv2,38 k-14 o3 a+1 | "
      "a+1 | "
      "f+1 | "
      "f1",
    "t82 @5 v6 q8 me0,600 mv2,38 o4 c+1 | "
      "c+1 | "
      "o3 a+1 | "
      "a1",
    "t82 @5 v6 q8 me0,600 mv2,38 k14 o4 f1 | "
      "e1 | "
      "c+1 | "
      "c1",
    "@n t82 q3 v13 o1 a16 r4. r16 v12 o4 d+16 r4. r16 | "
      "v13 o1 a16 r4. r16 v12 o4 d+16 r4. r16 | "
      "v13 o1 a16 r4. r16 v12 o4 d+16 r4. r16 | "
      "v13 o1 a16 r4. r16 v12 o4 d+16 r4. r16" } },
  { "OMEN", {
    "t90 @2 v9 q6 me1 mp4,50,300 o6 d+2. o5 b4 | "
      "f+1 | "
      "d+2 g4 d+4 | "
      "d1",
    "t90 @1 v11 q6 me4 o1 b2 b2 | "
      "b2 b2 | "
      "o2 g2 g2 | "
      "g2 g2",
    "t90 @3 v6 q8 me0 mv4,35 k-12 o3 b1 | "
      "b1 | "
      "g1 | "
      "g1",
    "t90 @3 v6 q8 me0 mv4,35 o4 d+1 | "
      "d+1 | "
      "o3 b1 | "
      "b1",
    "t90 @3 v6 q8 me0 mv4,35 k12 o4 g1 | "
      "f+1 | "
      "d+1 | "
      "d1",
    "@n t90 q3 v13 o1 a16 r16 a16 r4 r16 v12 o4 d+16 r4. r16 | "
      "v13 o1 a16 r16 a16 r4 r16 v12 o4 d+16 r4. r16 | "
      "v13 o1 a16 r16 a16 r4 r16 v12 o4 d+16 r4. r16 | "
      "v13 o1 a16 r16 a16 r4 r16 v12 o4 d+16 r4. r16" } },
  { "WHISPER", {
    "t70 @5 v9 q8 me0 mp2,45,500 mg150 o5 g+2 o6 c+2 | "
      "r1 | "
      "e2. c+4 | "
      "r1",
    "t70 @1 v10 q6 me0 o2 c+1 | "
      "d1 | "
      "c+1 | "
      "o3 c1",
    "t70 @3 v6 q8 me0,800 mv2,40 k-16 o4 c+1 | "
      "d1 | "
      "c+1 | "
      "c1",
    "t70 @3 v6 q8 me0,800 mv2,40 o4 e1 | "
      "f+1 | "
      "e1 | "
      "e1",
    "t70 @3 v6 q8 me0,800 mv2,40 k16 o4 g+1 | "
      "a1 | "
      "g+1 | "
      "g1",
    "t70 @3 v5 q8 me0,800 mv2,40 k-16 o6 c+1 | "
      "d1 | "
      "c+1 | "
      "c1" } },
  { "FOG", {
    "t64 @3 v9 q8 me0 ms-40 mp2,30,600 o5 g1 | "
      "a+1 | "
      "r1 | "
      "f2 a+2",
    "t64 @1 v11 q6 me0 o2 c1 | "
      "d+1 | "
      "f1 | "
      "a+1",
    "t64 @5 v6 q8 me0,900 mv2,45 k-18 o4 c1 | "
      "d+1 | "
      "f1 | "
      "o3 a+1",
    "t64 @5 v6 q8 me0,900 mv2,45 o4 f1 | "
      "g+1 | "
      "a+1 | "
      "d+1",
    "t64 @5 v6 q8 me0,900 mv2,45 k18 o4 g1 | "
      "a+1 | "
      "o5 c1 | "
      "o4 f1",
    "t64 @5 v5 q8 me0,900 mv2,45 k-18 o6 c1 | "
      "d+1 | "
      "f1 | "
      "o5 a+1" } },
  { "RITUAL", {
    "t98 @6 v10 q5 me5 mp7,35,120 o6 c8 r8 o5 e4 r8 a8 r4 | "
      "a8 r8 a8 r8 o6 d8 r8 f8 r8 | "
      "o5 g8 r8 o4 b4 r8 o5 e8 r4 | "
      "e4 c4 o4 a2",
    "t98 @1 v11 q6 me4 o1 a8 a8 a8 a8 a8 a8 a8 a8 | "
      "o2 d8 d8 d8 d8 d8 d8 d8 d8 | "
      "e8 e8 e8 e8 e8 e8 e8 e8 | "
      "o1 a8 a8 a8 a8 a8 a8 a8 a8",
    "t98 @2 v6 q8 me0 mv7,35 k-7 o3 a4 a4 a4 a4 | "
      "o4 d4 d4 d4 d4 | "
      "o3 e4 e4 e4 e4 | "
      "a4 a4 a4 a4",
    "t98 @2 v6 q8 me0 mv7,35 o4 c4 c4 c4 c4 | "
      "f4 f4 f4 f4 | "
      "o3 g4 g4 g4 g4 | "
      "o4 c4 c4 c4 c4",
    "t98 @2 v6 q8 me0 mv7,35 k7 o4 e4 e4 e4 e4 | "
      "a4 a4 a4 a4 | "
      "o3 b4 b4 b4 b4 | "
      "o4 e4 e4 e4 e4",
    "@n t98 q3 v13 o1 a16 r8 v10 o6 d+16 r8 v12 o4 d+16 r16 v13 o1 a16 r8 v10 o6 d+16 r8 v12 o4 d+16 r16 | "
      "v13 o1 a16 r8 v10 o6 d+16 r8 v12 o4 d+16 r16 v13 o1 a16 r8 v10 o6 d+16 r8 v12 o4 d+16 r16 | "
      "v13 o1 a16 r8. v10 o6 d+16 r16 v12 o4 d+16 r16 v13 o1 a16 r16 a16 r16 v12 o4 d+16 r16 v10 o6 d+16 r16 | "
      "v13 o1 a16 r8 v10 o6 d+16 r8 v12 o4 d+16 r16 v13 o1 a16 r8 v10 o6 d+16 r8 v12 o4 d+16 r16" } },
  { "VOID", {
    "t60 @2 v9 q8 me0 mp2,60,700 ms-30 o4 g+1 | "
      "r1 | "
      "b1 | "
      "r1",
    "t60 @1 v11 q6 me0 o1 g+1 | "
      "g+1 | "
      "b1 | "
      "o2 c+1",
    "t60 @3 v6 q8 me0,1000 mv1,50 k-20 o3 g+1 | "
      "g+1 | "
      "b1 | "
      "o4 c+1",
    "t60 @3 v6 q8 me0,1000 mv1,50 o4 d+1 | "
      "d+1 | "
      "f+1 | "
      "g+1",
    "t60 @3 v6 q8 me0,1000 mv1,50 k20 o4 g+1 | "
      "g+1 | "
      "b1 | "
      "o5 c+1",
    "t60 @3 v5 q8 me0,1000 mv1,50 k-20 o5 g+1 | "
      "g+1 | "
      "b1 | "
      "o6 c+1" } },
  { "STARFIELD", {
    "t100 @4 v10 q7 me3 mp6,30,180 o6 e4 r4 g+4 r4 | "
      "c+2. o5 b4 | "
      "o6 f+4 r4 a4 r4 | "
      "d+2 o5 b4 f+4",
    "t100 @1 v10 q6 me4 o2 e2 e2 | "
      "o3 c+2 c+2 | "
      "o2 f+2 f+2 | "
      "b2 b2",
    "t100 @5 v6 q8 me0,400 mv3,28 k-11 o4 g+1 | "
      "e1 | "
      "a1 | "
      "d+1",
    "t100 @5 v6 q8 me0,400 mv3,28 o4 b1 | "
      "g+1 | "
      "o5 c+1 | "
      "o4 f+1",
    "t100 @5 v6 q8 me0,400 mv3,28 k11 o5 d+1 | "
      "o4 b1 | "
      "o5 e1 | "
      "o4 a1",
    "@n t100 q3 v10 o6 d+16 r8. d+16 r8. d+16 r8. d+16 r8. | "
      "d+16 r8. d+16 r8. d+16 r8. d+16 r8. | "
      "d+16 r8. d+16 r8. d+16 r8. d+16 r8. | "
      "d+16 r8. d+16 r8. d+16 r8. d+16 r8." } },
  { "HYPERJUMP", {
    "t172 @1 v11 q5 me5 mp7,25,100 o5 c16 d+16 g16 o6 c16 d+16 g16 d+16 c16 o5 g16 o6 c16 d+16 g16 d+16 c16 o5 g8 | "
      "o4 a+16 o5 d16 f16 a+16 o6 d16 o5 a+16 f16 d16 o4 a+16 o5 d16 f16 a+16 o6 d16 o5 a+16 f16 d16 | "
      "o4 g+16 o5 c16 d+16 g+16 o6 c16 d+16 c16 o5 g+16 d+16 g+16 o6 c16 d+16 c16 o5 g+16 d+8 | "
      "b8 r8 d4 r8 g8 r4",
    "t172 @1 v11 q6 me4 o2 c16 g16 o3 c16 o2 g16 c16 g16 o3 c16 o2 g16 c16 g16 o3 c16 o2 g16 c16 g16 o3 c16 o2 g16 | "
      "a+16 o3 f16 a+16 f16 o2 a+16 o3 f16 a+16 f16 o2 a+16 o3 f16 a+16 f16 o2 a+16 o3 f16 a+16 f16 | "
      "o2 g+16 o3 d+16 g+16 d+16 o2 g+16 o3 d+16 g+16 d+16 o2 g+16 o3 d+16 g+16 d+16 o2 g+16 o3 d+16 g+16 d+16 | "
      "o2 g16 o3 d16 g16 d16 o2 g16 o3 d16 g16 d16 o2 g16 o3 d16 g16 d16 o2 g16 o3 d16 g16 d16",
    "t172 @7 v6 q8 me0 mv7,32 k-7 o4 c8 c8 c8 c8 c8 c8 c8 c8 | "
      "o3 a+8 a+8 a+8 a+8 a+8 a+8 a+8 a+8 | "
      "g+8 g+8 g+8 g+8 g+8 g+8 g+8 g+8 | "
      "g8 g8 g8 g8 g8 g8 g8 g8",
    "t172 @7 v6 q8 me0 mv7,32 o4 d+8 d+8 d+8 d+8 d+8 d+8 d+8 d+8 | "
      "d8 d8 d8 d8 d8 d8 d8 d8 | "
      "c8 c8 c8 c8 c8 c8 c8 c8 | "
      "o3 b8 b8 b8 b8 b8 b8 b8 b8",
    "t172 @7 v6 q8 me0 mv7,32 k7 o4 g8 g8 g8 g8 g8 g8 g8 g8 | "
      "f8 f8 f8 f8 f8 f8 f8 f8 | "
      "d+8 d+8 d+8 d+8 d+8 d+8 d+8 d+8 | "
      "d8 d8 d8 d8 d8 d8 d8 d8",
    "@n t172 q3 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 | "
      "v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 | "
      "v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 | "
      "v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16 v13 o1 a16 r16 v10 o6 d+16 d+16 v12 o4 d+16 r16 v10 o6 d+16 d+16" } },
  { "VIBRATO", {
    "t110 l2 v11 q8 me1 mp6,45,200 o5 [ e g | "
      "a g ]2",
    "t110 l1 v10 q8 me0 o3 [ c | "
      "f ]2",
    "t110 l1 v6  q8 me0 k7   o4 [ g | "
      "c ]2",
    "t110 l1 v6  q8 me0 k-7  o4 [ e | "
      "a ]2",
    "t110 l1 v5  q8 me0 mv3,30 o5 [ c | "
      "f ]2",
    "t110 l1 v5  q8 me0,400 mv2,30 k-14 o5 [ g | "
      "a ]2" } },
  { "PLUCK", {
    "t150 l16 v12 q8 me13 o5 [ c e g > c < g e ]8",
    "t150 l4  v11 q6 me2  o3 [ c c g g ]3",
    "t150 l2  v6  q8 me0  o4 [ e g ]3",
    "t150 l2  v6  q8 me0 k-8 o4 [ g b ]3",
    "t150 l1  v5  q8 me0,300 mv4,25 o5 [ c ]3",
    "@n t150 l8 v9 o5 [ c r c c ]6" } },
  { "SLIDE", {
    "t120 l4 v11 q8 me0 mg120 o4 [ c g > c < g ]4",
    "t120 l2 v10 q7 me3 o2 [ c f ]4",
    "t120 l8 v7  q3 me8 ms-900 o5 [ c r c r c r c r ]4",
    "t120 l1 v6  q8 me0 k-6 o4 [ e ]4",
    "t120 l1 v6  q8 me0 k6 mv3,28 o4 [ g ]4",
    "@n t120 l4 v8 o4 [ c r c r ]4" } },
  { "THICK", {
    "t100 l2 v10 q8 me0 k-6 mv5,35 o5 [ a > c < | "
      "b a ]2",
    "t100 l2 v10 q8 me0 k6         o5 [ a > c < | "
      "b a ]2",
    "t100 l1 v11 q8 me1 o2 [ a | "
      "e ]2",
    "t100 l1 v6  q8 me0 k-12 o4 [ c | "
      "b ]2",
    "t100 l1 v6  q8 me0 k12  o4 [ e | "
      "g ]2",
    "@n t100 l2 v7 o3 [ c r ]4" } },
};
static const int BGM_COUNT = (int)( sizeof(BGM_DEFS) / sizeof(BGM_DEFS[0]) );

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
static const int ROW_HELP  = 9;

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
// lavender everything you are not on.
static const int PAL_HEAD  = 1;       // WHITE -> YELLOW
static const int PAL_SEL   = 2;       // WHITE -> LIGHT_PEACH
static const int PAL_DIM   = 3;       // WHITE -> LAVENDER

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
// far side of 48 entries.
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
    // has to cover the "ON " that was there, and " 9/48" the "10/48".
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
    cursor( 1, ROW_HELP,     (BgPal)PAL_DIM ); print( "UP/DN  BGM/SE" );
    cursor( 1, ROW_HELP + 1, (BgPal)PAL_DIM ); print( "L/R    SELECT" );
    cursor( 1, ROW_HELP + 2, (BgPal)PAL_DIM ); print( "Z SE   X BGM" );
  }

  void _draw() override {
    cls( Color::DARK_BLUE );

    if( first_draw ){
      // Palettes are PPU state and stick once set, so this runs on frame 0 only.
      pal( Color::WHITE, Color::YELLOW,      PAL_HEAD );
      pal( Color::WHITE, Color::LIGHT_PEACH, PAL_SEL  );
      pal( Color::WHITE, Color::LAVENDER,    PAL_DIM  );
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
