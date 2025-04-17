/*
  PICO-8 LikeLibrary for C/C++ の簡単なサンプルです。

  黄色い丸顔のキャラクター(Foo君)がランダムで歩きます。
  PCのキーボードのカーソルキーで動作させることもできます。
  スマホ経由だとキーボードがないので操作できません。
  PCのキーボードの'z'で、描画モードをトグルできます。
*/
#include <pico8.h>

using namespace std;  
using namespace pico8;  

extern	const uint8_t  b8_image_sprite0[];
extern	const uint8_t  b8_image_sprite1[];

class _Pico8 : public Pico8 {
  Vec cam;

  // Foo君の座標
  Vec pos_foo;
  // Foo君の速度
  Vec v_foo;

  fx8 radius;
  int frame = 0;
  u32 mode = 0;

  // 初期化時に一回だけ呼び出される初期化関数です。
  void  _init(){
    // ./data/export/sprite0.png.cpp にC言語配列変数として収納されているスプライトイメージをロードします
    // sprite0.png.cpp は ./data/import/sprite0.png から変換されたデータです。
    // ./sdk/app/pico8_example/' で移動してから 'make' したときに変換されます。
    lsp(0,b8_image_sprite0 );

    // ./data/export/sprite1.png.cpp にC言語配列変数として収納されているスプライトイメージをロードします
    lsp(1,b8_image_sprite1 );

    // BG面(32tile x 32tile)を初期化します。
    // 1tileは 8x8 pixです
    mapsetup( TILES_32, TILES_32 );

    // BG面を0クリアします。
    b8PpuBgTile tile = {};
    mcls( tile );

    // BG面をランダムで埋めます
    for( int nn=0 ; nn<100 ; ++nn ){
      tile.XTILE = rand()%5;
      tile.YTILE = 2;
      msett( rand()&31,rand()&31,tile);
    }   

    // Foo君の位置を初期化
    pos_foo.set(64,64);
  }

  // 1/60秒毎に呼び出されます
  // 内部ステータスの更新のみを実行できます。
  // 描画系は出来ません。
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
      printf( "mode=%ld\n",mode);
    }

    radius += fx8(1,256);

#if 0
    // ボタンの入力状況を確認できます
    if( btn( BUTTON_LEFT ) )  printf("LEFT\n");
    if( btn( BUTTON_RIGHT ) ) printf("RIGHT\n");
    if( btn( BUTTON_UP ) )    printf("UP\n");
    if( btn( BUTTON_DOWN ) )  printf("DOWN\n");
    if( btn( BUTTON_O ) )     printf("O\n");
    if( btn( BUTTON_X ) )     printf("X\n");
    printf( "mouse:(%ld,%ld) left:%ld\n" , mousex(),mousey() ,mousestatus()& pico8::LEFT );
#endif
  }

  // 1/60秒毎に呼び出されます
  // 内部ステータスの描画をします。
  void  _draw(){
    // カメラ状態を初期化します
    camera();
    // 画面全体を LIGHT_PEACH で単色クリアします
    cls( LIGHT_PEACH );

    // 奥行き情報をセットします。
    setz( maxz() );

    // 描画対象クリッピングを初期化します
    clip();

    // カメラ位置をFoo君の座標から(64,64)だけ左上にセットします。
    cam = pos_foo - Vec(64,64);

    camera(cam.x,cam.y);
    // BG面(MAP)をカメラ位置を渡して描画します。
    map(cam.x,cam.y,BG_0);

    // 奥行き設定を手前にします。
    setz( maxz()/2 );

    // パレット設定します
    const u8 palsel = 1;
    pal(WHITE,BLACK,palsel);

    // 黄色い丸顔なFoo君をSpriteで描画します
    spr(16,pos_foo.x,pos_foo.y,1,1,v_foo.x<0,false);

    // mode の値によって描画サンプルを切り替えます
    // mode の値は PC キーボードの 'z' によって操作します
    switch( mode ){
      case  0:
        break;

      // 三角形描画サンプル
      case  1:{
        Poly pol;
        pol.pos0.set(10,47);
        pol.pos1.set(83,93);
        pol.pos2.set(3,111);
        poly(pol,LAVENDER );
        poly(20,119,123,199,38,177,RED );
      }break;

      // 単色塗りつぶし、点描画、矩形描画サンプル
      case 2:{
        rectfill(fx8(10),fx8(10),fx8(130),fx8(200),BLUE);
        rectfill(20,30,fx8(50),122,RED);
        rectfill(0,0,2,2,YELLOW);
        pset(77,113,LIGHT_GREY);
        rect(120,100,23,23,DARK_GREEN);
      }break;

      // 円描画サンプル
      case 3:{
        circ(fx8(64), fx8(63), 100 ,PINK );
        circfill(fx8(87), fx8(77), 30 , ORANGE);
      }break;

      // 線分描画サンプル
      case 4:{
        line(fx8(30),fx8(30),fx8(100),fx8(160),RED);
        line(fx8(30),fx8(10),fx8(198),fx8(11),RED);
        line(111,10,-32,122,ORANGE);
        line(-30,10,302,122,BROWN);
      }break;

      default:
        break;
    }

    // カーソル位置を(4,4)、色をWHITEに設定
    scursor(4,4,WHITE,0);
    sprint("mode=%d\n",mode );

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

// C/C++言語としてのmain()です。 
// PICO-8 ライブラリを動作させるおまじないです。
static  _Pico8* _pico8;
int main(){
  _pico8 = new _Pico8;
  _pico8->run();  // ::run() の中で無限Loopに入ります。
  return 0;
}
