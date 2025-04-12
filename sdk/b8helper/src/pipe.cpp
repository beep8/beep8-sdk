#include <pipe.h>
#include <trace.h>

namespace Pipe {

CNullPipe::CNullPipe(){}
CNullPipe::~CNullPipe(){}

void  CMemBufferPipe::Dump(){
  u32 cnt=0;
  for( const auto& it : _buff ){
    printf( "0x%02x,",it);
    if( (cnt&15) == 15 )  printf( "\n" );
    ++cnt;
  }
}

void  Move(
  std::shared_ptr< Pipe::CPipe > pipe_in_,
  std::shared_ptr< Pipe::CPipe > pipe_out_
){
  u8 reg8;
  while( pipe_in_->Pop( reg8 ) ){
    pipe_out_->Push( reg8 );
  }
}

} // namespace Pipe