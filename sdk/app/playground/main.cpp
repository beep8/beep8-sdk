#include <pico8.h>
using namespace pico8;

static const int FP = 256;
static const int SCRW = 128;
static const int SCRH = 240;

struct Marble {
  int x, y, vx, vy;
  bool active;
};

struct Bug {
  int x, y, vx, vy;
  bool active;
  int type;     // 0,1,2 color variants
  int wing;     // wing animation counter
  int wob;      // wobble phase
};

struct Spark {
  int x, y, vx, vy, life;
  Color col;
  bool active;
};

class App : public Pico8 {
public:
  Marble marbles[16];
  Bug bugs[8];
  Spark sparks[32];

  bool dragging;
  int startX, startY;   // pixel
  int curX, curY;       // pixel (fp not needed)
  int prevX, prevY;
  int smVX, smVY;       // smoothed drag velocity (pixels/frame)
  int score;
  int frame;
  int spawnTimer;

  void _init() override {
    for (int i = 0; i < 16; i++) marbles[i].active = false;
    for (int i = 0; i < 8; i++) bugs[i].active = false;
    for (int i = 0; i < 32; i++) sparks[i].active = false;
    dragging = false;
    smVX = smVY = 0;
    score = 0;
    frame = 0;
    spawnTimer = 0;
    // initial bugs
    for (int i = 0; i < 3; i++) spawnBug();
  }

  void spawnBug() {
    for (int i = 0; i < 8; i++) {
      if (!bugs[i].active) {
        Bug &b = bugs[i];
        b.active = true;
        b.type = rndi(3);
        bool fromLeft = rndi(2) == 0;
        b.x = (fromLeft ? -10 : SCRW + 10) * FP;
        b.y = (15 + rndi(70)) * FP;
        int sp = (FP / 3) + rndi(FP / 2);
        b.vx = fromLeft ? sp : -sp;
        b.vy = 0;
        b.wing = 0;
        b.wob = rndi(256);
        return;
      }
    }
  }

  void launchMarble(int px, int py, int vx, int vy) {
    for (int i = 0; i < 16; i++) {
      if (!marbles[i].active) {
        marbles[i].active = true;
        marbles[i].x = px * FP;
        marbles[i].y = py * FP;
        marbles[i].vx = vx;
        marbles[i].vy = vy;
        return;
      }
    }
  }

  void spawnSparks(int x, int y, Color col) {
    int made = 0;
    for (int i = 0; i < 32 && made < 8; i++) {
      if (!sparks[i].active) {
        sparks[i].active = true;
        sparks[i].x = x;
        sparks[i].y = y;
        sparks[i].vx = (rndi(2 * FP) - FP);
        sparks[i].vy = (rndi(2 * FP) - FP);
        sparks[i].life = 15 + rndi(10);
        sparks[i].col = col;
        made++;
      }
    }
  }

  void _update() override {
    frame++;

    // --- touch / drag handling ---
    bool down = btn(BUTTON_MOUSE_LEFT);
    int mx = mousex();
    int my = mousey();

    if (down) {
      if (!dragging) {
        // start
        dragging = true;
        startX = mx; startY = my;
        prevX = mx; prevY = my;
        smVX = 0; smVY = 0;
      } else {
        int dvx = mx - prevX;
        int dvy = my - prevY;
        // smoothed (weighted toward recent motion)
        smVX = (smVX + dvx * 3) / 4;
        smVY = (smVY + dvy * 3) / 4;
        prevX = mx; prevY = my;
      }
      curX = mx; curY = my;
    } else {
      if (dragging) {
        // release -> launch based on trajectory
        dragging = false;
        int vx = smVX * FP * 5 / 2;
        int vy = smVY * FP * 5 / 2;
        // require some flick
        int mag = (vx < 0 ? -vx : vx) + (vy < 0 ? -vy : vy);
        if (mag > FP) {
          launchMarble(curX, curY, vx, vy);
        }
      }
    }

    // --- update marbles ---
    for (int i = 0; i < 16; i++) {
      Marble &m = marbles[i];
      if (!m.active) continue;
      m.vy += 10; // gravity
      m.x += m.vx;
      m.y += m.vy;
      int px = m.x / FP;
      int py = m.y / FP;
      if (px < -16 || px > SCRW + 16 || py < -32 || py > SCRH + 16) {
        m.active = false;
        continue;
      }
      // collide with bugs
      for (int j = 0; j < 8; j++) {
        Bug &b = bugs[j];
        if (!b.active) continue;
        int dx = px - b.x / FP;
        int dy = py - b.y / FP;
        if (dx * dx + dy * dy < 64) {
          b.active = false;
          m.active = false;
          score += 10;
          Color cols[3] = {GREEN, ORANGE, LAVENDER};
          spawnSparks(b.x, b.y, cols[b.type]);
          break;
        }
      }
    }

    // --- update bugs ---
    for (int i = 0; i < 8; i++) {
      Bug &b = bugs[i];
      if (!b.active) continue;
      b.wing++;
      b.wob += 6;
      b.x += b.vx;
      // gentle vertical wobble
      int s = b.wob & 255;
      int wobv = ((s < 128) ? (s - 64) : (192 - s));
      b.y += wobv / 8;
      // keep in vertical band
      if (b.y < 10 * FP) b.y = 10 * FP;
      if (b.y > 95 * FP) b.y = 95 * FP;
      int px = b.x / FP;
      if (px < -20 || px > SCRW + 20) b.active = false;
    }

    // --- update sparks ---
    for (int i = 0; i < 32; i++) {
      Spark &s = sparks[i];
      if (!s.active) continue;
      s.x += s.vx;
      s.y += s.vy;
      s.vy += 12;
      s.life--;
      if (s.life <= 0) s.active = false;
    }

    // --- spawn bugs over time ---
    spawnTimer++;
    int interval = 70;
    if (spawnTimer > interval) {
      spawnTimer = 0;
      spawnBug();
    }
  }

  void drawBug(const Bug &b) {
    int x = b.x / FP;
    int y = b.y / FP;
    Color bodyc, wingc;
    switch (b.type) {
      case 0: bodyc = DARK_GREEN; wingc = GREEN; break;
      case 1: bodyc = BROWN; wingc = ORANGE; break;
      default: bodyc = DARK_PURPLE; wingc = LAVENDER; break;
    }
    bool up = (b.wing & 4) != 0;
    int wy = up ? -3 : 1;
    // wings
    circ(x - 4, y + wy, 3, wingc);
    circ(x + 4, y + wy, 3, wingc);
    // body
    rectfill(x - 2, y - 3, x + 2, y + 3, bodyc);
    circ(x, y - 4, 2, bodyc);
    // eyes
    pset(x - 1, y - 5, WHITE);
    pset(x + 1, y - 5, WHITE);
  }

  void _draw() override {
    cls(DARK_BLUE);

    // sky gradient hint
    rectfill(0, 0, SCRW, 10, BLACK);
    // ground line at bottom
    rectfill(0, SCRH - 14, SCRW, SCRH, DARK_GREEN);
    rectfill(0, SCRH - 14, SCRW, SCRH - 12, GREEN);

    // bugs
    for (int i = 0; i < 8; i++)
      if (bugs[i].active) drawBug(bugs[i]);

    // marbles
    for (int i = 0; i < 16; i++) {
      if (!marbles[i].active) continue;
      int x = marbles[i].x / FP;
      int y = marbles[i].y / FP;
      circ(x, y, 3, LIGHT_PEACH);
      pset(x - 1, y - 1, WHITE);
    }

    // sparks
    for (int i = 0; i < 32; i++) {
      if (!sparks[i].active) continue;
      pset(sparks[i].x / FP, sparks[i].y / FP, sparks[i].col);
    }

    // aiming feedback while dragging
    if (dragging) {
      circ(curX, curY, 4, PINK);
      // predicted launch direction
      int vx = smVX * 5 / 2;
      int vy = smVY * 5 / 2;
      int ex = curX, ey = curY;
      int sx = curX * FP, sy = curY * FP;
      int pvx = vx * FP * 5 / 2;
      int pvy = vy * FP * 5 / 2;
      // draw dotted trajectory preview
      for (int t = 1; t <= 12; t++) {
        sx += pvx;
        sy += pvy;
        pvy += 10;
        ex = sx / FP; ey = sy / FP;
        if (ex < 0 || ex > SCRW || ey < 0 || ey > SCRH) break;
        if (t % 2 == 0) pset(ex, ey, YELLOW);
      }
    }

    // HUD
    scursor(4, 2, WHITE);
    sprint("SCORE %d", score);

    scursor(4, SCRH - 10, LIGHT_GREY);
    sprint("FLICK TO SHOOT");
  }
};

int main() {
  App app;
  app.run();
  return 0;
}