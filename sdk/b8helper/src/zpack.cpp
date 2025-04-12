#include <algorithm>
#include <trace.h>
#include <zpack.h>
#include <rle.h>
#include <huffman.h>
//#define VERBOSE

// flat
// rle
// huffman
// rle -> huffman

using namespace std;
using namespace Huffman;
using namespace Rle;
using namespace Pipe;
using namespace ZPack;

void  CZPackEncoder::SetIn ( std::shared_ptr< Pipe::CPipe > pipe_in_  ){
  _pipe_in = pipe_in_;
}

void  CZPackEncoder::SetOut( std::shared_ptr< Pipe::CPipe > pipe_out_ ){
  _pipe_out = pipe_out_;
}

void  CZPackEncoder::Encode(){
  size_t size_of_org = 0;
  u8 reg8;
  while( _pipe_in->Pop( reg8 )){
    ++size_of_org;
  }
  _pipe_in->SeekPop(0);

  CHuffmanEncoder huf;
  huf.SetIn ( _pipe_in );
  auto pipe_mem_huf_packed = make_shared< CMemBufferPipe >();
  huf.SetOut( pipe_mem_huf_packed );
  huf.Encode();

  const size_t size_of_huf = pipe_mem_huf_packed->Size();

  pipe_mem_huf_packed->SeekPop(0);
  CRleEncoder rle;
  rle.SetIn ( pipe_mem_huf_packed );
  auto pipe_mem_huf_rle_packed = make_shared< CMemBufferPipe >();
  rle.SetOut( pipe_mem_huf_rle_packed );
  rle.Encode();

  const size_t size_of_huf_rle = pipe_mem_huf_rle_packed->Size();

  const size_t min_size =
    std::min(
      std::min( size_of_org , size_of_huf ),
      size_of_huf_rle
    );

  CompressionMethod cm = Flat;
  if( min_size == size_of_org ){
    cm = FlatWithSize;
  } else if ( min_size == size_of_huf ){
    cm = Huffman;
  } else if ( min_size == size_of_huf_rle ){
    cm = HuffmanToRle;
  }

  _pipe_out->Push( ZPack::Signature );
  _pipe_out->Push( cm );
  switch( cm ){
    case  HuffmanToRle:{
      pipe_mem_huf_rle_packed->SeekPop(0);
      Pipe::Move( pipe_mem_huf_rle_packed, _pipe_out );
    }break;
    case  Huffman:{
      pipe_mem_huf_packed->SeekPop(0);
      Pipe::Move( pipe_mem_huf_packed, _pipe_out );
    }break;
    case  Flat:{
      _pipe_in->SeekPop(0);
      Pipe::Move( _pipe_in, _pipe_out );
    }break;
    case  FlatWithSize:{
      u32 orgsize = size_of_org;
      _pipe_out->Push( orgsize & 0xff );
      orgsize >>= 8;
      _pipe_out->Push( orgsize & 0xff );
      orgsize >>= 8;
      _pipe_out->Push( orgsize & 0xff );

      _pipe_in->SeekPop(0);
      Pipe::Move( _pipe_in, _pipe_out );
    }break;
  }
}

// ---

void  CZPackDecoder::SetIn ( std::shared_ptr< Pipe::CPipe > pipe_in_  ){
  _pipe_in = pipe_in_;
}

void  CZPackDecoder::SetOut( std::shared_ptr< Pipe::CPipe > pipe_out_ ){
  _pipe_out = pipe_out_;
}

CZPackDecoder::DecodeResult CZPackDecoder::Decode () {
  u8 sig = 0x00;
  _pipe_in->Pop( sig );
  if( sig != ZPack::Signature ){
    return CZPackDecoder::DECODE_ERR_INVALID_SIGNATURE;
  }

  u8 compression_method = 0x00;
  _pipe_in->Pop( compression_method );
  CompressionMethod cm = CompressionMethod( compression_method & 3);
  switch( cm ){
    case  HuffmanToRle:{
#ifdef VERBOSE
WATCH("HuffmanToRle");
#endif
      CRleDecoder rle_decoder;
      rle_decoder.SetIn( _pipe_in  );
      auto pipe_mem_rle_decoded = make_shared< CMemBufferPipe >();
      rle_decoder.SetOut( pipe_mem_rle_decoded );
      auto rle_result = rle_decoder.Decode();
      if( rle_result != CRleDecoder::DECODE_OK ){
        return  CZPackDecoder::DECODE_INVALID_DATA;
      }
      CHuffmanDecoder hf_decoder;
      hf_decoder.SetIn(  pipe_mem_rle_decoded );
      hf_decoder.SetOut( _pipe_out );
      auto hf_result = hf_decoder.Decode();
      if( hf_result != CHuffmanDecoder::DECODE_OK ){
        return  CZPackDecoder::DECODE_INVALID_DATA;
      }
      return  CZPackDecoder::DECODE_OK;
    }break;

    case  Huffman:{
#ifdef VERBOSE
WATCH("Huffman");
#endif
      CHuffmanDecoder hf_decoder;
      hf_decoder.SetIn( _pipe_in  );
      hf_decoder.SetOut(_pipe_out );
      CHuffmanDecoder::DecodeResult result = hf_decoder.Decode();
      if( result != CHuffmanDecoder::DECODE_OK ){
        return  CZPackDecoder::DECODE_INVALID_DATA;
      }
      return  CZPackDecoder::DECODE_OK;
    }break;

    case  Flat:{
#ifdef VERBOSE
WATCH("Flat");
#endif
      Pipe::Move( _pipe_in , _pipe_out );
      return  CZPackDecoder::DECODE_OK;
    }break;
    case  FlatWithSize:{
#ifdef VERBOSE
WATCH("FlatWithSize");
#endif
      u32 orgsize = 0;
      for( int ii=0 ; ii<3 ; ++ii){
        u8  xx = 0;
        if( ! _pipe_in->Pop(xx) )  return CZPackDecoder::DECODE_INVALID_DATA;
        orgsize |= u32(xx)<<(ii*8);
      }

      for( u32 nn=0 ; nn<orgsize ; ++nn ){
        u8 xx;
        if( ! _pipe_in ->Pop (xx))  return CZPackDecoder::DECODE_INVALID_DATA;
        if( ! _pipe_out->Push(xx))  return CZPackDecoder::DECODE_INVALID_DATA;
      }

      return  CZPackDecoder::DECODE_OK;
    }break;
  }
  return  CZPackDecoder::DECODE_OK;
}