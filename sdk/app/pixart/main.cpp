// PixArt — BEEP-8 pixel art editor (standalone .b8 app)
//
// 128x128 canvas locked to the PICO-8 16-color palette. Two view modes:
//   - PIXEL:    a 16x16 window zoomed 8x (each logical pixel = an 8x8 dot),
//               panned over the full canvas; this is where you draw.
//   - OVERVIEW: the whole 128x128 canvas at 1:1, view-only. A tap or drag moves
//               the viewport box (snapped to 16px tiles). Return with View btn.
// Toolbar (three rows): row A = Pen / Hand / Eyedropper / Undo / Redo;
//   row B = Copy / Paste / Cut / View / Grid / Help;
//   row C = Flip-H / Flip-V / Mirror.
// Undo/Redo keep up to UNDO_MAX steps. Cut/Copy/Paste act on the current 16x16
// viewport tile in OVERVIEW mode; the two flips mirror it in either mode (the
// visible slice in PIXEL, the white box in OVERVIEW). Mirror is a persistent
// symmetry-draw mode (tap to cycle off -> left/right -> up/down): while on, the
// Pen also paints each pixel's reflection across the centre of the 16x16 window.
// Input is touch/mouse.
//
// NOTE: pico8 rectfill()/rect() take x1/y1 as EXCLUSIVE (right/bottom edge not
// drawn), so fills use +size and a 1px-wide line is (a, a+1).
//
// Not yet wired (later increments): PNG save/load (ROM-side C codec), and
// Save-with-optional-share to the CC0 asset commons.
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
static constexpr int BARC_Y  = BARB_Y + 20;  // toolbar row C (below row B)

static constexpr int UNDO_MAX = 4;            // undo / redo depth
static constexpr int FX_FRAMES = 8;           // copy/paste "pressed" nudge duration
static constexpr int SZ = CANVAS * CANVAS;    // bytes per canvas snapshot

// toolbar button ids. Screen positions come from btnPos(); the three rows are:
//   A: [Pen Hand Eye]     (left)  ...  [Undo Redo]     (right, edge-aligned)
//   B: [Copy Paste Cut]   (left)  ...  [View Grid Help] (right, edge-aligned)
//   C: [FlipH FlipV Mirror] (left)
enum Btn { B_PEN = 0, B_HAND, B_EYE, B_UNDO, B_REDO,
           B_VIEW, B_GRID, B_CUT, B_COPY, B_PASTE, B_HELP,
           B_FLIPH, B_FLIPV, B_MIRROR, B_N };

static inline int clampi(int v, int lo, int hi){
  return v < lo ? lo : (v > hi ? hi : v);
}

// Top-left screen position of toolbar button `id`. Pen/Hand/Eye and
// Copy/Paste/Help hug the left; Undo/Redo and View/Grid/Cut are right-aligned
// to the screen edge. Both hit-testing and drawing go through this, so the two
// stay in sync.
static void btnPos(int id, int& x, int& y){
  switch (id) {
    case B_PEN:   x = 0 * PITCH;             y = BARA_Y; break;
    case B_HAND:  x = 1 * PITCH;             y = BARA_Y; break;
    case B_EYE:   x = 2 * PITCH;             y = BARA_Y; break;
    case B_UNDO:  x = SCRW - ICON - PITCH;   y = BARA_Y; break;
    case B_REDO:  x = SCRW - ICON;           y = BARA_Y; break;
    case B_COPY:  x = 0 * PITCH;             y = BARB_Y; break;
    case B_PASTE: x = 1 * PITCH;             y = BARB_Y; break;
    case B_CUT:   x = 2 * PITCH;             y = BARB_Y; break;
    case B_VIEW:  x = SCRW - ICON - 2*PITCH; y = BARB_Y; break;
    case B_GRID:  x = SCRW - ICON - PITCH;   y = BARB_Y; break;
    case B_HELP:  x = SCRW - ICON;           y = BARB_Y; break;
    case B_FLIPH: x = 0 * PITCH;             y = BARC_Y; break;
    case B_FLIPV: x = 1 * PITCH;             y = BARC_Y; break;
    case B_MIRROR:x = 2 * PITCH;             y = BARC_Y; break;
    default:      x = 0;                     y = 0;      break;
  }
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
  { // B_CUT : scissors (blades crossing in an X, two round finger handles below)
    0b0010000000000100,0b0001000000001000,0b0000100000010000,0b0000010000100000,
    0b0000001001000000,0b0000000110000000,0b0000000110000000,0b0000001001000000,
    0b0000010000100000,0b0000100000010000,0b0001000000001000,0b0001100000011000,
    0b0010010000100100,0b0010010000100100,0b0001100000011000,0b0000000000000000 },
  { // B_COPY : two overlapping sheets, front one lined with text
    0b0000000000000000,0b0000000000000000,0b0000001111111100,0b0000001000000100,
    0b0000001000000100,0b0011111111000100,0b0010000001000100,0b0010000001000100,
    0b0010111101000100,0b0010000001000100,0b0010111101000100,0b0010000001111100,
    0b0010111101000000,0b0010000001000000,0b0011111111000000,0b0000000000000000 },
  { // B_PASTE : clipboard (with left clip) behind a lined document sheet
    0b0000000000000000,0b0011111111110000,0b0010000000010000,0b0010000000010000,
    0b0010000000010000,0b1110001111111100,0b1010001000000100,0b1010001000000100,
    0b1110001011110100,0b0010001000000100,0b0010001011110100,0b0010001000000100,
    0b0011111011110100,0b0000001000000100,0b0000001111111100,0b0000000000000000 },
  { // B_HELP : question mark (bowl curving down to a centered stem + dot)
    0b0000000000000000,0b0000011111100000,0b0000111001110000,0b0000110000110000,
    0b0000000000110000,0b0000000001110000,0b0000000011100000,0b0000000111000000,
    0b0000000110000000,0b0000000110000000,0b0000000110000000,0b0000000000000000,
    0b0000000110000000,0b0000000110000000,0b0000000000000000,0b0000000000000000 },
  { // B_FLIPH : mirror L/R - two triangles pointing outward across a dashed vert axis
    0b0000000110000000,0b0000000000000000,0b0000000110000000,0b0000010000100000,
    0b0000110110110000,0b0001110000111000,0b0011110110111100,0b0111110000111110,
    0b0111110110111110,0b0011110000111100,0b0001110110111000,0b0000110000110000,
    0b0000010110100000,0b0000000000000000,0b0000000110000000,0b0000000000000000 },
  { // B_FLIPV : mirror U/D - two triangles pointing outward across a dashed horiz axis
    0b0000000000000000,0b0000000110000000,0b0000001111000000,0b0000011111100000,
    0b0000111111110000,0b0001111111111000,0b0000000000000000,0b1010101010101010,
    0b1010101010101010,0b0000000000000000,0b0001111111111000,0b0000111111110000,
    0b0000011111100000,0b0000001111000000,0b0000000110000000,0b0000000000000000 },
  { // B_MIRROR : mirror-mode toggle (L/R symmetry) - hollow trapezoids facing a
    // dashed vertical axis. The U/D state swaps in kIconMirrorV (this rotated 90).
    0b0000000000000000,0b0000000110000000,0b0100000110000010,0b0110000000000110,
    0b0101100000110010,0b0100010110100010,0b0100010110100010,0b0100010000100010,
    0b0100010000100010,0b0100010110100010,0b0100010110100010,0b0101100000110010,
    0b0110000000000110,0b0100000110000010,0b0000000110000000,0b0000000000000000 },
};

// U/D mirror icon: the B_MIRROR bitmap rotated 90 degrees (trapezoids facing a
// dashed horizontal axis). Shown on the Mirror button while in up/down mode.
static const uint16_t kIconMirrorV[16] = {
  0b0000000000000000,0b0011111111111100,0b0001000000001000,0b0000100000010000,
  0b0000100000010000,0b0000011111100000,0b0000000000000000,0b0110011001100110,
  0b0110011001100110,0b0000000000000000,0b0000011111100000,0b0000100000010000,
  0b0000100000010000,0b0001000000001000,0b0011111111111100,0b0000000000000000 };

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
  int  mirror = 0;                 // symmetry-draw mode: 0 off, 1 left-right, 2 up-down
  bool help = false;               // HELP overlay: shortcuts only, editing disabled
  bool prevDrag = false;
  int  grabCX = 0, grabCY = 0;     // Hand-tool grab anchor (canvas coords)
  int  fxId = -1, fxTtl = 0;       // button-press feedback (copy/paste): id + frames left

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
  void doCut(){                    // copy the tile to the clipboard, then clear it
    doCopy();
    beginStroke();
    for (int y = 0; y < VIEW; ++y)
      for (int x = 0; x < VIEW; ++x) canvas[vy + y][vx + x] = 0;
  }
  void doFlipH(){                  // mirror the 16x16 tile left<->right (reverse cols)
    beginStroke();
    for (int y = 0; y < VIEW; ++y)
      for (int x = 0; x < VIEW / 2; ++x){
        uint8_t* a = &canvas[vy + y][vx + x];
        uint8_t* b = &canvas[vy + y][vx + VIEW - 1 - x];
        const uint8_t t = *a; *a = *b; *b = t;
      }
  }
  void doFlipV(){                  // mirror the 16x16 tile top<->bottom (reverse rows)
    for (int y = 0; y < VIEW / 2; ++y)
      for (int x = 0; x < VIEW; ++x){
        uint8_t* a = &canvas[vy + y][vx + x];
        uint8_t* b = &canvas[vy + VIEW - 1 - y][vx + x];
        const uint8_t t = *a; *a = *b; *b = t;
      }
  }
  // paint one logical pixel plus its mirror-mode reflection. The symmetry axis is
  // the centre of the visible 16x16 window, so the partner pixel sits at local
  // 15-x (left-right) or 15-y (up-down): canvas vx+15-(cx-vx) / vy+15-(cy-vy).
  // Both stay inside 0..CANVAS-1 because vx,vy <= VMAX and cx-vx,cy-vy are 0..15.
  void paint(int cx, int cy){
    canvas[cy][cx] = (uint8_t)sel;
    if      (mirror == 1) canvas[cy][vx + 15 - (cx - vx)] = (uint8_t)sel;
    else if (mirror == 2) canvas[vy + 15 - (cy - vy)][cx] = (uint8_t)sel;
  }
  // cut/copy/paste are overview-only; on a successful action flash the button (nudge +1,+1)
  void fireCut(){   if (!overview) return;             doCut();   fxId = B_CUT;   fxTtl = FX_FRAMES; }
  void fireCopy(){  if (!overview) return;             doCopy();  fxId = B_COPY;  fxTtl = FX_FRAMES; }
  void firePaste(){ if (!overview || !hasClip) return; doPaste(); fxId = B_PASTE; fxTtl = FX_FRAMES; }
  // flips work in both modes: they mirror the current 16x16 tile (the visible slice
  // in PIXEL mode, the white viewport box in OVERVIEW) -- always available.
  void fireFlipH(){ doFlipH(); fxId = B_FLIPH; fxTtl = FX_FRAMES; }
  void fireFlipV(){ doFlipV(); fxId = B_FLIPV; fxTtl = FX_FRAMES; }

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
          case 'x': fireCut();   break;                       // Ctrl+X (overview only)
          case 'c': fireCopy();  break;                       // Ctrl+C (overview only)
          case 'v': firePaste(); break;                       // Ctrl+V (overview only)
        }
      } else {
        switch (ascii) {
          case 'b': tool = B_PEN;  break;                     // pencil
          case 'i': tool = B_EYE;  break;                     // eyedropper
          case 'h': tool = B_HAND; break;                     // hand / pan
          case 'g': grid = !grid;  break;                     // toggle grid
          case 'm': mirror = (mirror + 1) % 3; break;         // cycle mirror mode
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
      case B_CUT:   fireCut();   break;               // cut/copy/paste = overview only
      case B_COPY:  fireCopy();  break;
      case B_PASTE: firePaste(); break;
      case B_HELP:  help = !help; break;              // toggle the HELP overlay
      case B_FLIPH: fireFlipH(); break;               // flips = both modes
      case B_FLIPV: fireFlipV(); break;
      case B_MIRROR: mirror = (mirror + 1) % 3; break; // cycle off -> L/R -> U/D
    }
  }

  void _init() override {
    memset(canvas, 0, sizeof(canvas));
    uCount = rCount = 0;
    hasClip = false;
    mirror = 0;
    vx = vy = 0;                   // start on the top-left tile (0,0)-(15,15)
  }

  void _update() override {
    pollKeys();
    if (fxTtl > 0) --fxTtl;                 // fade the copy/paste press feedback

    const b8HifMouseStatus* ms = b8HifGetMouseStatus();
    const int  mx    = ms->mouse_x >> 4;   // fixed-point (/16) -> pixels
    const int  my    = ms->mouse_y >> 4;
    const bool drag  = ms->is_dragging;
    const bool press = drag && !prevDrag;

    if (help) {                     // HELP overlay is modal: no editing; a tap exits
      if (press) help = false;
      prevDrag = drag;
      return;
    }

    if (drag) {
      if (my < EDIT_H) {                        // inside the edit area
        if (overview) {
          // view-only: tap or drag moves the viewport box, snapped to 16px tiles
          vx = clampi((mx / GRID_PX) * GRID_PX, 0, VMAX);
          vy = clampi((my / GRID_PX) * GRID_PX, 0, VMAX);
        } else {
          const int cx = clampi(vx + mx / DOT, 0, CANVAS - 1);
          const int cy = clampi(vy + my / DOT, 0, CANVAS - 1);
          switch (tool) {
            case B_PEN:
              if (press) beginStroke();
              paint(cx, cy);                          // + mirror reflection when on
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
          tool = B_PEN;                            // picking a color implies "draw"
        } else {                                   // toolbar buttons (positions from btnPos)
          for (int id = 0; id < B_N; ++id) {
            int bx, by; btnPos(id, bx, by);
            if (mx >= bx && mx < bx + ICON && my >= by && my < by + ICON) { dispatch(id); break; }
          }
        }
      }
    }
    prevDrag = drag;
  }

  void drawIcon(int x, int y, const uint16_t* bits, Color fg){
    for (int r = 0; r < 16; ++r){
      const uint16_t row = bits[r];
      for (int c = 0; c < 16; ++c)
        if ((row >> (15 - c)) & 1) pset(x + c, y + r, fg);
    }
  }

  void drawButton(int bx, int by, Color fg, bool pressed, const uint16_t* bits){
    rectfill(bx, by, bx + ICON, by + ICON, WHITE);      // button bg (white panel)
    const int d = pressed ? 1 : 0;                       // press feedback: nudge (+1,+1)
    drawIcon(bx + d, by + d, bits, fg);
  }

  // full-screen keyboard-shortcut list; nothing of the editor is drawn behind it
  void drawHelp(){
    cls(DARK_BLUE);
    int y = 6;
    sprint(4, y, WHITE, "SHORTCUTS");        y += 16;
    sprint(4, y, LIGHT_GREY, "B  PEN");       y += 11;
    sprint(4, y, LIGHT_GREY, "H  HAND");      y += 11;
    sprint(4, y, LIGHT_GREY, "I  EYEDROPPER"); y += 11;
    sprint(4, y, LIGHT_GREY, "G  GRID");      y += 11;
    sprint(4, y, LIGHT_GREY, "M  MIRROR");    y += 15;
    sprint(4, y, LIGHT_GREY, "CTRL+Z  UNDO"); y += 11;
    sprint(4, y, LIGHT_GREY, "CTRL+Y  REDO"); y += 15;
    sprint(4, y, WHITE, "OVERVIEW ONLY:");    y += 11;
    sprint(4, y, LIGHT_GREY, "CTRL+X  CUT");  y += 11;
    sprint(4, y, LIGHT_GREY, "CTRL+C  COPY"); y += 11;
    sprint(4, y, LIGHT_GREY, "CTRL+V  PASTE"); y += 16;
    sprint(4, y, BLUE, "TAP TO RETURN");
  }

  void _draw() override {
    if (help) { drawHelp(); return; }        // HELP overlay replaces the whole screen
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
      // OVERVIEW draws the canvas 1:1 (no pan), so grid lines sit at fixed screen
      // positions. In PIXEL mode the slice is panned by vx/vy, so the grid must
      // scroll with it (offset by the sub-cell part of the pan) to stay aligned
      // with the pixels under it as the Hand tool moves 1 canvas-pixel at a time.
      const int offx = overview ? 0 : (vx * DOT) % GRID_PX;
      const int offy = overview ? 0 : (vy * DOT) % GRID_PX;
      for (int g = -offx; g <= EDIT_H; g += GRID_PX)
        if (g >= 0) rectfill(g, 0, g + 1, EDIT_H, (Color)5); // vertical
      for (int g = -offy; g <= EDIT_H; g += GRID_PX)
        if (g >= 0) rectfill(0, g, EDIT_H, g + 1, (Color)5); // horizontal
    }
    // viewport box: 1px OUTSIDE the edited region [vx,vx+VIEW-1] x [vy,vy+VIEW-1].
    // Edges past the canvas edge (e.g. vx==0 -> left col -1) clip off-screen.
    if (overview) rect(vx - 1, vy - 1, vx + VIEW, vy + VIEW, WHITE);

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

    // toolbar buttons (positions via btnPos). fg = BLACK when active/available,
    // LIGHT_GREY when inactive/disabled; the tool trio shows the current tool black.
    for (int id = 0; id < B_N; ++id) {
      Color fg = BLACK;
      switch (id) {
        case B_PEN: case B_HAND: case B_EYE: fg = (id == tool) ? BLACK : LIGHT_GREY; break;
        case B_UNDO:  if (uCount == 0)               fg = LIGHT_GREY; break;
        case B_REDO:  if (rCount == 0)               fg = LIGHT_GREY; break;
        // View (magnifier): black while NOT in overview (tap to zoom out), grey once in it.
        case B_VIEW:  if (overview)                  fg = LIGHT_GREY; break;
        // Grid: ON = black, OFF = light grey (toggle indicated by icon color).
        case B_GRID:  if (!grid)                     fg = LIGHT_GREY; break;
        case B_CUT:   if (!overview)                 fg = LIGHT_GREY; break;  // overview only
        case B_COPY:  if (!overview)                 fg = LIGHT_GREY; break;  // overview only
        case B_PASTE: if (!overview || !hasClip)     fg = LIGHT_GREY; break;
        case B_HELP:  break;                          // always available
        case B_FLIPH: case B_FLIPV: break;            // available in both modes
        case B_MIRROR: if (mirror == 0) fg = LIGHT_GREY; break;  // off = grey, on = black
      }
      int bx, by; btnPos(id, bx, by);
      const bool pressed = (fxTtl > 0 && fxId == id);   // cut/copy/paste tap flash
      // Mirror shows the L/R glyph for off & left-right, the rotated one for up-down.
      const uint16_t* bits = (id == B_MIRROR && mirror == 2) ? kIconMirrorV : kIcon[id];
      drawButton(bx, by, fg, pressed, bits);
    }
    // light-grey box grouping the three mutually-exclusive tools (drawn over the
    // button panels so its left edge stays visible at column 0). Top edge nudged
    // up 1px (BARA_Y-2) for a touch more breathing room above the icons.
    rect(0, BARA_Y - 2, 2 * PITCH + ICON, BARA_Y + ICON, LIGHT_GREY);
  }
};

static PixArt* app;

int main(){
  app = new PixArt;
  app->run();
  return 0;
}
