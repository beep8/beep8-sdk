#include <stdio.h>
#include <beep8.h>
#include <list>
#include <cobj.h>
#include <handle.h>

using namespace std;

static  u32 id_cobj = 1;

#define N_MAX_PRIORITY  (3)
static  list< HObj >  _list_hobjs[ N_MAX_PRIORITY ];

#define N_COBJ_LIST (1<<5)
static  list< CObj* > _list_all_cobj[ N_COBJ_LIST ];

static s32 _cnt_pause = 0;

void  CObj::Step(){
  this->vOnStep();
}
void  CObj::Draw( b8PpuCmd* cmd_ ){
  this->vOnDraw( cmd_ );
}
void  CObj::Touch(){
  this->vOnTouch();
}

CObj::CObj(){
  _id_cobj = id_cobj++;
  _list_all_cobj[ _id_cobj & (N_COBJ_LIST-1) ].push_back( this );
}

CObj::~CObj(){
}

void  CObjHolder_Reset(){
  u32 ii;
  id_cobj = 1;
  _cnt_pause = 0;
  for( ii=0 ; ii < N_MAX_PRIORITY ; ++ii){
    _list_hobjs[ ii ].clear();
  }

  for( ii=0 ; ii < N_COBJ_LIST ; ++ii ){
    list< CObj* >::iterator it = _list_all_cobj[ ii ].begin();
    while( it != _list_all_cobj[ ii ].end() ){
      delete *it;
      ++it;
    }
    _list_all_cobj[ ii ].clear();
  }
}

HObj CObjHolder_Entry( CObj* obj , u32 priority ){
  _ASSERT( priority < N_MAX_PRIORITY , "invalid priority" );
  _ASSERT( obj , "obj is nullptr" );
  _ASSERT( obj->GetHandle() == 0 , "already entried" );

  HObj hObj = Handle_Entry( obj );
  _list_hobjs[ priority ].push_back( hObj );
  obj->SetPriority( priority );
  obj->SetHandle( hObj );
  return  hObj;
}

void  CObjHolder_Remove( HObj hObj ){
  if( !Handle_IsAlive( hObj ) ) return;

  CObj* obj = static_cast< CObj* >( Handle_GetPointer( hObj ) );
  _list_hobjs[ obj->GetPriority() ].remove( hObj );
}

void  CObjHolder_Enum( std::vector< HObj >& dest_ , u32 prio_ , u32 type_id_ ){
  dest_.clear();
  list< HObj >::iterator it;
  list< HObj >& lst = _list_hobjs[ prio_ ];
  for( it = lst.begin() ; it != lst.end() ; ++it){
    CObj* obj = static_cast< CObj* >( Handle_GetPointer( *it ) );
    if( false ==  obj->IsTypeOf( type_id_ ) ) continue;
    dest_.push_back( *it );
  }
}

void  CObjHolder_Step( b8PpuCmd* cmd_ ){
  list< HObj >::iterator it;
  if( _cnt_pause <= 0 ){
    // step
    for( u32 prio=0 ; prio < N_MAX_PRIORITY ; ++prio){
      list< HObj >& lst = _list_hobjs[ prio ];
      for( it = lst.begin() ; it != lst.end() ; ++it){
        CObj* obj = static_cast< CObj* >( Handle_GetPointer( *it ) );
        if( !obj )  continue;
        obj->Step();
        obj->_cnt_step_called++;
      }
    }

    // touch
    for( u32 prio=0 ; prio < N_MAX_PRIORITY ; ++prio){
      list< HObj >& lst = _list_hobjs[ prio ];
      for( it = lst.begin() ; it != lst.end() ; ++it ){
        CObj* obj = static_cast< CObj* >( Handle_GetPointer( *it ) );
        if( !obj )  continue;
        if( 0 == obj->_cnt_step_called ) continue;
        obj->Touch();
      }
    }
  }
  --_cnt_pause;

  // draw
  for( u32 prio=0 ; prio < N_MAX_PRIORITY ; ++prio){
    list< HObj >& lst = _list_hobjs[ prio ];
    for( it = lst.begin() ; it != lst.end() ; ++it ){
      CObj* obj = static_cast< CObj* >( Handle_GetPointer( *it ) );
      if( !obj )  continue;
      if( 0 == obj->_cnt_step_called ) continue;
      obj->Draw( cmd_ );
    }
  }

  // gc
  for( u32 prio=0 ; prio < N_MAX_PRIORITY ; ++prio){
    list< HObj >& lst = _list_hobjs[ prio ];
    it = lst.begin();
    while( it != lst.end() ){
      CObj* obj = static_cast< CObj* >( Handle_GetPointer( *it ) );
      if( obj->IsReqKill() ){
        Handle_Remove( *it );
        _list_all_cobj[ obj->GetId() & (N_COBJ_LIST-1) ].remove( obj );
        delete obj;
        it = lst.erase( it );
      } else {
        ++it;
      }
    }
  }
}

void  CObjHolder_Pause( s32 cnt_pause_ ){
  _cnt_pause = cnt_pause_;
}

CObj* cobj( HObj hObj ){
  if( !Handle_IsAlive( hObj ) ) return nullptr;
  return  static_cast< CObj* >( Handle_GetPointer( hObj ) );
}