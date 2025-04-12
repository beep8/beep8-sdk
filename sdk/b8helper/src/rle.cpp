// MIT License

// Copyright (c) 2022 ZooK

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#include <vector>
#include <rle.h>
#include <trace.h>
#include "stdio.h"

using namespace std;
using namespace Pipe;

constexpr u8  END_OF_MARK = 0x00;
namespace Rle {

// ------------------------------------------------------------------------------
// rle encoder
// ------------------------------------------------------------------------------
CRleEncoder::EncodeResult CRleEncoder::Encode(){
  _pipe_in->SeekPop(0);

  struct RunLen {
    u16 _data;
    u8 _rep;
    void  Reset(){
      _data = 0xffff;
      _rep  = 0;
    }
  };

  RunLen rl;
  rl.Reset();

  vector< u8 > uniq;
  u8 reg8;
  while( _pipe_in->Pop( reg8 ) ){
    const bool equal = (rl._data == reg8 && rl._rep < 0x7f ) ? true : false;
    bool reset = false;
    if( equal ){
      if( uniq.size() >= 2 ){
        _pipe_out->Push( -s8( uniq.size()-1 ) );
        for( size_t ii=0 ; ii < uniq.size()-1 ; ++ii ){
          _pipe_out->Push( uniq[ii] );
        }
      }
      uniq.clear();
    } else {
      if( rl._rep >= 2 ){
        _pipe_out->Push( rl._rep  );
        _pipe_out->Push( rl._data );
      }
      uniq.push_back( reg8 );

      if( uniq.size() >= 0x7f ){
        _pipe_out->Push( -s8( uniq.size() ) );
        for( size_t ii=0 ; ii < uniq.size() ; ++ii ){
          _pipe_out->Push( uniq[ii] );
        }
        reset = true;;
      }
    }

    if( reset ){
      uniq.clear();
      rl.Reset();

    } else {
      rl._rep = equal ? rl._rep+1 : 1;
      rl._data = reg8;
    }
  }

  if( uniq.size() >= 2 ){
    _pipe_out->Push( -s8( uniq.size() ) );
    for( const auto& it : uniq ){
      _pipe_out->Push( it );
    }
  } else if( rl._rep >= 1 ){
    _pipe_out->Push( rl._rep  );
    _pipe_out->Push( rl._data );
  }

  _pipe_out->Push( END_OF_MARK );
  return  CRleEncoder::ENCODE_OK;
}

void  CRleEncoder::SetIn( std::shared_ptr< Pipe::CPipe > pipe_in_ ){
  _pipe_in = pipe_in_;
}

void  CRleEncoder::SetOut( std::shared_ptr< Pipe::CPipe > pipe_out_ ){
  _pipe_out = pipe_out_;
}

// ------------------------------------------------------------------------------
// rle decoder
// ------------------------------------------------------------------------------
void  CRleDecoder::SetIn ( std::shared_ptr< Pipe::CPipe > pipe_in_  ){
  _pipe_in = pipe_in_;
}

void  CRleDecoder::SetOut( std::shared_ptr< Pipe::CPipe > pipe_out_ ){
  _pipe_out = pipe_out_;
}

union	ForceCast8 {
  u8 _du8;
  s8 _ds8;
};

CRleDecoder::DecodeResult  CRleDecoder::Decode(){
  ForceCast8 fc8;
  u8 reg8;
  while( _pipe_in->Pop( reg8 ) ){
    fc8._du8 = reg8;
    if( fc8._ds8 > 0){
      _pipe_in->Pop( reg8 );
      for( s8 ii=0 ; ii<fc8._ds8 ; ++ii ){
        _pipe_out->Push( reg8 );
      }
    } else if( fc8._ds8 < 0){
      for( s8 ii=0 ; ii< -fc8._ds8 ; ++ii ){
        _pipe_in->Pop( reg8 );
        _pipe_out->Push( reg8 );
      }
    } else if (fc8._du8 == END_OF_MARK ){
      return  CRleDecoder::DECODE_OK;
    }
  }
  return  CRleDecoder::DECODE_OK;
}

} // namespace Rle