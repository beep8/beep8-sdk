
# PICO-8-like API for BEEP-8 (C/C++)

This document is a reference for the PICO-8-like API implemented for the BEEP-8 system in
C/C++ (`pico8.h` / `pico8.cpp`). It helps developers familiar with PICO-8 (a Lua-based
fantasy console) transition to creating BEEP-8 applications in C/C++.

Everything below lives in the `pico8` namespace. Signatures are given exactly as declared in
`pico8.h`; the doc-comments in that header remain the authoritative, per-function reference.
Coordinates typed `fx8` are 8.8 fixed-point but accept plain integer/float literals.

Unless noted otherwise, drawing functions:
- honor the current draw color set by `color()` when their color argument is `CURRENT`,
- honor the current depth set by `setz()`,
- are offset by `camera()` (the background layer drawn by `map()` is the exception — it
  ignores `camera()`),
- treat the end coordinate of a span (`x1`/`y1`) as **exclusive**, unlike PICO-8.

---

## Application skeleton

Subclass `pico8::Pico8` and override the reserved callbacks, then call `run()`:

```cpp
#include "pico8.h"
using namespace pico8;

class MyApp : public Pico8 {
    void _init()   override {}          // called once at startup
    void _update() override {}          // called every frame (game logic)
    void _draw()   override {           // called every frame after _update()
        cls(BLACK);
        spr(1, 10, 10);
        rectfill(20, 20, 40, 40, RED);
    }
};

int main() {
    MyApp app;
    app.run();
    return 0;
}
```

---

> **Drawing calls are only legal inside `_draw()`.** `cls`, `pset`, `rect`, `rectfill`, `line`,
> `circ`, `circfill`, `poly`, `spr`, `sprb`, `map`, `pal` and `setpal` all emit PPU commands into
> the frame's command buffer, and each one asserts `NOT_DURING_DRAWING` if called from `_init()`
> or `_update()`. That compiles cleanly and then dies on the first frame with
> `PICO-8 API ERROR : 1`, so it is a runtime-only trap. One-time setup that happens to be a
> drawing call — `pal()`/`setpal()` are the usual cases — goes inside `_draw()` behind a
> first-frame flag; the palette itself persists once written. `lsp()` and `mapsetup()` are the
> setup calls that do belong in `_init()`.
>
> `cursor()`, `print()`, `scursor()` and `sprint()` are *not* subject to this — they write to a
> text stream rather than the PPU command buffer.

## Enums

### `Color`
16-color palette identical to PICO-8's default, plus `CURRENT`.

| # | name | # | name | # | name | # | name |
|---|------|---|------|---|------|---|------|
| 0 | `BLACK` | 4 | `BROWN` | 8 | `RED` | 12 | `BLUE` |
| 1 | `DARK_BLUE` | 5 | `DARK_GREY` | 9 | `ORANGE` | 13 | `LAVENDER` |
| 2 | `DARK_PURPLE` | 6 | `LIGHT_GREY` | 10 | `YELLOW` | 14 | `PINK` |
| 3 | `DARK_GREEN` | 7 | `WHITE` | 11 | `GREEN` | 15 | `LIGHT_PEACH` |

`CURRENT` (16) is a sentinel meaning "use the current draw color".

### `Button`
`BUTTON_LEFT` (0), `BUTTON_RIGHT` (1), `BUTTON_UP` (2), `BUTTON_DOWN` (3), `BUTTON_O` (4,
`z` key), `BUTTON_X` (5, `x` key), `BUTTON_MOUSE_LEFT` (6), `BUTTON_ANY` (0x10).

### `BgPal` / `BgIndex`
`BgPal`: `BG_PAL_0`..`BG_PAL_3`, `BG_PAL_CURRENT`. `BgIndex`: `BG_0`..`BG_3`, `BG_MAX`.

### `MouseBtn`
`LEFT` (1<<0), `RIGHT` (1<<1, reserved).

### `Error`
`NO_ERROR`, `NOT_DURING_DRAWING`, `INVALID_PARAM`, `NOT_INITIALIZED`, `EMPTY_SPAN`. Retrieve
via `seterr()`/error handling internally; most functions no-op on invalid parameters.

---

## Drawing state

```cpp
Color color(Color color);                       // set draw color, returns previous
int   setz(int otz);                            // set depth [0, maxz()], returns previous
int   getz();                                   // current depth
int   maxz();                                   // maximum depth
const Vec&  camera(fx8 x = 0, fx8 y = 0);       // set camera offset, returns previous
const Rect& clip(fx8 x, fx8 y, fx8 w, fx8 h);   // set clip rect, returns previous
const Rect& clip(const Rect& rc);               // set clip rect from Rect
const Rect& clip();                             // reset clip to full screen
```

Higher `setz()` values are drawn **behind** lower ones. `color()` affects graphics only —
text color is managed separately (see `sprint()` / `cursor()`).

---

## Primitive drawing

```cpp
void cls(Color color = BLACK);                                  // clear whole screen (ignores clip/setz)
void pset(fx8 x, fx8 y, Color color = CURRENT);                 // single pixel
void line(fx8 x0, fx8 y0, fx8 x1, fx8 y1, Color color = CURRENT);
void line(const Line& ln, Color color = CURRENT);
void rect(fx8 x0, fx8 y0, fx8 x1, fx8 y1, Color color = CURRENT);      // outline
void rectfill(fx8 x0, fx8 y0, fx8 x1, fx8 y1, Color color = CURRENT);  // filled
void circ(fx8 x, fx8 y, fx8 r = 4, Color col = CURRENT);              // outline
void circfill(fx8 x, fx8 y, fx8 r = 4, Color col = CURRENT);          // filled
void poly(const Poly& pol, Color color = CURRENT);                          // filled triangle
void poly(fx8 x0, fx8 y0, fx8 x1, fx8 y1, fx8 x2, fx8 y2, Color color = CURRENT);
```

`rectfill`/`rect`/`line` end coordinates are exclusive. `rect(x0,y0,x1,y1)` outlines exactly
the box `rectfill(x0,y0,x1,y1)` fills — the outline is drawn inside it, on columns `x0`/`x1-1`
and rows `y0`/`y1-1` — so the two can be layered without a 1px mismatch. `poly` (filled
triangle) is a BEEP-8 extension not present in PICO-8.

---

## Sprites & VRAM banks

BEEP-8 VRAM is 4-bit color, 512×512 px, addressed as a 4×4 grid of 16 banks of 128×128 px
(16×16 sprites of 8×8). Banks 14 and 15 are reserved for system use (font data).

```cpp
void spr(int n, fx8 x = 0, fx8 y = 0, u8 w = 1, u8 h = 1,
         bool flip_x = false, bool flip_y = false, u8 selpal = 0);       // bank 0 only, n in [0,255]
void sprb(u8 bank, int n, fx8 x = 0, fx8 y = 0, u8 w = 1, u8 h = 1,
          bool flip_x = false, bool flip_y = false, u8 selpal = 0);      // any bank 0-15
void lsp(u8 bank, const uint8_t* srcimg);       // load 8192-byte (4bpp 128x128) sheet into bank
Color sget(u8 x, u8 y, u8 bank = 0);            // read sprite-sheet pixel color (bank 0-13)
```

`selpal` selects one of the palettes configured with `setpal()`/`pal()`. `spr()` is limited
to bank 0; use `sprb()` for other banks. `lsp()` is blocking; the bank must be free.

---

## Palettes

```cpp
void pal(Color c0, Color c1, u8 palsel = 0);                    // remap c0 -> c1 in palette palsel (0-15)
void setpal(int palsel, const std::array<unsigned char, 16>& pidx);  // set all 16 entries of a palette
```

Unlike PICO-8 (whose 3rd `pal()` argument chooses draw vs. screen palette), BEEP-8's 3rd
argument `palsel` selects **which of up to 16 palettes** to modify.

---

## Text — sprite layer (`sprint`)

Rendered as sprites (per-pixel positioning, depth-sortable). Because sprites are part of the
framebuffer, these must be re-issued **every frame**; `cls()` erases them.

```cpp
struct SprCursor { int x, y; Color color; int z; void Reset(); };
const SprCursor& scursor(int x = 0, int y = 0, Color color = CURRENT, int z = 0);
void sprint(std::string_view format, ...);                     // uses current scursor
void sprint(int x, int y, Color color, std::string_view format, ...);
```

Supports ANSI-style escape sequences for BEEP-8 color codes, e.g.
`sprint("\e[50mRed\e[0m\n")` (foreground 50), `sprint("\e[57;72mWhite on purple\n")`.

**`scursor()` takes pixels, and the font is 8x8.** A glyph advances 8 px horizontally and a
line of text is 8 px tall, so consecutive lines must be **8 or more pixels apart** —
`scursor(x, 8)`, `scursor(x, 16)`, `scursor(x, 24)` … Passing row numbers (`scursor(x, 1)`,
`scursor(x, 2)`) stacks every line on top of the previous one.

The unit is pixels because this layer draws text as *sprites*, which are not bound to the tile
grid. The background layer below writes into the 8x8 tilemap, so its `cursor()` is in tile
units — one unit there is eight here: `cursor(2, 3)` puts text where `scursor(16, 24)` would.

`sprint()` neither wraps nor clips. At the default 128 px width a line holds at most 16
characters; anything past the right edge is drawn off-screen and silently lost, so the string
just looks truncated. Check `x + strlen(text) * 8 <= resw()`, counting what `%d`/`%s` expand
to rather than the length of the format string.

## Text — background layer (`print`)

Rendered as 8×8 background tiles using palette colors. Persists across frames without
re-issuing (closer to PICO-8's `print()`).

```cpp
struct BgCursor { int x, y; BgPal pal; void Reset(); };
const BgCursor& cursor(int x = 0, int y = 0, BgPal pal = BG_PAL_CURRENT);  // x,y in TILE units
void print(std::string_view format, ...);
void print(int x, int y, BgPal pal, std::string_view format, ...);
```

Escape sequences: `\e[row;colH` (move), `\e[Nq` (select palette N), `\e[2J` (clear).

---

## Background maps

BEEP-8's map API differs from PICO-8: you configure a background layer, then draw the whole
layer at a pixel scroll offset (it always fills the screen and ignores `camera()`).

```cpp
void mapsetup(BgTiles wtile, BgTiles htile,
              std::optional<BgTilesPtr> tiles = std::nullopt,
              u8 uwrap = B8_PPU_BG_WRAP_CLAMP, u8 vwrap = B8_PPU_BG_WRAP_CLAMP,
              BgIndex index = BG_0);
void map(s16 upix, s16 vpix, BgIndex index = BG_0);       // draw layer at pixel offset
void mapdraw(s16 upix, s16 vpix, BgIndex index = BG_0);   // alias of map()
```

`BgTiles` is a power-of-two enum: `TILES_8`, `TILES_16`, … `TILES_32768`. Call `mapsetup()`
before `map()`.

Tile access:

```cpp
b8PpuBgTile mgett(u32 x, u32 y, BgIndex index = BG_0);    // get full tile struct
u16 mget(u32 x, u32 y, BgIndex index = BG_0);             // get tile id (YTILE*16 + XTILE)
void mset(u32 x, u32 y, u8 v, u8 bank = 0, BgIndex index = BG_0, uint8_t pal = 0);
void msett(u32 x, u32 y, b8PpuBgTile tile, BgIndex index = BG_0);   // set full tile struct
void mcls(b8PpuBgTile tile = b8PpuBgTile{0,0,0,0,0}, BgIndex index = BG_0);  // fill map
```

---

## Sprite flags

```cpp
void fset(u8 sprite_index, u8 flag_index = 0xff, u8 value = 0, u8 sprite_pattern_bank = 0);
u8   fget(u8 sprite_index, u8 flag_index = 0xff, u8 sprite_pattern_bank = 0);
```

8 flags per sprite (0-7). `flag_index == 0xff` reads/writes all flags as a bit field.
`sprite_pattern_bank` is a BEEP-8 extension (PICO-8 has a single bank).

---

## Input

```cpp
u32  btn(Button button = BUTTON_ANY, u8 player = 0);   // held; no arg -> bitmask of all buttons
bool btnp(Button button, u8 player = 0);               // just-pressed (auto-repeat after 15f, every 4f)
u32  btnr(Button button, u8 player = 0);               // frames since released (BEEP-8 extension)
```

Only player 0 is supported. `btn()` with no argument returns a bitmask (bit N = `BUTTON_N`).

Mouse / touch:

```cpp
fx8 mousex();          // sub-pixel X
fx8 mousey();          // sub-pixel Y
u32 mousestatus();     // bitmask, MouseBtn::LEFT etc.
s32 stat(int index);   // PICO-8 compat: 32=mouse X (int), 33=mouse Y (int), 34=left button
```

Prefer `mousex()`/`mousey()`/`mousestatus()`; `stat()` exists only for PICO-8 compatibility.

---

## Math & random

Trig uses **radians** (2π per turn), not PICO-8's [0,1] range. Prefer the `pico8::`-qualified
name to avoid colliding with libc.

```cpp
fx8 cos(fx8 rad);      fx8 sin(fx8 rad);      fx8 atan2(fx8 y, fx8 x);  // note (y, x) order
fx8 abs(fx8 x);        fx8 flr(fx8 x);        fx8 cel(fx8 x);           // cel = ceil
fx8 max(fx8 x, fx8 y); fx8 min(fx8 x, fx8 y); fx8 mid(fx8 a, fx8 b, fx8 c);
fx8 clamp(fx8 lo, fx8 v, fx8 hi);   // alias of mid()
fx8 sgn(fx8 x);        // sgn(0) == 1, per PICO-8
fx8 sqrt(fx8 x);       // sqrt(negative) == 0, per PICO-8
```

Random:

```cpp
fx8 rnd(fx8 x = 1.0);          // [0, x), fractional
fx8 rndi(fx8 x);               // [0, floor(x)), integer value
fx8 rndf(fx8 x0, fx8 x1);      // [x0, x1], fractional
u32 rndu();                    // full 32-bit
void srand(u32 seed);          // seed the generator
template<class T> const T& rndt(std::span<const T>);   // random element of a container/array
```

Screen size:

```cpp
fx8 resw();    // screen width in pixels
fx8 resh();    // screen height in pixels
```

---

## Sound — `<sound.h>`

Sound is not part of `pico8.h`; it lives in its own header and needs no assets.

```cpp
#include <sound.h>
```

Nothing has to be initialised and nothing has to be ticked — `Pico8::run()` advances the
sequencer once per frame, and the APU is only touched once a game actually asks for a sound.

`<sound.h>` owns every APU channel. Do not call `b8Apu*` or write `B8_APU_*` registers while
using it; the low-level driver in `<b8/apu.h>` is for tools that want raw register control
(the sound editor), not for games.

### Sound effects

| Function | Notes |
|---|---|
| `sndSfx(SndSfx id)` | Fire-and-forget one-shot. Safe to call while music plays; the effects side owns two tone voices and one noise voice, and the oldest effect is dropped when they are all busy. |

Presets, grouped by the kind of game event:

| Group | Presets |
|---|---|
| Player action | `SFX_JUMP`, `SFX_DOUBLEJUMP`, `SFX_LAND`, `SFX_STEP`, `SFX_SWIPE`, `SFX_DASH`, `SFX_SHOOT`, `SFX_CHARGE` |
| Weapons | `SFX_LASER`, `SFX_MISSILE`, `SFX_ZAP`, `SFX_RELOAD` |
| Impact / destruction | `SFX_HIT`, `SFX_BOUNCE`, `SFX_BREAK`, `SFX_BLOCK`, `SFX_CRUSH`, `SFX_DAMAGE` |
| Explosions | `SFX_EXPLODE` (mid), `SFX_BOOM` (deep and long), `SFX_BLAST` (a sharp crack falling into a low tail), `SFX_RUMBLE` (a distant roar) |
| Pickups / rewards | `SFX_COIN`, `SFX_GEM`, `SFX_HEAL`, `SFX_POWERUP`, `SFX_LEVELUP`, `SFX_UNLOCK`, `SFX_EXTRALIFE` |
| Menus / UI | `SFX_SELECT`, `SFX_CONFIRM`, `SFX_CANCEL`, `SFX_DENY`, `SFX_BLIP`, `SFX_TEXT`, `SFX_PAUSE` |
| Game state | `SFX_ALARM`, `SFX_COUNTDOWN`, `SFX_START`, `SFX_CLEAR`, `SFX_VICTORY`, `SFX_GAMEOVER` |
| Environment | `SFX_SPLASH`, `SFX_BUBBLE`, `SFX_WIND`, `SFX_FIRE`, `SFX_DOOR`, `SFX_ENGINE`, `SFX_MAGIC`, `SFX_WARP` |

Those 50 names are the complete list. Several of them are two voices at once (a
noise body under a pitched thump, a fanfare doubled an octave up), which is why
there is no way to layer two of them yourself: pick the one whose name matches
the event and let it be one sound.

### Music (MML)

| Function | Notes |
|---|---|
| `sndBgmPlay(t0, …, t5)` | Up to 6 tracks, loops forever. `t1`–`t5` are optional. |
| `sndBgmPlayOnce(t0, …)` | Same, but stops at the end — for jingles. |
| `sndBgmStop()` / `sndBgmIsPlaying()` | Stop / query. |
| `sndBgmVolume(pct)` / `sndSfxVolume(pct)` | Master trims, 0–100. |
| `sndStopAll()` | Music and effects. |
| `sndSetWave(slot, "…")` | Define waveform `slot` (0–15) from 32 hex digits. |
| `sndSetWaveData(slot, u8[32])` | Same, from an array of samples 0–15. |
| `sndResetWaves()` | Restore all 16 slots to the factory table. |

Tracks are parsed lazily straight off the caller's string, so the strings must outlive
playback — string literals or statics, never a local buffer.

| MML | Meaning |
|---|---|
| `t120` | Tempo BPM (20–300, global). |
| `o4` / `>` / `<` | Octave 1–7 (`o4 c` = middle C) / up / down. |
| `l8` | Default note length: 1, 2, 4, 8, 16, 32. |
| `v10` | Volume 0–15. |
| `q6` | Gate 1–8 — the note sounds for `q/8` of its length. 8 = legato. |
| `@0` | Waveform 0–15 (0 pulse, 1 saw, 2–7 shaped, 8–15 your own). |
| `@w8={…}` | Define waveform 8 here, as 32 hex digits — see below. |
| `@n` | Switch the track to the noise generator, for drums. Low octaves read as kicks, high ones as hats. Only one track at a time — there is a single noise generator, and the track's own tone channel goes silent. |
| `cdefgab` | A note; optional `+`/`#`/`-`, then a length (`c16`), then dots (`c4.`). |
| `r` / `^` | Rest / tie. |
| `[ … ]4` | Repeat 4 times (nests 4 deep). |
| `n60` | A note by absolute key number 0–95 (48 = `o4 a` = A4 440 Hz). A length needs a comma: `n60,16`. |

Whitespace and `|` are ignored.

Per-track voicing. These stay set until changed, so they normally sit at the head of a
track:

| MML | Meaning |
|---|---|
| `mp6,40` | Vibrato: 6 Hz, ±40 cents. Full form `mp<hz>,<cents>,<delay_ms>,<shape>`; `mp0` off. |
| `mv5,50` | Tremolo: 5 Hz, dipping 50 % at the bottom of each cycle. Same arguments; `mv0` off. |
| `me4` | Volume envelope decay 0–15. `me0` holds the level for the whole note (organ, pad); `me12` is a hard pluck. Default 6. |
| `me0,300` | … with a 300 ms fade-in: `me<decay>,<attack_ms>`. |
| `ms-600` | Pitch sweep in cents per second from the start of every note. Negative falls; `ms0` off. |
| `mg80` | Portamento: each note slides in from the previous one over 80 ms. `mg0` off. |
| `k-8` | Detune the track by ±cents. Two tracks a few cents apart read as one thick voice. |

LFO shapes are `0` sine (default), `1` triangle, `2` square — a trill, not a waver — and
`3` a falling ramp. Both LFOs are sampled on the sequencer's 120 Hz tick, so 1–10 Hz is the
smooth range and 20 Hz is the ceiling. On an `@n` noise track only `mp` does anything: that
generator's volume moves in 6 dB steps, too coarse for a tremolo or an envelope.

### Your own waveforms

`@0`–`@15` pick one of the chip's 16 waveform slots. Slots 0–7 hold the factory tones;
slots **8–15 start silent and are yours**. A waveform is 32 samples of 4 bits, so it is
written as **32 hex digits**: `0` the bottom of the wave, `f` the top, `8` the middle
(silence). Whitespace and `|` between digits are ignored, and fewer than 32 digits are
padded with `8`.

Define one from the music itself, or from code — same digits either way:

```cpp
sndBgmPlay("@w8={89abcdeffedcba98 7654321001234567} @8 t120 o4 l8 cdefg");

sndSetWave( 8, "89abcdeffedcba98 7654321001234567" );   // a triangle
sndSetWave( 9, "ffffffffffffffff 0000000000000000" );   // a hard square
sndBgmPlay ( "@8 t120 o4 l8 cdefg", "@9 t120 o3 l4 c g" );
```

Slots are global, not per track: `@w8` in one track redefines slot 8 for every track using
it, which is what makes it a shared instrument. Slots 0–7 can be overwritten too, but
`sndSfx` builds its tone presets out of them — change those and the effects change with
them.

Six tracks is a melody, a counter-melody, a three-note chord and a bass at once — or a
lead doubled by a second track a few cents away (`k`) over a smaller arrangement. Sound
effects get the remaining two tone voices plus the effects noise generator.

```cpp
void _init() override {
  sndBgmPlay("t130 o5 l8 v10 q6 mp6,35,150 [ c e g > c < ]2",  // melody, with vibrato
             "t130 o3 l4 v11 @1 me2 [ c c g g ]2",             // bass, long decay
             "t130 o4 l2 v6  q8 @2 me0 [ e g ]2",              // pad, held flat
             "t130 o4 l2 v6  q8 @2 me0 k8 [ e g ]2",           // ... detuned twin
             0,                                                // (unused)
             "@n t130 o5 l8 v9 [ c r > c < c ]4");             // drums
}
void _update() override { if( btnp(BUTTON_O) ) sndSfx(SFX_JUMP); }
```

> Browsers only start audio after the first tap/click on the page, so the opening moment of a
> game is silent no matter what this API does. A "TAP TO START" hint is the usual fix.

## Key differences from PICO-8

1. **C/C++ instead of Lua** — subclass `Pico8`, override `_init`/`_update`/`_draw`, call `run()`.
2. **Multiple palettes** — up to 16, selected via `palsel`/`selpal`; `pal()`'s 3rd argument
   picks the palette, not draw-vs-screen.
3. **Multiple VRAM banks** — 16 banks of 128×128; `sprb()`/`lsp()` reach banks beyond bank 0.
4. **Depth sorting** — `setz()`/`getz()`/`maxz()` give every draw call an explicit z; there is
   no `flip()`.
5. **Exclusive end coordinates** — `rect`/`rectfill`/`line` do not draw the `x1`/`y1` edge
   (PICO-8 is inclusive on both). `rect` and `rectfill` cover the same box for the same args.
6. **Two text layers** — `sprint()` (sprite layer, re-issue each frame) vs. `print()`
   (background tiles, persistent). Text color is independent of `color()`.
7. **Radians** — `sin`/`cos`/`atan2` use radians; `atan2` takes `(y, x)`.
8. **Extensions** — `poly()` (filled triangle), `btnr()`, `rndi/rndf/rndu/rndt`, sub-pixel
   mouse (`mousex`/`mousey`).
9. **No `sfx()` / `music()`** — those are PICO-8-only. BEEP-8's sound is `<sound.h>`
   (preset effects + MML music); see the Sound section above.
10. **Different `map()`** — configure with `mapsetup()`, then `map(upix, vpix)` draws the whole
   layer at a scroll offset (ignores `camera()`); it is not PICO-8's
   `map(cel_x, cel_y, sx, sy, ...)`.

### Not currently supported
`sspr` (scaled sprite blit), `flip()`, `palt()` (per-color transparency), `pget()`, and
`sset()` are not implemented.
