/*
  Simple sample of PICO-8 LikeLibrary for C/C++.

  A yellow round-faced character (Foo) walks randomly.
  You can also operate it with the PC keyboard arrow keys.
  On smartphones, you cannot operate it because there is no keyboard.
*/
#include <pico8.h>

using namespace std;  
using namespace pico8;  

/*
  Define constants for sprite pattern indices.
  The sprite patterns are stored in ./data/import/sprite0.png as
  a 128x128 dot image (16x16 tiles).
*/
static  constexpr u8  SPR_FOO       = 16;
static  constexpr u8  SPR_DRAGONFLY = 19;
static  constexpr u8  SPR_WALL0     = 32;
static  constexpr u8  SPR_WALL1     = 33;
static  constexpr u8  SPR_WALL2     = 34;
static  constexpr u8  SPR_COIN      = 35;
static  constexpr u8  SPR_FLOWER    = 39;
static  constexpr u8  SPR_CLOUD     = 48;

/*
  The yellow coin blinking and shadows are implemented using
  palette animations. BEEP-8 provides 16 palettes (#0–#15),
  of which #0–#3 are shared between the BG and SPRITE layers.
*/
static  constexpr u8  PAL_COIN_BLINK = 3;
static  constexpr u8  PAL_SHADOW     = 4;

/*
  Define the shadow palette: setting all colors to 1 (dark blue)
  makes everything appear as a shadow color.
*/
static  constexpr array<unsigned char, 16> palette_shadow = {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

extern  const uint8_t  b8_image_sprite0[];
extern  const uint8_t  b8_image_sprite1[];

/*
  Flag definitions for patterns using fset()/fget().
  Flags such as "these patterns are walls", "these patterns
  are fruits", and "these patterns are coins" are set once
  in init().
*/
enum  BgFlag {
  FLAG_WALL,
  FLAG_FRUITE,
  FLAG_COIN
};

static  constexpr u8  N_CLOUD = 4;

// Work memory variables, initialized in init().

// 2D coordinates passed to camera(). Vec is a 2D vector type.
static  Vec cam;

// Foo's position
static  Vec pos_foo;
// Foo's velocity
static  Vec v_foo;

// Dragonfly position
static  Vec pos_dragon_fly;

// Mouse position in the previous frame
static  Vec prev_mouse;
// Mouse movement velocity based on the difference between
// previous and current frame mouse positions
static  Vec v_mouse;

// Frame counter incremented by 1 in update() (1/60s)
static  int frame = 0;

static  s16 cnt_coin_disp = -1;
static  s16 cnt_coin = 0;

// Cloud work structure
struct  Cloud {
  Vec pos;
  Vec velocity;

  void step() {
    constexpr fx8 range(256);
    pos += velocity;
    if (pos.x > range) pos.x -= range;
    if (pos.y > range) pos.y -= range;
  }
};
static  Cloud clouds[N_CLOUD];

static  void  checkWallCollisionXY(Vec& pos);
static  void  blinkCoin();

// Equivalent to PICO-8's _init()
static  void  init() {
  // Load the sprite image stored as a C array in ./data/export/sprite0.png.cpp
  // sprite0.png.cpp is generated from ./data/import/sprite0.png when you run make.
  lsp(0, b8_image_sprite0);

  // Load the sprite image stored as a C array in ./data/export/sprite1.png.cpp
  lsp(1, b8_image_sprite1);

  // Initialize the BG layer (32 tiles x 32 tiles).
  // Each tile is 8x8 pixels.
  mapsetup(TILES_32, TILES_32);

  // Clear the BG layer to tile 0.
  b8PpuBgTile tile = {};
  mcls(tile);

  // Fill the BG layer randomly.
  for (int nn = 0; nn < 100; ++nn) {
    tile.XTILE = rand() % 12;
    tile.YTILE = 2;
    tile.PAL   = (tile.XTILE == 3) ? PAL_COIN_BLINK : 0;
    msett(rand() & 31, rand() & 31, tile);
  }

  // Initialize Foo's position.
  pos_foo.set(64, 64);

  pos_dragon_fly = pos_foo + Vec(0, 73);
  prev_mouse     = Vec(mousex(), mousey());

  // Set flags for each sprite pattern (BG patterns)
  fset(SPR_WALL0, FLAG_WALL, 1);
  fset(SPR_WALL1, FLAG_WALL, 1);
  fset(SPR_WALL2, FLAG_WALL, 1);
  fset(SPR_COIN,  FLAG_COIN, 1);
  for (u8 ii = SPR_FLOWER; ii < 48; ++ii) fset(ii, FLAG_FRUITE, 1);

  // Initialize cloud work memory (position and velocity)
  // using random values.
  for (auto& cloud : clouds) {
    cloud.pos.set(
      pico8::rnd(256),
      pico8::rnd(256)
    );
    cloud.velocity.set(
      fx8(2,10) + pico8::rnd(fx8(2,30)),
      fx8(1,10)
    );
  }
}

static  void  update() {
  ++frame;

  // Calculate the target marker from mouse coordinates
  // and store it in pos_dragon_fly.
  const Vec mpos(mousex(), mousey());
  v_mouse = mpos - prev_mouse;
  v_mouse.x = pico8::clamp(-10, v_mouse.x, +10);
  v_mouse.y = pico8::clamp(-10, v_mouse.y, +10);
  prev_mouse = mpos;
  pos_dragon_fly += v_mouse;

  // Compute direction from Foo to the dragonfly.
  const Vec lv = pos_dragon_fly - pos_foo;
  const fx8 dir = pico8::atan2(lv.y, lv.x);

  fx8 velocity(0);
  if (pos_foo.distanceTo(pos_dragon_fly) > 5) {
    // fx8() constructs an 8-bit fixed-point number from a fraction;
    // fx8(55,100) represents 55/100 = 0.55.
    velocity = fx8(55,100);
  }
  v_foo.set(
    velocity * pico8::cos(dir),
    velocity * pico8::sin(dir)
  );

  // btn() retrieves keyboard button states in a PC environment.
  if (btn(BUTTON_LEFT))  v_foo.x = -1;
  if (btn(BUTTON_RIGHT)) v_foo.x = +1;
  if (btn(BUTTON_UP))    v_foo.y = -1;
  if (btn(BUTTON_DOWN))  v_foo.y = +1;
  pos_foo += v_foo;

  // Clamp Foo's position within the BG area assumed to be 32x32 tiles.
  pos_foo.x = pico8::clamp(pos_foo.x, 0, TILES_32*8);
  pos_foo.y = pico8::clamp(pos_foo.y, 0, TILES_32*8);

  // Check collision between Foo's position and
  // BG tiles with the WALL attribute.
  checkWallCollisionXY(pos_foo);

  // Check collision between Foo's position and fruit/coin tiles.
  for (int sx = -3; sx <= +3; sx += 6) {
    for (int sy = -3; sy <= +3; sy += 6) {
      // Compute xt, yt to determine which BG tile Foo is on;
      // each tile is 8×8 dots.
      const u32 xt = ((pos_foo.x + sx) >> 3) & (TILES_32 - 1);
      const u32 yt = ((pos_foo.y + sy) >> 3) & (TILES_32 - 1);

      // mget() is PICO-8-compatible: retrieves the pattern ID at the tile coordinates.
      const u16 tile = mget(xt, yt);

      // fget() is PICO-8-compatible: retrieves flags set by fset() on the pattern.
      if (fget(tile, FLAG_FRUITE)) mset(xt, yt, 0);
      if (fget(tile, FLAG_COIN)) {
        mset(xt, yt, 0);
        ++cnt_coin;
      }
    }
  }

  // Update cloud states.
  for (auto& cloud : clouds) {
    cloud.step();
  }
}

static  void  draw() {
  // Enable or disable the debug string output via dprint().
  dprintenable(false);

  // Initialize the camera state.
  camera();

  // Clear the entire screen with GREEN.
  cls(DARK_GREEN);

  // Applies depth setting to all subsequent draw calls:
  // 0 is frontmost, maxz() is backmost.
  setz(maxz());

  // Palette animation to make the coin sparkle.
  blinkCoin();

  // Set the camera position to Foo's position minus (64,100).
  cam = pos_foo - Vec(64,100);
  camera(cam.x, cam.y);

  // Draw the BG layer (MAP) with the camera position.
  map(cam.x, cam.y, BG_0);

  // Set depth to the foreground.
  setz(maxz()/2);

  // Set the palette.
  const u8 palsel = 1;
  pal(WHITE, BLACK, palsel);

  // Draw the yellow round-faced Foo sprite.
  spr(SPR_FOO, pos_foo.x-4, pos_foo.y-4, 1, 1, v_foo.x < 0, false);

  if (cnt_coin_disp != cnt_coin) {
    cnt_coin_disp = cnt_coin;
    cursor(0,0);
    print("Coin:%d", cnt_coin_disp);
  }

  // Perform a debug print every 32 frames.
  // Enable by setting dprintenable(true).
  if (!(frame & 31)) dprint("frame=%d\n", frame);

  spr(SPR_DRAGONFLY + ((frame>>1)&1), pos_dragon_fly.x-4, pos_dragon_fly.y-4-20);
  setz(maxz() - 1);

  // Set the palette for shadows.
  setpal(PAL_SHADOW, palette_shadow);
  if (frame & 1)
    spr(SPR_DRAGONFLY + ((frame>>1)&1), pos_dragon_fly.x-4, pos_dragon_fly.y-4, 1, 1, false, false, PAL_SHADOW);

  scursor(pos_dragon_fly.x+8-cam.x, pos_dragon_fly.y-cam.y-20, YELLOW);
  sprint("%d %d", (int)(pos_dragon_fly.x-4+8), (int)(pos_dragon_fly.y-4-20));

  // Draw a small lake.
  circfill(-30, 77, 50, BLUE);

  // Draw a desert.
  circfill(98, 350, 50, LIGHT_PEACH);

  // Cloud rendering
  for (u8 ii = 0; ii < N_CLOUD; ++ii) {
    setz(1); // Set to frontmost for clouds
    sprb(0, SPR_CLOUD,
      clouds[ii].pos.x,
      clouds[ii].pos.y - 33,
      2, 2
    );

    // Represent rain falling from clouds with blue lines
    setz(2);
    const auto x = clouds[ii].pos.x + rnd(16);
    const auto y = clouds[ii].pos.y + 7 - 33 + rnd(16);
    const auto len = rnd(25);
    line(x, y, x, y+len, BLUE);

    // Draw the cloud shadows with blinking every other frame
    if ((ii + frame) & 1) continue;

    setz(maxz() - 1);
    sprb(0, SPR_CLOUD,
      clouds[ii].pos.x,
      clouds[ii].pos.y,
      2, 2,
      false, false, PAL_SHADOW
    );
  }
}

// Determine walls on the BG layer
void checkWallCollisionXY(Vec& pos) {
  constexpr std::pair<int,int> offs[] = {
    { +4,  0 }, { -4,  0 }, {  0, +4 }, {  0, -4 }
  };

  for (auto [ox, oy] : offs) {
    const u32 xt = ((pos.x + ox) >> 3) & (TILES_32 - 1);
    const u32 yt = ((pos.y + oy) >> 3) & (TILES_32 - 1);
    const u16 tile = mget(xt, yt);

    if (fget(tile, FLAG_WALL)) {
      if (ox != 0) {
        pos.x = (ox > 0)
          ? pico8::min(pos.x, (xt << 3) - 4)
          : pico8::max(pos.x, ((xt + 1) << 3) + 4);
      } else {
        pos.y = (oy > 0)
          ? pico8::min(pos.y, (yt << 3) - 4)
          : pico8::max(pos.y, ((yt + 1) << 3) + 4);
      }
    }
  }
}

// Palette animation data for coin blinking
constexpr size_t PaletteCount = 3;
constexpr size_t PaletteSize  = 16;
const array<array<unsigned char, PaletteSize>, PaletteCount> palettes = {{
  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
  {0,1,2,3,4,5,6,10,8,4,9,11,12,13,14,15},
  {0,1,2,3,4,5,6,9,8,9,4,11,12,13,14,15},
}};
static const u8 idx_coin[8] = {0,0,0,0,1,2,2,1};
static void blinkCoin() {
  const u8 idx = idx_coin[(frame>>2)&7];
  const auto& pal = palettes[idx];
  setpal(PAL_COIN_BLINK, pal);
}

class Pico8App : public Pico8 {
  // Initialization function called only once at startup.
  void _init() override {
    // In this sample we call init() for a C-style implementation,
    // but implementing directly in this member function _init()
    // would be more C++-style.
    init();
  }

  // Called every 1/60th of a second.
  // Only internal status updates are allowed.
  // Drawing operations are not allowed here.
  void _update() override {
    update();
  }

  // Called every 1/60th of a second.
  // Perform drawing of internal status here.
  void _draw() override {
    draw();
  }

public:
  virtual ~Pico8App(){}
};

// main() for C/C++ language.
// Magic incantation to run the PICO-8 library.
int main() {
  Pico8App app;
  app.run();  // ::run() enters an infinite loop.
  return 0;
}

