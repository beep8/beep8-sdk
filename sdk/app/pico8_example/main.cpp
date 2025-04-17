/*
  Simple sample of PICO-8 LikeLibrary for C/C++.

  A yellow round-faced character (Foo) walks randomly.
  You can also operate it with the PC keyboard arrow keys.
  On smartphones, you cannot operate it because there is no keyboard.
  You can toggle the drawing mode with 'z' on the PC keyboard.
*/
#include <pico8.h>

using namespace std;  
using namespace pico8;  

extern	const uint8_t  b8_image_sprite0[];
extern	const uint8_t  b8_image_sprite1[];

class _Pico8 : public Pico8 {
  Vec cam;

  // Foo's position
  Vec pos_foo;
  // Foo's velocity
  Vec v_foo;

  fx8 radius;
  int frame = 0;
  u32 mode = 0;

  // Initialization function called only once at startup.
  void  _init(){
    // Load the sprite image stored as a C array in ./data/export/sprite0.png.cpp
    // sprite0.png.cpp is converted from ./data/import/sprite0.png.
    // Conversion occurs when you 'make' after moving to './sdk/app/pico8_example/'.
    lsp(0,b8_image_sprite0 );

    // Load the sprite image stored as a C array in ./data/export/sprite1.png.cpp
    lsp(1,b8_image_sprite1 );

    // Initialize the BG layer (32 tiles x 32 tiles).
    // 1 tile is 8x8 pixels.
    mapsetup( TILES_32, TILES_32 );

    // Clear the BG layer to 0.
    b8PpuBgTile tile = {};
    mcls( tile );

    // Fill the BG layer randomly.
    for( int nn=0 ; nn<100 ; ++nn ){
      tile.XTILE = rand()%5;
      tile.YTILE = 2;
      msett( rand()&31, rand()&31, tile );
    }   

    // Initialize Foo's position.
    pos_foo.set(64,64);
  }

  // Called every 1/60th of a second.
  // Only internal status updates are allowed.
  // Drawing operations are not allowed here.
  void  _update(){
    ++frame;
    if( btn( BUTTON_LEFT ) ){
      v_foo.x = -1;
    } else if( btn( BUTTON_RIGHT ) ){
      v_foo.x = +1;
    } else {
      v_foo.x = 0;
    }

    if( btn( BUTTON_UP ) ){
      v_foo.y = -1;
    } else if( btn( BUTTON_DOWN ) ){
      v_foo.y = +1;
    } else {
      v_foo.y = 0;
    }
    pos_foo += v_foo;

    if( btnp( BUTTON_O ) ){
      mode = (mode+1)%5;
      printf( "mode=%ld\n", mode );
    }

    radius += fx8(1,256);

#if 0
    // You can check the button input status.
    if( btn( BUTTON_LEFT ) )  printf("LEFT\n");
    if( btn( BUTTON_RIGHT ) ) printf("RIGHT\n");
    if( btn( BUTTON_UP ) )    printf("UP\n");
    if( btn( BUTTON_DOWN ) )  printf("DOWN\n");
    if( btn( BUTTON_O ) )     printf("O\n");
    if( btn( BUTTON_X ) )     printf("X\n");
    printf( "mouse:(%ld,%ld) left:%ld\n", mousex(), mousey(), mousestatus() & pico8::LEFT );
#endif
  }

  // Called every 1/60th of a second.
  // Perform drawing of internal status here.
  void  _draw(){
    // Initialize the camera state.
    camera();
    // Clear the entire screen with LIGHT_PEACH.
    cls( LIGHT_PEACH );

    // Set depth information.
    setz( maxz() );

    // Initialize drawing target clipping.
    clip();

    // Set the camera position to Foo's position minus (64,64).
    cam = pos_foo - Vec(64,64);

    camera(cam.x, cam.y);
    // Draw the BG layer (MAP) with the camera position.
    map(cam.x, cam.y, BG_0);

    // Set depth to the foreground.
    setz( maxz()/2 );

    // Set the palette.
    const u8 palsel = 1;
    pal(WHITE, BLACK, palsel);

    // Draw the yellow round-faced Foo sprite.
    spr(16, pos_foo.x, pos_foo.y, 1, 1, v_foo.x < 0, false);

    // Switch drawing samples based on the value of mode.
    // The value of mode is controlled by the 'z' key on the PC keyboard.
    switch( mode ){
      case  0:
        break;

      // Triangle drawing sample.
      case  1:{
        Poly pol;
        pol.pos0.set(10,47);
        pol.pos1.set(83,93);
        pol.pos2.set(3,111);
        poly(pol, LAVENDER);
        poly(20,119,123,199,38,177, RED);
      } break;

      // Solid fill, point drawing, rectangle drawing sample.
      case 2:{
        rectfill(fx8(10), fx8(10), fx8(130), fx8(200), BLUE);
        rectfill(20, 30, fx8(50), 122, RED);
        rectfill(0, 0, 2, 2, YELLOW);
        pset(77, 113, LIGHT_GREY);
        rect(120, 100, 23, 23, DARK_GREEN);
      } break;

      // Circle drawing sample.
      case 3:{
        circ(fx8(64), fx8(63), 100, PINK);
        circfill(fx8(87), fx8(77), 30, ORANGE);
      } break;

      // Line drawing sample.
      case 4:{
        line(fx8(30), fx8(30), fx8(100), fx8(160), RED);
        line(fx8(30), fx8(10), fx8(198), fx8(11), RED);
        line(111, 10, -32, 122, ORANGE);
        line(-30, 10, 302, 122, BROWN);
      } break;

      default:
        break;
    }

    // Set the cursor position to (4,4) with WHITE color.
    scursor(4, 4, WHITE, 0);
    sprint("mode=%d\n", mode);

    const fx8 cx = 64;
    const fx8 cy = 128;

    const fx8 xx = cx + fx8(50) * cos(radius);
    const fx8 yy = cy + fx8(50) * sin(radius);
    const fx8 ww = 2;

    rectfill(
      cx-ww, cy-ww,
      cx+ww, cy+ww,
      RED
    );

    rectfill(
      xx-ww, yy-ww,
      xx+ww, yy+ww,
      BLUE
    );

    const fx8 dir = pico8::atan2(yy - cy, xx - cx);
    const fx8 ax = cx + fx8(30) * pico8::cos(dir);
    const fx8 ay = cy + fx8(30) * pico8::sin(dir);
    rectfill(
      ax-ww, ay-ww,
      ax+ww, ay+ww,
      GREEN
    );
  }
};

// main() for C/C++ language.
// Magic incantation to run the PICO-8 library.
static _Pico8* _pico8;
int main(){
  _pico8 = new _Pico8;
  _pico8->run();  // ::run() enters an infinite loop.
  return 0;
}
