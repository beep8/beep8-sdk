
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
9. **Different `map()`** — configure with `mapsetup()`, then `map(upix, vpix)` draws the whole
   layer at a scroll offset (ignores `camera()`); it is not PICO-8's
   `map(cel_x, cel_y, sx, sy, ...)`.

### Not currently supported
`sspr` (scaled sprite blit), `flip()`, `palt()` (per-color transparency), `pget()`, and
`sset()` are not implemented.
