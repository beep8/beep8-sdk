// PixArt — BEEP-8 pixel art editor (standalone .b8 app)
//
// 128x128 canvas locked to the PICO-8 16-color palette. Two view modes:
//   - PIXEL:    a 16x16 window zoomed 8x (each logical pixel = an 8x8 dot),
//               panned over the full canvas; this is where you draw.
//   - OVERVIEW: the whole 128x128 canvas at 1:1, view-only. A tap moves the
//               viewport box (snapped to 16px tiles). Return with the View btn.
// Toolbar (two rows): row A = Pen / Hand / Eyedropper / Undo / Redo;
//                     row B = View / Grid / Copy / Paste.
// Undo/Redo keep up to UNDO_MAX steps. Copy/Paste act on the current 16x16
// viewport tile. Input is touch/mouse (b8HifGetMouseStatus).
//
// NOTE: pico8 rectfill()/rect() take x1/y1 as EXCLUSIVE (right/bottom edge not
// drawn), so fills use +size and a 1px-wide line is (a, a+1).
//
// Not yet wired (later increments): Mirror L/R + U/D, PNG save/load (ROM-side
// C codec), and Save-with-optional-share to the CC0 asset commons.
#include <pico8.h>
#include <beep8.h>
#include <string.h>

using namespace pico8;

// ---- layout (screen is 128x240 portrait) -----------------------------------
static constexpr int SCRW    = 128;
static constexpr int SCRH    = 240;
static constexpr int CANVAS  = 128;   // canvas is CANVAS x CANVAS logical pixels
static constexpr int VIEW    = 16;    // PIXEL mode shows VIEW x VIEW pixels
static constexpr int DOT     = 8;     // each logical pixel = DOT x DOT on screen
static constexpr int EDIT_H  = VIEW * DOT;    // 128: edit area height (top square)
static constexpr int VMAX    = CANVAS - VIEW;  // 112: max viewport origin
static constexpr int GRID_PX = 16;    // grid spacing on screen (both modes)

static constexpr int PAL_SW   = 16;           // swatch size (16px: finger-friendly on phones)
static constexpr int PAL_COLS = 8;            // swatches per row (2 rows x 8 = 16 colors)
static constexpr int PAL_Y    = EDIT_H + 2;   // palette top
static constexpr int PAL_H    = 2 * PAL_SW;   // 32: palette is 2 rows tall
static constexpr int BARA_Y   = PAL_Y + PAL_H + 2;  // toolbar row A (below palette)
static constexpr int BARB_Y   = BARA_Y + 20;  // toolbar row B
static constexpr int ICON    = 16;            // icon / button size
static constexpr int PITCH   = 21;            // toolbar button pitch
static constexpr int STAT_Y  = BARB_Y + 20;

static constexpr int UNDO_MAX = 4;            // undo / redo depth
static constexpr int SZ = CANVAS * CANVAS;    // bytes per canvas snapshot

// toolbar button ids. Row A = [0..4], row B = [5..8].
enum Btn { B_PEN = 0, B_HAND, B_EYE, B_UNDO, B_REDO, B_VIEW, B_GRID, B_COPY, B_PASTE, B_N };
static constexpr int ROWA = 5;                // first 5 ids are on row A
static constexpr int ROWB = B_N - ROWA;       // remaining ids on row B

static inline int clampi(int v, int lo, int hi){
  return v < lo ? lo : (v > hi ? hi : v);
}

// 16x16 1-bit icons (bit 15 = leftmost pixel), indexed by Btn.
static const uint16_t kIcon[B_N][16] = {
  { // B_PEN : a pencil (eraser cap, gap, body, tapering to a sharp tip)
    0b0000011111100000,0b0000011111100000,0b0000000000000000,0b0000011111100000,
    0b0000011111100000,0b0000011111100000,0b0000011111100000,0b0000011111100000,
    0b0000011111100000,0b0000001111000000,0b0000001111000000,0b0000000110000000,
    0b0000000110000000,0b0000000100000000,0b0000000000000000,0b0000000000000000 },
  { // B_HAND : move cross with arrowheads
    0b0000000110000000,0b0000001111000000,0b0000011111100000,0b0000001111000000,
    0b0010000110000100,0b0110000110000110,0b1111111111111111,0b1111111111111111,
    0b0110000110000110,0b0010000110000100,0b0000001111000000,0b0000011111100000,
    0b0000001111000000,0b0000000110000000,0b0000000000000000,0b0000000000000000 },
  { // B_EYE : eyedropper (round rubber bulb, thin tube, pointed tip)
    0b0000001111000000,0b0000011111100000,0b0000011111100000,0b0000011111100000,
    0b0000001111000000,0b0000000110000000,0b0000000110000000,0b0000000110000000,
    0b0000000110000000,0b0000000110000000,0b0000000110000000,0b0000000110000000,
    0b0000000110000000,0b0000000110000000,0b0000000100000000,0b0000000000000000 },
  { // B_UNDO : left arrow (shaft to the right, apex left)
    0b0000000010000000,0b0000000110000000,0b0000001110000000,0b0000011110000000,
    0b0000111110000000,0b0001111110000000,0b0111111111111100,0b1111111111111100,
    0b0111111111111100,0b0001111110000000,0b0000111110000000,0b0000011110000000,
    0b0000001110000000,0b0000000110000000,0b0000000010000000,0b0000000000000000 },
  { // B_REDO : right arrow (shaft to the left, apex right)
    0b0000000001000000,0b0000000001100000,0b0000000001110000,0b0000000001111000,
    0b0000000001111100,0b0000000001111110,0b0011111111111110,0b0011111111111111,
    0b0011111111111110,0b0000000001111110,0b0000000001111100,0b0000000001111000,
    0b0000000001110000,0b0000000001100000,0b0000000001000000,0b0000000000000000 },
  { // B_VIEW : magnifying glass (hollow lens upper-left, handle to lower-right)
    0b0000011111000000,0b0000100000100000,0b0001000000010000,0b0001000000010000,
    0b0001000000010000,0b0001000000010000,0b0001000000010000,0b0000100000100000,
    0b0000011111100000,0b0000000000110000,0b0000000000011000,0b0000000000001100,
    0b0000000000000110,0b0000000000000011,0b0000000000000000,0b0000000000000000 },
  { // B_GRID : tic-tac-toe grid
    0b0000010000100000,0b0000010000100000,0b0000010000100000,0b0000010000100000,
    0b0000010000100000,0b1111111111111111,0b0000010000100000,0b0000010000100000,
    0b0000010000100000,0b0000010000100000,0b1111111111111111,0b0000010000100000,
    0b0000010000100000,0b0000010000100000,0b0000010000100000,0b0000010000100000 },
  { // B_COPY : two overlapping sheets
    0b0000000000000000,0b0000000000000000,0b0011111111000000,0b0010000001000000,
    0b0010000001000000,0b0010001111111100,0b0010001000000100,0b0010001000000100,
    0b0010001000000100,0b0011111111000100,0b0000001000000100,0b0000001000000100,
    0b0000001000000100,0b0000001111111100,0b0000000000000000,0b0000000000000000 },
  { // B_PASTE : clipboard with clip tab
    0b0000000000000000,0b0000001111000000,0b0000001111000000,0b0001111111111000,
    0b0001000000001000,0b0001000000001000,0b0001000000001000,0b0001000000001000,
    0b0001000000001000,0b0001000000001000,0b0001000000001000,0b0001000000001000,
    0b0001000000001000,0b0001000000001000,0b0001111111111000,0b0000000000000000 },
};

class PixArt : public Pico8 {
  uint8_t canvas[CANVAS][CANVAS];             // color index 0..15 per pixel
  uint8_t uStack[UNDO_MAX][CANVAS][CANVAS];   // undo snapshots (newest at top)
  uint8_t rStack[UNDO_MAX][CANVAS][CANVAS];   // redo snapshots
  uint8_t clip[VIEW][VIEW];                   // 16x16 copy/paste clipboard
  int  uCount = 0, rCount = 0;
  bool hasClip = false;
  int  vx = 0, vy = 0;             // PIXEL viewport top-left (canvas coords)
  int  sel = 7;                    // selected color (7 = white)
  int  tool = B_PEN;
  bool overview = false;           // false: PIXEL mode, true: OVERVIEW mode
  bool grid = false;               // grid overlay on/off
  bool prevDrag = false;
  int  grabCX = 0, grabCY = 0;     // Hand-tool grab anchor (canvas coords)

  // push a copy of `src` onto a snapshot stack, dropping the oldest when full
  static void push(uint8_t* base, int& cnt, const uint8_t* src){
    if (cnt == UNDO_MAX){ memmove(base, base + SZ, SZ * (UNDO_MAX - 1)); cnt = UNDO_MAX - 1; }
    memcpy(base + cnt * SZ, src, SZ);
    ++cnt;
  }
  void beginStroke(){              // snapshot pre-edit state; invalidate redo
    push(reinterpret_cast<uint8_t*>(uStack), uCount, reinterpret_cast<uint8_t*>(canvas));
    rCount = 0;
  }
  void doUndo(){
    if (uCount == 0) return;
    uint8_t* cur = reinterpret_cast<uint8_t*>(canvas);
    push(reinterpret_cast<uint8_t*>(rStack), rCount, cur);
    memcpy(cur, reinterpret_cast<uint8_t*>(uStack) + (--uCount) * SZ, SZ);
  }
  void doRedo(){
    if (rCount == 0) return;
    uint8_t* cur = reinterpret_cast<uint8_t*>(canvas);
    push(reinterpret_cast<uint8_t*>(uStack), uCount, cur);
    memcpy(cur, reinterpret_cast<uint8_t*>(rStack) + (--rCount) * SZ, SZ);
  }
  void doCopy(){
    for (int y = 0; y < VIEW; ++y)
      for (int x = 0; x < VIEW; ++x) clip[y][x] = canvas[vy + y][vx + x];
    hasClip = true;
  }
  void doPaste(){
    if (!hasClip) return;
    beginStroke();
    for (int y = 0; y < VIEW; ++y)
      for (int x = 0; x < VIEW; ++x) canvas[vy + y][vx + x] = clip[y][x];
  }

  // Aseprite-compatible keyboard shortcuts (keys come from the HIF keyboard
  // FIFO: low 16 bits = ASCII, high 16 bits = modifier status).
  void pollKeys(){
    int guard = 0;
    while (B8_HIF_KB_LEN > 0 && guard++ < 32) {
      const u32 k     = B8_HIF_KB_RX;
      const int ascii = k & 0xffff;
      const bool ctrl = ((k >> 16) & B8_HIF_KB_STATUS_CONTROL) != 0;
      if (ctrl) {
        switch (ascii) {
          case 'z':            doUndo(); break;               // Ctrl+Z
          case 'y': case 'Z':  doRedo(); break;               // Ctrl+Y / Ctrl+Shift+Z
          case 'c': if (overview) doCopy();  break;           // Ctrl+C (overview only)
          case 'v': if (overview) doPaste(); break;           // Ctrl+V (overview only)
        }
      } else {
        switch (ascii) {
          case 'b': tool = B_PEN;  break;                     // pencil
          case 'i': tool = B_EYE;  break;                     // eyedropper
          case 'h': tool = B_HAND; break;                     // hand / pan
          case 'g': grid = !grid;  break;                     // toggle grid
        }
      }
    }
  }

  void dispatch(int btn){
    switch (btn) {
      case B_PEN: case B_HAND: case B_EYE: tool = btn; break;
      case B_UNDO:  doUndo(); break;
      case B_REDO:  doRedo(); break;
      case B_VIEW:  overview = !overview; break;
      case B_GRID:  grid = !grid; break;
      case B_COPY:  if (overview) doCopy();  break;   // copy/paste = overview only
      case B_PASTE: if (overview) doPaste(); break;
    }
  }

  void _init() override {
    memset(canvas, 0, sizeof(canvas));
    uCount = rCount = 0;
    hasClip = false;
    vx = vy = 0;                   // start on the top-left tile (0,0)-(15,15)
  }

  void _update() override {
    pollKeys();

    const b8HifMouseStatus* ms = b8HifGetMouseStatus();
    const int  mx    = ms->mouse_x >> 4;   // fixed-point (/16) -> pixels
    const int  my    = ms->mouse_y >> 4;
    const bool drag  = ms->is_dragging;
    const bool press = drag && !prevDrag;

    if (drag) {
      if (my < EDIT_H) {                        // inside the edit area
        if (overview) {
          // view-only: a tap moves the viewport box, snapped to 16px tiles
          if (press) {
            vx = clampi((mx / GRID_PX) * GRID_PX, 0, VMAX);
            vy = clampi((my / GRID_PX) * GRID_PX, 0, VMAX);
          }
        } else {
          const int cx = clampi(vx + mx / DOT, 0, CANVAS - 1);
          const int cy = clampi(vy + my / DOT, 0, CANVAS - 1);
          switch (tool) {
            case B_PEN:
              if (press) beginStroke();
              canvas[cy][cx] = (uint8_t)sel;
              break;
            case B_EYE:
              if (press) sel = canvas[cy][cx];
              break;
            case B_HAND:
              if (press) { grabCX = vx + mx / DOT; grabCY = vy + my / DOT; }
              else { vx = clampi(grabCX - mx / DOT, 0, VMAX);
                     vy = clampi(grabCY - my / DOT, 0, VMAX); }
              break;
          }
        }
      } else if (press) {                       // discrete tap on the UI panel
        if (my >= PAL_Y && my < PAL_Y + PAL_H && mx >= 0 && mx < SCRW) {
          const int col = clampi(mx / PAL_SW, 0, PAL_COLS - 1);
          const int row = (my - PAL_Y) / PAL_SW;   // 0 (top) or 1 (bottom)
          sel = clampi(row * PAL_COLS + col, 0, 15);
        } else if (my >= BARA_Y && my < BARA_Y + ICON) {
          // match the drawn layout: Undo/Redo shifted right by ICON
          for (int i = 0; i < ROWA; ++i) {
            const int bx = i * PITCH + (i >= B_UNDO ? ICON : 0);
            if (mx >= bx && mx < bx + ICON) { dispatch(i); break; }
          }
        } else if (my >= BARB_Y && my < BARB_Y + ICON) {
          const int i = mx / PITCH; if (i < ROWB) dispatch(ROWA + i);
        }
      }
    }
    prevDrag = drag;
  }

  void drawIcon(int x, int y, int id, Color fg){
    for (int r = 0; r < 16; ++r){
      const uint16_t bits = kIcon[id][r];
      for (int c = 0; c < 16; ++c)
        if ((bits >> (15 - c)) & 1) pset(x + c, y + r, fg);
    }
  }

  void drawButton(int id, int bx, int by, Color fg, bool active){
    rectfill(bx, by, bx + ICON, by + ICON, WHITE);      // button bg (white panel)
    drawIcon(bx, by, id, fg);
    if (active) rect(bx - 1, by - 1, bx + ICON, by + ICON, LIGHT_GREY);
  }

  void _draw() override {
    cls(BLACK);

    if (overview) {
      // whole canvas at 1:1, horizontal runs coalesced (skip black bg)
      for (int y = 0; y < CANVAS; ++y) {
        int x = 0;
        while (x < CANVAS) {
          const int c = canvas[y][x];
          int x2 = x;
          while (x2 < CANVAS && canvas[y][x2] == c) ++x2;
          if (c != 0) rectfill(x, y, x2, y + 1, (Color)c);  // cols x..x2-1
          x = x2;
        }
      }
    } else {
      // 16x16 slice zoomed by DOT
      for (int gy = 0; gy < VIEW; ++gy)
        for (int gx = 0; gx < VIEW; ++gx) {
          const int c  = canvas[vy + gy][vx + gx];
          const int sx = gx * DOT, sy = gy * DOT;
          rectfill(sx, sy, sx + DOT, sy + DOT, (Color)c);
        }
    }

    if (grid) {                                  // overlay grid lines
      for (int g = 0; g <= EDIT_H; g += GRID_PX) {
        rectfill(g, 0, g + 1, EDIT_H, (Color)5); // vertical
        rectfill(0, g, EDIT_H, g + 1, (Color)5); // horizontal
      }
    }
    // viewport box: 1px OUTSIDE the edited region [vx,vx+VIEW-1] x [vy,vy+VIEW-1].
    // Edges past the canvas edge (e.g. vx==0 -> left col -1) clip off-screen.
    if (overview) rect(vx - 1, vy - 1, vx + VIEW, vy + VIEW, WHITE);
    rect(0, 0, EDIT_H, EDIT_H, (Color)5);        // edit-area border

    // white background for everything below the palette (toolbars sit on white)
    rectfill(0, PAL_Y + PAL_H, SCRW, SCRH, WHITE);

    // palette: 16 colors as 16x16 swatches, 8 per row x 2 rows
    for (int i = 0; i < 16; ++i) {
      const int px = (i % PAL_COLS) * PAL_SW;
      const int py = PAL_Y + (i / PAL_COLS) * PAL_SW;
      rectfill(px, py, px + PAL_SW, py + PAL_SW, (Color)i);
    }
    { // selection marker
      const int sx = (sel % PAL_COLS) * PAL_SW;
      const int sy = PAL_Y + (sel / PAL_COLS) * PAL_SW;
      rect(sx, sy, sx + PAL_SW - 1, sy + PAL_SW - 1, WHITE);
    }

    // toolbar row A: Pen / Hand / Eye / Undo / Redo
    for (int i = 0; i < ROWA; ++i) {
      Color fg = BLACK;
      bool active = false;
      if (i < 3) {                          // Pen/Hand/Eye are inert in overview
        if (overview) fg = LIGHT_GREY;
        else active = (i == tool);
      } else {
        if (i == B_UNDO && uCount == 0) fg = LIGHT_GREY;
        if (i == B_REDO && rCount == 0) fg = LIGHT_GREY;
      }
      // Undo/Redo are shifted right by ICON to separate them from Pen/Hand/Eye
      const int bx = i * PITCH + (i >= B_UNDO ? ICON : 0);
      drawButton(i, bx, BARA_Y, fg, active);
    }
    // toolbar row B: View / Grid / Copy / Paste
    for (int j = 0; j < ROWB; ++j) {
      const int id = ROWA + j;
      Color fg = BLACK;
      // View (magnifier): black while NOT in overview (tap to zoom out), grey once in it; no active frame.
      if (id == B_VIEW  && overview)               fg = LIGHT_GREY;
      if (id == B_COPY  && !overview)              fg = LIGHT_GREY;  // overview only
      if (id == B_PASTE && (!overview || !hasClip)) fg = LIGHT_GREY;
      bool active = (id == B_GRID && grid);
      drawButton(id, j * PITCH, BARB_Y, fg, active);
    }
  }
};

static PixArt* app;

int main(){
  app = new PixArt;
  app->run();
  return 0;
}
