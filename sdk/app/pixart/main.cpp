// PixArt — BEEP-8 pixel art editor (standalone .b8 app)
//
// 128x128 canvas locked to the PICO-8 16-color palette. Two view modes:
//   - PIXEL:    a 16x16 window zoomed 8x (each logical pixel = an 8x8 dot),
//               panned over the full canvas; this is where you draw.
//   - OVERVIEW: the whole 128x128 canvas at 1:1, view-only. A tap or drag moves
//               the viewport box (snapped to 16px tiles). Return with View btn.
// Toolbar (three rows): row A = Pen / Hand / Eyedropper / Fill / Undo / Redo;
//   row B = Copy / Paste / Cut / View / Grid / Help;
//   row C = Flip-H / Flip-V / Mirror ... plus, right-aligned across rows C and D,
//   a boxed 2x2 transfer block: [Save Load] over [Download Import].
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
// The transfer block's top row is the cloud (Save / Load), its bottom row the
// local device (Download / Import) -- the icons mirror that split, cloud glyph
// vs tray glyph, with the arrow pointing into the glyph to store and out of it
// to retrieve. Save / Load sync the canvas with this device's private cloud
// slot: Save encodes to an indexed PNG (png::EncodeIndexed -> base64url ->
// cloudsave::Set), Load reverses it (cloudsave::Get -> base64 ->
// png::DecodeIndexed) and is undoable. The slot key is per-device (see
// kSlotField). Download saves the PNG to the user's device as a local
// file over the SCI /download driver, streamed a chunk per frame to stay under
// the 8KB FIFO. Not yet wired: share-to-the-CC0-asset-commons.
//
// HOSTED MODE: when an embedding page owns the file -- the AI Playground opens
// PixArt as the editor for its sprite0.png tab -- there is no local file to
// pick or save, so the bottom row of the transfer block disappears and the ROM
// does that traffic itself: it pulls the page's sheet in at startup and pushes
// the canvas back shortly after the user stops drawing (see pumpHost()). The
// cloud row is unaffected. upload::Stat() reports the wiring; everything else
// on screen behaves identically either way.
#include <pico8.h>
#include <beep8.h>
#include <string.h>
#include <png.h>
#include <base64.h>
#include <cloudsave.h>
#include <savedata.h>
#include <download.h>
#include <upload.h>

// base64url-encoded length (incl. NUL) for n input bytes -- a compile-time
// constant so the save scratch buffer can be a fixed static array.
#define B64_CAP(n) ((((n) + 2) / 3) * 4 + 1)

// PixArt's cloud slot is PER-DEVICE, not one world-shared key: on first run a
// random 16-char alnum key is generated and stashed in browser-local savedata
// (localStorage, per-ROM scope), then reused, so each device owns a private
// slot and devices no longer overwrite each other. (Per-DEVICE, not per-user
// across devices -- true cross-device identity needs login, which arrives with
// the gallery / beep8.assets service.) kSlotField is the savedata field the key
// lives under; the resolved key is held in PixArt::slotKey.
static const char kSlotField[] = "pixslot";

// Shared save/load scratch, used one operation at a time and kept off the stack
// (~16.6KB PNG + ~22KB base64 + 16KB decoded indices + ~16.5KB inflate scratch).
// 128 == CANVAS below; g_raw holds the inflated, still-filtered image, so it is
// h*(1+w) = 128*129 bytes.
static uint8_t g_png[PNG_ENCODE_CAP(128, 128)];
static char    g_b64[B64_CAP(PNG_ENCODE_CAP(128, 128))];
static uint8_t g_tmp[128 * 128];
static uint8_t g_raw[128 * 129];
// Imported PNGs come from arbitrary tools, so they can be far larger than our own
// stored-deflate output (g_png): extra chunks, different filters, truecolor. Give
// import its own roomy receive buffer; oversize files past this are rejected
// cleanly (not decoded) rather than corrupting the canvas.
static uint8_t g_imp[64 * 1024];
// Reply buffer for upload::Stat() -- one flag byte, kept apart from g_imp so a
// status poll can never be confused with (or clobber) an in-flight import.
static uint8_t g_stat[4];

// --- browser-local savedata helpers (per-ROM localStorage over SCI) ----------
// One command per open: "get <key>\n" -> value bytes then EOF, or "set k=v\n".
static int sd_get(const char* key, char* buf, int cap){
  savedata::Reset();
  FILE* fp = fopen("/savedata/con0", "r+");
  if (!fp) return -1;
  char cmd[48];
  const int m = snprintf(cmd, sizeof(cmd), "get %s\n", key);
  fwrite(cmd, 1, m, fp); fflush(fp);
  int n = fread(buf, 1, cap - 1, fp);
  if (n < 0) n = 0;
  buf[n] = 0;
  fclose(fp);
  return n;
}
static void sd_set(const char* key, const char* val){
  savedata::Reset();
  FILE* fp = fopen("/savedata/con0", "r+");
  if (!fp) return;
  char cmd[48];
  const int m = snprintf(cmd, sizeof(cmd), "set %s=%s\n", key, val);
  fwrite(cmd, 1, m, fp); fflush(fp);
  fclose(fp);
}

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
static constexpr int BARD_Y  = BARC_Y + 20;  // toolbar row D (import; below row C)

static constexpr int UNDO_MAX = 4;            // undo / redo depth
static constexpr int FX_FRAMES = 8;           // copy/paste "pressed" nudge duration
static constexpr int SZ = CANVAS * CANVAS;    // bytes per canvas snapshot
static constexpr int DL_CHUNK = 4000;         // local-download bytes per frame (< 8KB SCI FIFO)

// The upload channel carries two kinds of reply, one at a time (see pumpHost).
static constexpr int IMP_IDLE = 0;            // nothing outstanding
static constexpr int IMP_FILE = 1;            // a file transfer is arriving
static constexpr int IMP_STAT = 2;            // a 1-byte status reply is arriving
static constexpr int HOST_STAT_WAIT = 120;    // 2s: a host this old cannot be hosted
static constexpr int HOST_POLL      = 15;     // idle status polls, 4x/second
static constexpr int HOST_SYNC      = 30;     // push edits back 0.5s after the last one

// toolbar button ids. Screen positions come from btnPos(); the four rows are:
//   A: [Pen Hand Eye Fill] (left) ...  [Undo Redo]     (right, edge-aligned)
//   B: [Copy Paste Cut]   (left)  ...  [View Grid Help] (right, edge-aligned)
//   C: [FlipH FlipV Mirror] (left) ...  [Save Load]     (right, edge-aligned)
//   D:                          ...      [DL  Import]   (right, edge-aligned)
// Rows C/D right form one 2x2 transfer block, boxed together on screen:
// row C is the cloud slot, row D is a local file; the left column stores (out),
// the right column retrieves (in). The icons encode the same two axes.
enum Btn { B_PEN = 0, B_HAND, B_EYE, B_UNDO, B_REDO,
           B_VIEW, B_GRID, B_CUT, B_COPY, B_PASTE, B_HELP,
           B_FLIPH, B_FLIPV, B_MIRROR, B_FILL, B_SAVE, B_LOAD, B_DL,
           B_IMPORT, B_N };

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
    case B_FILL:  x = 3 * PITCH;             y = BARA_Y; break;
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
    // transfer block: [Save Load] over [DL Import], right-aligned (see enum Btn)
    case B_SAVE:  x = SCRW - ICON - PITCH;   y = BARC_Y; break;
    case B_LOAD:  x = SCRW - ICON;           y = BARC_Y; break;
    case B_DL:    x = SCRW - ICON - PITCH;   y = BARD_Y; break;
    case B_IMPORT:x = SCRW - ICON;           y = BARD_Y; break;
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
  { // B_FILL : paint bucket - a filled can (handle loop on top) tilted right, with
    // a paint drip running off the lower-right lip.
    0b0000001111000000,0b0000001001000000,0b0000001001000000,0b0000001111000000,
    0b0000011111100000,0b0000111111110000,0b0001111111111000,0b0001111111111000,
    0b0000111111111100,0b0000011111101110,0b0000001111000110,0b0000000110000100,
    0b0000000000000000,0b0000000000000000,0b0000000000000000,0b0000000000000000 },
  // The four transfer buttons form a 2x2 block (see btnPos): the *glyph* says
  // where the data goes (cloud on top = this device's cloud slot, tray at the
  // bottom = a file on this device) and the *arrow* points at that glyph for
  // "put" and away from it for "get". So cloud+up = save, cloud+down = load,
  // tray+down = download, tray+up = import -- no two of them share a silhouette.
  { // B_SAVE : cloud with an arrow rising INTO it (store to the cloud slot)
    0b0000011100000000,0b0001111111011000,0b0011111111111100,0b0111111111111110,
    0b0111111111111110,0b0111111111111110,0b0000000000000000,0b0000000110000000,
    0b0000001111000000,0b0000011111100000,0b0000111111110000,0b0001111111111000,
    0b0000001111000000,0b0000001111000000,0b0000000000000000,0b0000000000000000 },
  { // B_LOAD : cloud with an arrow dropping OUT of it (fetch the cloud slot)
    0b0000011100000000,0b0001111111011000,0b0011111111111100,0b0111111111111110,
    0b0111111111111110,0b0111111111111110,0b0000000000000000,0b0000001111000000,
    0b0000001111000000,0b0001111111111000,0b0000111111110000,0b0000011111100000,
    0b0000001111000000,0b0000000110000000,0b0000000000000000,0b0000000000000000 },
  // The two device-side buttons additionally carry a 3x5 word ("DL" / "UP") on
  // rows 0-4, which costs the arrow its shaft: rows 6-10 hold a bare triangle
  // and rows 11-13 the tray. Row 14-15 stay blank so the transfer block's grey
  // box (bottom edge on SCRH-1, i.e. row 15 of these buttons) clears the glyph.
  { // B_DL : "DL" over a triangle dropping INTO an open tray (PNG -> this device)
    0b0000011001000000,0b0000010101000000,0b0000010101000000,0b0000010101000000,
    0b0000011001110000,0b0000000000000000,0b0001111111111000,0b0000111111110000,
    0b0000011111100000,0b0000001111000000,0b0000000110000000,0b0110000000000110,
    0b0110000000000110,0b0111111111111110,0b0000000000000000,0b0000000000000000 },
  { // B_IMPORT : "UP" over a triangle rising OUT of the tray (local PNG -> editor)
    0b0000010101100000,0b0000010101010000,0b0000010101100000,0b0000010101000000,
    0b0000011101000000,0b0000000000000000,0b0000000110000000,0b0000001111000000,
    0b0000011111100000,0b0000111111110000,0b0001111111111000,0b0110000000000110,
    0b0110000000000110,0b0111111111111110,0b0000000000000000,0b0000000000000000 },
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
  uint8_t fillStk[VIEW * VIEW][2];            // flood-fill work stack (window-bounded)
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
  int  netPhase = 0;               // 0 idle; 1/2: paint status a frame before the blocking net op
  int  netOp = 0;                  // 0 = save, 1 = load
  int  msgTtl = 0;                 // frames left to show netMsg
  const char* netMsg = "";         // last result banner ("SAVED" / "LOADED" / ...)
  char slotKey[17] = "";           // this device's cloud slot key (16 alnum + NUL)
  int  dlPhase = 0;                // 0 idle, 1 streaming a local-download PNG
  int  dlOff = 0, dlLen = 0;       // bytes of the PNG (in g_png) sent / total
  bool dlSilent = false;           // this download is a hosted sync: no banner
  int  impPhase = IMP_IDLE;        // what the upload channel is busy with (IMP_*)
  bool impSilent = false;          // this import is the hosted startup pull
  // hosted mode (see pumpHost() and the file header)
  bool hostKnown = false;          // has the host answered a status poll yet?
  bool hosted = false;             // an embedding page owns our file I/O
  int  statWait = 0;               // frames the outstanding status poll has waited
  int  statTtl = 0;                // frames until the next status poll
  int  dirtyTtl = 0;               // frames until edits are pushed back (0 = clean)

  // push a copy of `src` onto a snapshot stack, dropping the oldest when full
  static void push(uint8_t* base, int& cnt, const uint8_t* src){
    if (cnt == UNDO_MAX){ memmove(base, base + SZ, SZ * (UNDO_MAX - 1)); cnt = UNDO_MAX - 1; }
    memcpy(base + cnt * SZ, src, SZ);
    ++cnt;
  }
  // The canvas changed: in hosted mode, restart the countdown to pushing it
  // back to the embedding page. Every frame of a drag re-arms it, so the push
  // happens once the user pauses rather than mid-stroke. No-op standalone.
  void touch(){ dirtyTtl = HOST_SYNC; }

  void beginStroke(){              // snapshot pre-edit state; invalidate redo
    push(reinterpret_cast<uint8_t*>(uStack), uCount, reinterpret_cast<uint8_t*>(canvas));
    rCount = 0;
    touch();
  }
  void doUndo(){
    if (uCount == 0) return;
    uint8_t* cur = reinterpret_cast<uint8_t*>(canvas);
    push(reinterpret_cast<uint8_t*>(rStack), rCount, cur);
    memcpy(cur, reinterpret_cast<uint8_t*>(uStack) + (--uCount) * SZ, SZ);
    touch();
  }
  void doRedo(){
    if (rCount == 0) return;
    uint8_t* cur = reinterpret_cast<uint8_t*>(canvas);
    push(reinterpret_cast<uint8_t*>(uStack), uCount, cur);
    memcpy(cur, reinterpret_cast<uint8_t*>(rStack) + (--rCount) * SZ, SZ);
    touch();
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
    touch();
  }
  // paint one logical pixel plus its mirror-mode reflection. The symmetry axis is
  // the centre of the visible 16x16 window, so the partner pixel sits at local
  // 15-x (left-right) or 15-y (up-down): canvas vx+15-(cx-vx) / vy+15-(cy-vy).
  // Both stay inside 0..CANVAS-1 because vx,vy <= VMAX and cx-vx,cy-vy are 0..15.
  void paint(int cx, int cy){
    canvas[cy][cx] = (uint8_t)sel;
    if      (mirror == 1) canvas[cy][vx + 15 - (cx - vx)] = (uint8_t)sel;
    else if (mirror == 2) canvas[vy + 15 - (cy - vy)][cx] = (uint8_t)sel;
    touch();                       // beginStroke() only fires on press, not per pixel
  }
  // flood-fill the 4-connected run of colour `from` reachable from (sx,sy),
  // recolouring it to `sel`. Bounded to the visible 16x16 window [vx,vx+15] x
  // [vy,vy+15] (Paint Bucket only acts on what is on screen). Cells are
  // recoloured as they are pushed, so each enters the stack at most once and
  // fillStk (VIEW*VIEW deep) can never overflow.
  void floodFill(int sx, int sy, uint8_t from){
    const uint8_t to = (uint8_t)sel;
    if (from == to) return;
    const int x0 = vx, y0 = vy, x1 = vx + VIEW - 1, y1 = vy + VIEW - 1;
    int sp = 0;
    canvas[sy][sx] = to;
    fillStk[sp][0] = (uint8_t)sx; fillStk[sp][1] = (uint8_t)sy; ++sp;
    while (sp > 0){
      const int x = fillStk[--sp][0], y = fillStk[sp][1];
      const int nx[4] = { x + 1, x - 1, x, x };
      const int ny[4] = { y, y, y + 1, y - 1 };
      for (int i = 0; i < 4; ++i){
        const int ax = nx[i], ay = ny[i];
        if (ax < x0 || ax > x1 || ay < y0 || ay > y1) continue;
        if (canvas[ay][ax] != from) continue;
        canvas[ay][ax] = to;
        fillStk[sp][0] = (uint8_t)ax; fillStk[sp][1] = (uint8_t)ay; ++sp;
      }
    }
  }
  // cut/copy/paste are overview-only; on a successful action flash the button (nudge +1,+1)
  void fireCut(){   if (!overview) return;             doCut();   fxId = B_CUT;   fxTtl = FX_FRAMES; }
  void fireCopy(){  if (!overview) return;             doCopy();  fxId = B_COPY;  fxTtl = FX_FRAMES; }
  void firePaste(){ if (!overview || !hasClip) return; doPaste(); fxId = B_PASTE; fxTtl = FX_FRAMES; }
  // flips work in both modes: they mirror the current 16x16 tile (the visible slice
  // in PIXEL mode, the white viewport box in OVERVIEW) -- always available.
  void fireFlipH(){ doFlipH(); fxId = B_FLIPH; fxTtl = FX_FRAMES; }
  void fireFlipV(){ doFlipV(); fxId = B_FLIPV; fxTtl = FX_FRAMES; }

  // Encode the canvas to an indexed PNG, base64url it, and push it to the global
  // cloud slot. Blocking (cloudsave::Set does the HTTP round-trips internally).
  // The two scratch buffers are static (~16.6KB + ~22KB) to stay off the stack.
  // Generate a fresh 16-char alnum cloud key. rnd() is already seeded from the
  // host real-time clock at startup (pico8 _reset), and we fold the clock in
  // again, so two fresh devices starting at different times never collide.
  void genKey(char out[17]){
    static const char AB[] =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"; // 62
    srand(rndu() ^ (u32)B8_INF_CAL_L ^ ((u32)B8_INF_CAL_H << 1));
    for (int i = 0; i < 16; ++i) out[i] = AB[rndu() % 62];
    out[16] = 0;
  }
  // Resolve this device's cloud key from savedata, generating + persisting one
  // on first run. Always yields a valid cloudsave key (^[0-9A-Za-z]{16}$).
  void ensureSlotKey(){
    char buf[24] = {0};
    const int n = sd_get(kSlotField, buf, sizeof(buf));
    bool valid = (n == 16);
    for (int i = 0; i < n && valid; ++i){
      const char c = buf[i];
      valid = (c>='0'&&c<='9') || (c>='a'&&c<='z') || (c>='A'&&c<='Z');
    }
    if (valid){ memcpy(slotKey, buf, 16); slotKey[16] = 0; return; }
    genKey(slotKey);
    sd_set(kSlotField, slotKey);
  }

  bool doSave(){
    const int n = png::EncodeIndexed(&canvas[0][0], CANVAS, CANVAS,
                                     png::kPico8Palette, 16, g_png, sizeof(g_png));
    if (n <= 0) return false;
    const int m = base64::encode(g_png, n, g_b64, sizeof(g_b64));
    if (m <= 0) return false;
    return cloudsave::Set(slotKey, g_b64, m);
  }

  // Fetch the cloud slot, decode the PNG, and replace the canvas. Blocking.
  // Returns 1 on success, 0 if the slot is empty, -1 on transport/decode error.
  int doLoad(){
    const int m = cloudsave::Get(slotKey, g_b64, sizeof(g_b64));
    if (m == 0) return 0;                     // slot never set
    if (m <  0) return -1;                     // bad key / transport error
    const int n = base64::decode(g_b64, m, g_png, sizeof(g_png));
    if (n <= 0) return -1;
    int w = 0, h = 0;
    const int px = png::DecodeIndexed(g_png, n, g_tmp, sizeof(g_tmp), &w, &h,
                                      g_raw, sizeof(g_raw));
    if (px <= 0 || w != CANVAS || h != CANVAS) return -1;
    beginStroke();                             // make the load undoable
    memcpy(canvas, g_tmp, SZ);
    return 1;
  }

  // Local download (browser Save-As) is streamed across frames to respect the
  // 8KB SCI FIFO. startDownload() encodes the PNG and arms the pump;
  // pumpDownload() emits the "<len>\n" header then one <=DL_CHUNK chunk per
  // frame until the whole PNG is sent (host vdownload.js then offers the file).
  // `silent` marks a hosted background sync: same bytes over the same channel,
  // but no "DOWNLOAD..." / "DOWNLOADED" banner, because the user did not ask
  // for a download -- they just stopped drawing for half a second.
  void startDownload(bool silent = false){
    if (dlPhase != 0 || netPhase != 0 || impPhase != IMP_IDLE) return;
    const int n = png::EncodeIndexed(&canvas[0][0], CANVAS, CANVAS,
                                     png::kPico8Palette, 16, g_png, sizeof(g_png));
    if (n <= 0) { netMsg = "DL FAILED"; msgTtl = 90; return; }
    dlLen = n; dlOff = 0; dlPhase = 1; dlSilent = silent;
  }
  void pumpDownload(){
    if (dlPhase == 0) return;
    download::Reset();
    FILE* fp = fopen("/download/con0", "wb");
    if (fp) {
      if (dlOff == 0) {                        // first frame: the "<len>\n" header
        char hdr[16];
        const int hl = snprintf(hdr, sizeof(hdr), "%d\n", dlLen);
        fwrite(hdr, 1, hl, fp);
      }
      int n = dlLen - dlOff;
      if (n > DL_CHUNK) n = DL_CHUNK;
      fwrite(g_png + dlOff, 1, n, fp);
      fclose(fp);
      dlOff += n;
    } else {
      dlOff = dlLen;                           // driver unavailable -> give up
    }
    if (dlOff >= dlLen) {
      dlPhase = 0;
      if (!dlSilent) { netMsg = "DOWNLOADED"; msgTtl = 90; }
      dlSilent = false;
    }
  }

  // ---- hosted mode ---------------------------------------------------------
  // Standalone, the two local-file buttons are the whole story: the user says
  // when a file is read or written. Inside a page that owns the file, that
  // question is already answered -- the sprite sheet on screen IS the file --
  // so the ROM moves the bytes itself and the buttons disappear.
  //
  // Everything here rides one request/response channel that only the ROM may
  // start, so this is a small state machine rather than an event handler:
  //   boot        -> "stat": are we hosted? (a host too old to answer times out)
  //   hosted      -> pull the page's sheet in once, then poll "stat" while idle
  //   after edits -> push the canvas back HOST_SYNC frames after the last one
  //   on request  -> the page raises STAT_FLUSH when it needs the bytes now
  //                  (leaving the editor, or building the ROM), which just
  //                  collapses the countdown to "next idle frame".
  void pumpHost(){
    if (impPhase == IMP_STAT) {
      int n = 0;
      const upload::State st = upload::Poll(g_stat, sizeof(g_stat), &n);
      if (st == upload::DONE && n == 1) {
        const int flags   = g_stat[0];
        const bool first  = !hostKnown;
        hostKnown = true;
        hosted    = (flags & upload::STAT_HOSTED) != 0;
        impPhase  = IMP_IDLE;
        if (first && hosted) {                 // open the page's sheet at startup
          upload::Begin(); impPhase = IMP_FILE; impSilent = true;
        } else if (flags & upload::STAT_FLUSH) {
          dirtyTtl = 1;                        // push on the next idle frame
        }
      } else if (st != upload::WAITING || ++statWait > HOST_STAT_WAIT) {
        impPhase = IMP_IDLE;                   // silent or confused host: standalone
        hostKnown = true;
      }
      return;
    }
    if (!hosted) return;                       // standalone: nothing to sync
    if (impPhase != IMP_IDLE || netPhase != 0 || dlPhase != 0) return;
    if (dirtyTtl > 1) { --dirtyTtl; return; }  // still settling after an edit
    if (dirtyTtl == 1) { dirtyTtl = 0; startDownload(true); return; }
    if (--statTtl <= 0) {
      statTtl = HOST_POLL; statWait = 0;
      upload::Stat(); impPhase = IMP_STAT;
    }
  }

  // Nearest PICO-8 color to an arbitrary RGB (squared-distance), used to snap an
  // imported PNG's palette onto our fixed 16 colors.
  static uint8_t nearestPico8(int r, int g, int b){
    int best = 0, bestd = 1 << 30;
    for (int i = 0; i < 16; ++i){
      const int dr = r - png::kPico8Palette[i*3];
      const int dg = g - png::kPico8Palette[i*3+1];
      const int db = b - png::kPico8Palette[i*3+2];
      const int d  = dr*dr + dg*dg + db*db;
      if (d < bestd){ bestd = d; best = i; }
    }
    return (uint8_t)best;
  }

  // Import a local PNG (already in g_png, n bytes) into the canvas: decode the
  // indices, snap its PLTE to the nearest PICO-8 colors, and blit it centered on
  // a cleared canvas (undoable). Returns 1 ok, -2 too big (>128), -1 on failure.
  int doImport(int n){
    int w = 0, h = 0;
    const int px = png::DecodeIndexed(g_imp, n, g_tmp, sizeof(g_tmp), &w, &h,
                                      g_raw, sizeof(g_raw));
    if (px <= 0) return -1;
    if (w > CANVAS || h > CANVAS) return -2;
    static uint8_t pal[256 * 3];
    const int pc = png::GetPalette(g_imp, n, pal, sizeof(pal));
    if (pc <= 0) return -1;
    uint8_t lut[256];
    for (int i = 0; i < pc;  ++i) lut[i] = nearestPico8(pal[i*3], pal[i*3+1], pal[i*3+2]);
    for (int i = pc; i < 256; ++i) lut[i] = 0;
    beginStroke();                             // make the import undoable
    memset(canvas, 0, sizeof(canvas));
    const int ox = (CANVAS - w) / 2, oy = (CANVAS - h) / 2;
    for (int y = 0; y < h; ++y)
      for (int x = 0; x < w; ++x)
        canvas[oy + y][ox + x] = lut[ g_tmp[y * w + x] ];
    return 1;
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

  // The local-file row has no meaning when a page owns the file: it is neither
  // drawn nor tappable there (the sync is automatic -- see pumpHost()).
  bool btnHidden(int id) const { return hosted && (id == B_DL || id == B_IMPORT); }

  void dispatch(int btn){
    switch (btn) {
      case B_PEN: case B_HAND: case B_EYE: case B_FILL: tool = btn; break;
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
      case B_SAVE:  if (netPhase == 0 && dlPhase == 0 && impPhase == IMP_IDLE) { netOp = 0; netPhase = 1; } break;
      case B_LOAD:  if (netPhase == 0 && dlPhase == 0 && impPhase == IMP_IDLE) { netOp = 1; netPhase = 1; } break;
      case B_DL:    startDownload(); break;   // stream the PNG to a local file
      case B_IMPORT:                          // pull a local PNG file into the canvas
        if (netPhase == 0 && dlPhase == 0 && impPhase == IMP_IDLE) { upload::Begin(); impPhase = IMP_FILE; }
        break;
    }
  }

  void _init() override {
    memset(canvas, 0, sizeof(canvas));
    uCount = rCount = 0;
    hasClip = false;
    mirror = 0;
    vx = vy = 0;                   // start on the top-left tile (0,0)-(15,15)
    ensureSlotKey();               // resolve/create this device's cloud slot key
    // Ask the host how file I/O is wired before drawing anything: if a page
    // owns the file, the first thing to do is open it (see pumpHost()).
    upload::Stat(); impPhase = IMP_STAT; statWait = 0;
  }

  void _update() override {
    pollKeys();
    if (fxTtl > 0) --fxTtl;                 // fade the copy/paste press feedback
    if (msgTtl > 0) --msgTtl;               // fade the save-result banner

    // Save/Load are two-phase so the "SAVING..." / "LOADING..." banner paints for
    // a frame before we block on the HTTP round-trip: phase 1 arms it, phase 2
    // (next frame) runs the blocking op.
    if (netPhase == 1) {
      netPhase = 2;
    } else if (netPhase == 2) {
      if (netOp == 0) {
        netMsg = doSave() ? "SAVED" : "SAVE FAILED";
      } else {
        const int r = doLoad();
        netMsg = (r > 0) ? "LOADED" : (r == 0 ? "NO DATA" : "LOAD FAILED");
      }
      msgTtl  = 90;
      netPhase = 0;
    }
    pumpDownload();     // stream a pending local download, one chunk per frame

    // PNG import: after B_IMPORT (or the hosted startup pull) arms
    // upload::Begin(), drain the host's reply across frames; when the whole
    // file has arrived, decode + remap it. A hosted startup pull is silent
    // unless it fails -- the page opening its own file is not news.
    if (impPhase == IMP_FILE) {
      int n = 0;
      const upload::State st = upload::Poll(g_imp, sizeof(g_imp), &n);
      if (st == upload::DONE) {
        const int r = doImport(n);
        if (r != 1 || !impSilent) {
          netMsg = (r == 1) ? "IMPORTED" : (r == -2) ? "TOO BIG (<=128)" : "IMPORT FAILED";
          msgTtl = 90;
        }
        impPhase = IMP_IDLE; impSilent = false;
        dirtyTtl = 0;                    // just took the page's copy: not dirty
      } else if (st == upload::CANCELLED) {
        if (!impSilent) { netMsg = "CANCELLED"; msgTtl = 90; }
        impPhase = IMP_IDLE; impSilent = false;
      } else if (st == upload::ERROR) {
        netMsg = "IMPORT FAILED"; msgTtl = 90;
        impPhase = IMP_IDLE; impSilent = false;
      }
    }
    pumpHost();         // hosted mode: startup pull, idle polls, edit push-back

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
            case B_FILL:                              // paint bucket: fill on tap only
              if (press && canvas[cy][cx] != (uint8_t)sel) {
                beginStroke();
                floodFill(cx, cy, canvas[cy][cx]);
              }
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
            if (btnHidden(id)) continue;
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
    if (overview) rect(vx - 1, vy - 1, vx + VIEW + 1, vy + VIEW + 1, WHITE);

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
      rect(sx, sy, sx + PAL_SW, sy + PAL_SW, WHITE);
    }

    // toolbar buttons (positions via btnPos). fg = BLACK when active/available,
    // LIGHT_GREY when inactive/disabled; the tool trio shows the current tool black.
    for (int id = 0; id < B_N; ++id) {
      if (btnHidden(id)) continue;             // hosted: no local-file row
      Color fg = BLACK;
      switch (id) {
        case B_PEN: case B_HAND: case B_EYE: fg = (id == tool) ? BLACK : LIGHT_GREY; break;
        // Fill is disabled in OVERVIEW (view-only): always grey there; else it is a
        // normal tool-trio member (black when selected).
        case B_FILL:  fg = overview ? LIGHT_GREY : ((id == tool) ? BLACK : LIGHT_GREY); break;
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
        case B_SAVE: case B_LOAD: case B_DL: case B_IMPORT:
          // Grey while a net/download/import op the user started is running.
          // Hosted background syncs are excluded: they fire every time drawing
          // settles, and blinking the buttons four times a minute would read as
          // a glitch. (A tap landing inside those few frames is dropped by
          // dispatch()'s guard, same as before -- it just is not advertised.)
          if (netPhase != 0 || (dlPhase != 0 && !dlSilent)
              || (impPhase == IMP_FILE && !impSilent)) fg = LIGHT_GREY;
          break;
      }
      int bx, by; btnPos(id, bx, by);
      const bool pressed = (fxTtl > 0 && fxId == id);   // cut/copy/paste tap flash
      // Mirror shows the L/R glyph for off & left-right, the rotated one for up-down.
      const uint16_t* bits = (id == B_MIRROR && mirror == 2) ? kIconMirrorV : kIcon[id];
      drawButton(bx, by, fg, pressed, bits);
    }
    // light-grey box grouping the four mutually-exclusive tools (drawn over the
    // button panels so its left edge stays visible at column 0). Top edge nudged
    // up 1px (BARA_Y-2) for a touch more breathing room above the icons.
    rect(0, BARA_Y - 2, 3 * PITCH + ICON + 1, BARA_Y + ICON + 1, LIGHT_GREY);
    // light-grey box grouping the 2x2 transfer block (cloud row over file row).
    // rect() is exclusive like rectfill(), so its right/bottom edges land on
    // columns/rows x1-1 / y1-1: passing SCRW/SCRH puts them on the last
    // on-screen column/row. Both land on pixels the 16x16 glyphs leave blank,
    // so the box never cuts into an icon. Hosted, the file row is gone, so the
    // box closes up around the cloud row alone.
    rect(SCRW - ICON - PITCH - 2, BARC_Y - 2, SCRW,
         hosted ? (BARC_Y + ICON + 2) : SCRH, LIGHT_GREY);

    // net status banner, drawn on top of the edit area: "SAVING..."/"LOADING..."
    // while the blocking op is pending, then the result for msgTtl frames.
    // A hosted background sync (silent download / status poll / startup pull)
    // deliberately shows nothing: it is not an operation the user started.
    const bool busyShown = (netPhase != 0) || (dlPhase != 0 && !dlSilent)
                        || (impPhase == IMP_FILE && !impSilent);
    if (busyShown || msgTtl > 0) {
      const char* t = (netPhase != 0) ? (netOp == 0 ? "SAVING..." : "LOADING...")
                    : (dlPhase  != 0 && !dlSilent)  ? "DOWNLOAD..."
                    : (impPhase == IMP_FILE && !impSilent) ? "IMPORT..."
                    : netMsg;
      rectfill(0, 54, SCRW, 74, BLACK);
      rect(0, 54, SCRW, 74, WHITE);
      sprint(30, 60, WHITE, t);
    }
  }
};

static PixArt* app;

int main(){
  app = new PixArt;
  app->run();
  return 0;
}
