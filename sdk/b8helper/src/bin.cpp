#include <unordered_map>
#include "bin.h"
#include <b8/assert.h>
#include <trace.h>

using namespace std;

static  CBinHolder* _holder = nullptr;

void    CBinHolder::Register( CBin* bin_ ){
  _ASSERT( _dict.find( bin_->_name ) == _dict.end() , "duplicate registrations are not possible" );
  _dict[ bin_->_name ] = bin_;
}

CBin*   CBinHolder::Get( const char* name_ ) {
  _ASSERT( _holder != nullptr , "not found");
  if( _holder->_dict.find( name_ ) == _holder->_dict.end() ){
    for( auto name : _holder->_dict ){
      printf( "%s\n",name.first.c_str() );
    }
    _ASSERT( _holder->_dict.find( name_ ) != _holder->_dict.end() , "not found" );
  }
  return _holder->_dict[ name_ ];
}

const std::unordered_map< std::string , CBin* >& CBinHolder::GetDictionary() {
  return _holder->_dict;
}

CBinHolder::CBinHolder(){
  _holder = this;
}

const void* CBin::Addr() const {
  return  _addr;
}

size_t  CBin::Size() const {
  return _size;
}

CBin::CBin( const char* name_ , const void* addr_ , size_t size_ ){
  if( _holder == nullptr ){
    new CBinHolder;
  }
  WATCH( name_ );
  _name = string( name_ );
  _addr = addr_;
  _size = size_;
  _holder->Register( this );
}
