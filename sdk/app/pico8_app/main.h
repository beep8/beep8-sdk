#pragma once
#include <pico8.h>
#include <list>
#include <memory>
#include <array>
#include <span>
#include <vector>

template <typename T, typename... Args>
std::shared_ptr<T> create(Args&&... args) {
    return std::make_shared<T>(forward<Args>(args)...);
}
struct OneGame {
  bool  isReady = true;
  u16   frmReady = 0;
  bool  isGameOver = false;
  u16   frmGameOver = 0;
  OneGame();
};

class Pico8App : public pico8::Pico8 {
  bool  isFirstDraw = true;

  int frm = 0;
  int frmPause = 0;

  u16 upFlushAnim = 0;
  std::span<const pico8::Color> flushAnim; 

  void  _init();
  void  _update();
  void  _draw();
  void  reqOneGame();
};
