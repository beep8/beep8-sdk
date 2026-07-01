#include <pico8.h>
#include <sublibc.h>
#include "main.h"

using namespace std;  
using namespace pico8;  
extern	const uint8_t  b8_image_sprite0[ (128*128)>>1 ];

static  constexpr auto  flushAnimUsual  = to_array<pico8::Color>({DARK_BLUE});
static  constexpr auto  flushAnimLose   = to_array<pico8::Color>({RED,BLACK,ORANGE,RED,BLACK,RED,BLACK,ORANGE,ORANGE,ORANGE,DARK_BLUE,ORANGE,RED,ORANGE,ORANGE,RED});
static  constexpr auto  flushAnimBlue   = to_array<pico8::Color>({BLUE,DARK_BLUE,DARK_BLUE,BLACK,WHITE,BLUE,BLACK,BLUE,DARK_BLUE,BLUE,BLUE,BLACK,YELLOW,BLUE,BLUE,DARK_BLUE,WHITE,BLUE,BLACK,BLUE,BLACK});

class OneGame; 
using OneGameSP = shared_ptr< OneGame >;
OneGameSP oneGame;
static  bool  reqResetOneGame = false;
static  u32 cntRndAccume = 0x777;

class   Pico8App;
static  Pico8App* app;

OneGame::OneGame(){
}

void  Pico8App::_init(){
  upFlushAnim = 0;
  flushAnim = flushAnimUsual;  
  lsp(0,b8_image_sprite0 );
  mapsetup( TILES_32, TILES_32, nullopt, B8_PPU_BG_WRAP_CLAMP, B8_PPU_BG_WRAP_CLAMP, BG_0 );
  mapsetup( TILES_32, TILES_32, nullopt, B8_PPU_BG_WRAP_CLAMP, B8_PPU_BG_WRAP_CLAMP, BG_1 );
  frm = 0;
  reqResetOneGame = true;
}

void  Pico8App::_update(){
  cntRndAccume += rndi(0xffff);

  if( reqResetOneGame ){
    oneGame = create< OneGame >();
    print("\e[2J");
    reqResetOneGame = false;
    flushAnim = flushAnimUsual;  
  }

  if( frmPause > 0 ){
    --frmPause;
    return;
  }
  ++frm;

  if( oneGame->isReady ){
    oneGame->frmReady++;
    oneGame->isReady = oneGame->frmReady < 80 ? true : false;
    return;

  } else if( oneGame->isGameOver ){
    oneGame->frmGameOver++;
    return;
  }
}

void  Pico8App::_draw(){
  camera();

  bool bTitle = false;
  if( bTitle ){
    cls( GREEN );
  } else {
    cls( clamp_to_edge( flushAnim , upFlushAnim ) );
  }

  if( upFlushAnim < flushAnim.size() ) ++upFlushAnim;

  const auto zMax = maxz();
  setz( zMax );

  if( isFirstDraw ){
    isFirstDraw = false;
  }

  setz( zMax - 2 );

  if( bTitle ){
  } else {
    map(0,0,BG_1);
  } 

  if( oneGame->isReady ){
    if( (oneGame->frmReady>>1) & 15){
      scursor(50,12*8 );
      sprint( "Ready !" );
    }
  }

  setz( zMax - 4 );
}

int main(){
  app = new Pico8App;
  app->run();
  return 0;
}
