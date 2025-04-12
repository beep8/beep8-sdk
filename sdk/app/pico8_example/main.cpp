#include <pico8.h>

using namespace std;  
using namespace pico8;  

extern	const uint8_t  b8_image_sprite0[];
extern	const uint8_t  b8_image_sprite1[];

class _Pico8 : public Pico8 {
  Vec cam;

  fx8 radius;
  int frame = 0;
  u32 mode = 0;

  void  _init(){
    lsp(0,b8_image_sprite0 );
    lsp(1,b8_image_sprite1 );

    frame = 0;
    mapsetup( TILES_32, TILES_32 );

    b8PpuBgTile tile = {};
    mcls( tile );

    tile.YTILE = 2;

    for( int nn=0 ; nn<30 ; ++nn ){
      msett( rand()&15,rand()&15,tile);
    }   
    for( int yy=0 ; yy<TILES_32 ; ++yy ){
      for( int xx=0 ; xx<TILES_32 ; ++xx ){
        b8PpuBgTile tr;
        tr = mgett(xx,yy);
        if( tr.XTILE == 0 && tr.YTILE == 0 ){
          printf(".");
        } else {
          printf("x");
        } 
      }
      printf("\n");
    }
  }

  void  _update(){
    ++frame;
    if( btn( BUTTON_LEFT ) ){
      cam.x -= 1;
    } else if( btn( BUTTON_RIGHT ) ){
      cam.x += 1;
    }
    if( btn( BUTTON_UP ) ){
      cam.y -= 1;
    } else if( btn( BUTTON_DOWN ) ){
      cam.y += 1;
    }

    if( btnp( BUTTON_O ) ){
      mode = (mode+1)%5;
      printf( "mode=%ld\n",mode);
    }

    radius += fx8(1,256);

#if 0
    if( btn( BUTTON_LEFT ) )  printf("LEFT\n");
    if( btn( BUTTON_RIGHT ) ) printf("RIGHT\n");
    if( btn( BUTTON_UP ) )    printf("UP\n");
    if( btn( BUTTON_DOWN ) )  printf("DOWN\n");
    if( btn( BUTTON_O ) )     printf("O\n");
    if( btn( BUTTON_X ) )     printf("X\n");
#else
    //printf( "mouse:(%d,%d) left:%ld\n" , mousex(),mousey() ,mousestatus()& pico8::LEFT );
#endif
  }

  void  _draw(){
    camera();
    cls( LIGHT_PEACH );

    setz( maxz() );
    clip();

    setz(0);
    clip();

    setz( maxz()-1 );
    Rect rc;
    rc.x = rc.y = 10;
    rc.w = 128-rc.x*2;
    rc.h = 224-rc.y*2;
    color( WHITE );
    rect    (rc.x-1,rc.y-1,rc.x + rc.w,rc.y + rc.h,BLACK);
    rectfill(rc.x,rc.y,rc.x + rc.w,rc.y + rc.h);
    clip(rc.x,rc.y,rc.w,rc.h);

    setz( maxz() );
    camera();
    map(cam.x,cam.y,BG_0);
    camera(cam.x,cam.y);
    setz( maxz()/2 );

    const u8 palsel = 1;
    pal(WHITE,BLACK,palsel);

    auto db = stat( 1000 );
    switch( mode ){
      case  0:
        //spr(16+256,43,55, 5,4,false,true,palsel);
        sprb(1,16,43,55, 5,4,false,true,palsel);
        break;

      case  1:{
        Poly pol;
        pol.pos0.set(10,47);
        pol.pos1.set(83,93);
        pol.pos2.set(3,111);
        poly(pol,LAVENDER );
        poly(20,119,123,199,38,177,RED );
      }break;

      case 2:{
        rectfill(fx8(10),fx8(10),fx8(130),fx8(200),BLUE);
        rectfill(20,30,fx8(50),122,RED);
        rectfill(0,0,2,2,YELLOW);
        pset(77,113,LIGHT_GREY);
        rect(120,100,23,23,DARK_GREEN);
      }break;

      case 3:{
        circ(fx8(64), fx8(63), 100 ,PINK );
        circfill(fx8(87), fx8(77), 30 , ORANGE);
      }break;

      case 4:{
        line(fx8(30),fx8(30),fx8(100),fx8(160),RED);
        line(fx8(30),fx8(10),fx8(198),fx8(11),RED);
        line(111,10,-32,122,ORANGE);
        line(-30,10,302,122,BROWN);
      }break;

      default:
        break;
    }

    const auto diff = stat( 1000 ) - db;
    scursor(4,4,WHITE,0);
    sprint("mode=%d db=%d\n",mode, diff );

    const fx8 cx = 64;
    const fx8 cy = 128;

    const fx8 xx = cx + fx8(50) * cos( radius );
    const fx8 yy = cy + fx8(50) * sin( radius );
    const fx8 ww = 2;

    rectfill(
      cx-ww,cy-ww,
      cx+ww,cy+ww,
      RED
    );

    rectfill(
      xx-ww,yy-ww,
      xx+ww,yy+ww,
      BLUE
    );

    const fx8 dir = pico8::atan2( yy - cy , xx - cx );
    const fx8 ax = cx + fx8(30) * pico8::cos( dir );
    const fx8 ay = cy + fx8(30) * pico8::sin( dir );
    rectfill(
      ax-ww,ay-ww,
      ax+ww,ay+ww,
      GREEN
    );
  }
};

static  _Pico8* _pico8;

int main(){
  _pico8 = new _Pico8;
  _pico8->run();
  return 0;
}
