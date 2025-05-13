#include <map>
#include <b8/hif.h>
#include <b8/sys.h>
#include <b8/assert.h>
#include <hif_decoder.h>
#include <trace.h>

using namespace std;

class ImplCHifDecoder{
  friend  class CHifDecoder;
  map< u8 , b8HifEvent > _map_hifev;  // TODO: remove

  u32 _hdl = 1;
  map< u8 , HifPoint* > _map_hifp; 

  void  ClearIdentifierZero(){
    for(
      map< u8 , b8HifEvent >::iterator it = _map_hifev.begin() ;
      it != _map_hifev.end() ;
    ){
      if( it->second.identifier == 0 ){
        //trace( it->second.identifier );
        it = _map_hifev.erase( it );
      } else {
        ++it;
      }
    }
  }
};

CHifDecoder::CHifDecoder(){
  impl = new ImplCHifDecoder;
}

CHifDecoder::~CHifDecoder(){
  delete impl;
}

const b8HifMouseStatus* CHifDecoder::GetMouseStatus(){
  return  b8HifGetMouseStatus();
}

const map< u8 , HifPoint* >& CHifDecoder::GetStatus(){
  for (auto it = impl->_map_hifp.begin() ; it != impl->_map_hifp.end() ; ){
    switch( it->second->ev.type ){
      case B8_HIF_EV_TOUCH_CANCEL:
      case B8_HIF_EV_TOUCH_END:
      case B8_HIF_EV_MOUSE_UP:{
        delete it->second;
        it = impl->_map_hifp.erase(it);
      }break;

      default: ++it; break;
    }
  }

  b8HifEvents result;
  b8HifGetEvents( &result );
  if( result.num == 0 ) return  impl->_map_hifp;

  b8HifEvent* ev = &result.events[0];
  for( size_t ii=0 ; ii < result.num ; ++ii,++ev ){
    switch( ev->type ){
      case  B8_HIF_EV_TOUCH_START:
      case  B8_HIF_EV_MOUSE_DOWN:
      case  B8_HIF_EV_MOUSE_HOVER_MOVE:
      {
        auto it = impl->_map_hifp.find( ev->identifier );
        if (it == impl->_map_hifp.end()) {  // TODO: 
          HifPoint* hp = new HifPoint( *ev );
          hp->hdl = impl->_hdl++;
          hp->ptype =
            ev->type == B8_HIF_EV_TOUCH_START ?  HifPoint::PointType::Touch :
            ev->type == B8_HIF_EV_MOUSE_DOWN  ?  HifPoint::PointType::Mouse : HifPoint::PointType::None ;
          impl->_map_hifp[ ev->identifier ] = hp;
        }
      }break;
      default: break;
    }

    switch( ev->type ){
      case  B8_HIF_EV_TOUCH_MOVE:
      case  B8_HIF_EV_TOUCH_CANCEL:
      case  B8_HIF_EV_TOUCH_END:
      case  B8_HIF_EV_MOUSE_MOVE:
      case  B8_HIF_EV_MOUSE_UP:
      case  B8_HIF_EV_MOUSE_HOVER_MOVE:
      {
        auto it = impl->_map_hifp.find( ev->identifier );
        if (it != impl->_map_hifp.end()) {
          impl->_map_hifp[ ev->identifier ]->ev = *ev;
        }
      }break;
      default: break;
    }
  }
  return  impl->_map_hifp;
}
