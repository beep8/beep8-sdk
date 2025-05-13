#include <stdio.h>
#include <beep8.h>
#include <nesctrl.h>
#include "pad.h"
#include "trace.h"

using namespace std;

constexpr u32 OTZ = 6;

static CPadObj* _cpadobj[4] = {nullptr,nullptr,nullptr,nullptr};

class ImplCPadObj {
  friend class CPadObj;
  CNesCtrl* _pnes_ctrl = nullptr;

  bool  GetStatus( u32 pad_  , EPad epad_ )  const {
    if( pad_ & (1 << (int)epad_) ){
      return true;
    }
    if( nullptr == _pnes_ctrl ) return false;

    ENesCtrl enc;
    switch( epad_ ){
      case  EPad::START:
      case  EPad::SELECT:
      case  EPad::EPAD_MAX:
        return false;
      case  EPad::LEFT:
        enc = ENesCtrl::LEFT;
        break;
      case  EPad::UP:
        enc = ENesCtrl::UP;
        break;
      case  EPad::RIGHT:
        enc = ENesCtrl::RIGHT;
        break;
      case  EPad::DOWN:
        enc = ENesCtrl::DOWN;
        break;
      case  EPad::BTN_A:
        enc = ENesCtrl::BTN_A;
        break;
      case  EPad::BTN_B:
        enc = ENesCtrl::BTN_B;
        break;
    }
    return  _pnes_ctrl->GetCntHold( enc ) > 0;
  }

  ~ImplCPadObj(){
    if( _pnes_ctrl )  delete _pnes_ctrl;
  }
};

void  CPadObj::vOnStep(){
  if( impl->_pnes_ctrl ){
    impl->_pnes_ctrl->Step();
  }

  const u32 pad = B8_HIF_PAD( _idx );

  for( u8 nn=0 ; nn<8 ; ++nn){
    if( impl->GetStatus( pad  , (EPad)nn ) ){
      _cnt_hold   [ nn ]++;
      _cnt_release[ nn ] = 0;
    } else {
      _cnt_hold   [ nn ] = 0;
      _cnt_release[ nn ]++;
    }
  }
}

void  CPadObj::vOnDraw( b8PpuCmd* cmd_ ){
  if( impl->_pnes_ctrl ){
    impl->_pnes_ctrl->Draw( cmd_ , OTZ );
  }
}

CPadObj::CPadObj( u32 idx_ , const CfgNesCtrl* pcfg_ )
  : _idx( idx_ )
{
  _ASSERT( _idx < 4 , "invalid idx");
  impl = new ImplCPadObj;
  if( pcfg_ ){
    impl->_pnes_ctrl = new CNesCtrl( *pcfg_ );
    impl->_pnes_ctrl->Reset();
  }
  _cpadobj[ _idx ] = this;
}

CPadObj::~CPadObj(){
  _cpadobj[ _idx ] = nullptr;
  delete impl;
}

CPadObj*  _CPadObj( u32 idx_ ){
  _ASSERT( idx_ < 4 , "invalid idx");
  _ASSERT( _cpadobj[ idx_ ] != nullptr , "no CPadObj" );
  return _cpadobj[ idx_ ];
}