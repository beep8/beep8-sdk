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

#include <unordered_map>
#include <memory>
#include <vector>
#include <list>
#include <string>
#include <cstr.h>
#include <sublibc.h>
#include <trace.h>
#include <huffman.h>

//#define VERBOSE

using namespace std;
using namespace Huffman;
using namespace Pipe;

#define MAX_SIZE_POW2       (20)
#define SIG_GENERAL_HUFFMAN (0x77)

class SerialBytes{
  std::shared_ptr< Pipe::CPipe > _pipe_in  = std::make_shared< Pipe::CNullPipe >();
  std::shared_ptr< Pipe::CPipe > _pipe_out = std::make_shared< Pipe::CNullPipe >();

public:
  u8      _cnt_send_bits = 0;
  u8      _send_byte = 0x00;
  u32     _cnt_recv_bits = 0;
  std::vector< u8 > _buff;
  void  PushBits( bool b_ );
  void  PushBitsU8( u8 byte_ ){
    PushBitsN( 8 , byte_ );
  }
  void  PushBitsN( u8 bits_ , u32 senddata_ );
  void  SetOut( std::shared_ptr< Pipe::CPipe > pipe_out_ );

  u8      _reg_pop = 0x00;
  bool  PopBits(){
    const u32 bit = _cnt_recv_bits&7;
    if( bit == 0 ){
      if( _pipe_in ) _pipe_in->Pop( _reg_pop );
    }
    ++_cnt_recv_bits;
    const bool retval = (_reg_pop&(1<<7)) ? true : false;
    _reg_pop <<= 1;
    return  retval;
  }
  u8    PopBitsU8();
  u32   PopBitsN( u8 bits_ );
  void  SetIn ( std::shared_ptr< Pipe::CPipe > pipe_in_  );
  void  Flush();
  SerialBytes();
  size_t  size()  const {
    return _buff.size();
  }
};

void  SerialBytes::SetIn( std::shared_ptr< Pipe::CPipe > pipe_in_ ){
  _pipe_in = pipe_in_;
}

void  SerialBytes::SetOut( std::shared_ptr< Pipe::CPipe > pipe_out_ ){
  _pipe_out = pipe_out_;
}

void  SerialBytes::PushBits( bool b_ ){
  const u8 xx = (b_ == true) ? 1:0;
  _send_byte = (_send_byte<<1) | xx;
  ++_cnt_send_bits;
  if( _cnt_send_bits >= 8 ){
    if( _pipe_out ) _pipe_out->Push( _send_byte );

    _buff.push_back( _send_byte );
    _send_byte = 0x00;
    _cnt_send_bits = 0;
  }
}

void  SerialBytes::PushBitsN( u8 bits_ , u32 senddata_ ){
  for( int sft = bits_-1 ; sft >= 0 ; --sft ){
    PushBits( senddata_ & (1<<sft) ? true : false );
  }
}

u32   SerialBytes::PopBitsN( u8 bits_ ){
  u32 result = 0x00000000;
  for( u8 nn=0 ; nn<bits_ ; ++nn ){
    result = (result<<1)| (PopBits() ? 1:0);
  }
  return result;
}

u8  SerialBytes::PopBitsU8(){
  return  PopBitsN(8);
}

SerialBytes::SerialBytes(){
  _buff.reserve(0x200);
}

void  SerialBytes::Flush(){
  if( _cnt_send_bits > 0 ){
    for( u8 nn = _cnt_send_bits ; nn < 8 ; ++nn ){
      PushBits( false );
    }
  }

#ifdef VERBOSE
  int cnt = 0;
  for( u8 byte : _buff ){
    printf( "0x%02x,",byte );
    if((cnt&15) == 15 ) printf("\n");
    ++cnt;
  }
#endif
}

struct HuffmanCode{
  u8    _len  = 0;
  u32   _code = 0x00000000;
  u32   Get() const;
  void  PushLeft();
  void  PushRight();
  void  Send( SerialBytes& sb ) const;
  str32 ToStr() const;
};

struct KeyHash {
  std::size_t operator()(const HuffmanCode& k) const {
    return (k._code<<8);
  }
};

struct KeyEqual {
  bool operator()(const HuffmanCode& lhs, const HuffmanCode& rhs) const {
    return lhs._code == rhs._code;
  }
};

// ------------------------------------------------------------------------------
// huffman encoder
// ------------------------------------------------------------------------------
struct  Branch;
struct  WorkEnc{
  static WorkEnc* instance;
  static WorkEnc& GetSingleInstance() {
    _ASSERT( instance , "invalid instance" );
    return *instance;
  }

  u32     _freq_ap[ 0x100 ];
  size_t  _up_pool_of_branches = 0;
  u8      _pool_of_branches[ 32 * 512 ];  // 32 = sizeof( Branch )
  unordered_map< HuffmanCode,u8,KeyHash,KeyEqual >   _h2c;  // huffman2code
  HuffmanCode _c2h[ 0x100 ];  // code2huffman

  void  RegisterCode( HuffmanCode huffman_code_ , u8 code_ );
  void  Export( std::shared_ptr< Pipe::CPipe > pipe_in_,  SerialBytes& sb  , size_t bytesize_ );

  u8      _send_bits = 0;
  u8      _send_byte = 0x00;
  void  SendSerial( bool b_ );
  WorkEnc(){
    WorkEnc::instance = this;
    memsetz(_freq_ap,sizeof(_freq_ap));
  }
  ~WorkEnc();
};
WorkEnc* WorkEnc::instance = nullptr;

struct Branch{
  friend  struct WorkEnc;
  u8          _data     = 0x00;
  u32         _freq_ap  = 0;
  HuffmanCode _huffman_code;
  Branch*     _left     = nullptr;
  Branch*     _right    = nullptr;
  WorkEnc*    _work;
  void  Dump()  const {
    printf( "_data=0x%02x %ld times\n" , _data , _freq_ap );
  }

#ifdef VERBOSE
  void  DumpRecursive(int depth=0)  const {
    for( int nn=0 ; nn<depth ; ++nn ) printf("-");

    if( _left == nullptr && _right == nullptr ){
      str32 strhuff = _huffman_code.ToStr();
      // L:=Leaf
      printf( "L:%ld times H:%sb=0x%lx C:0x%02x\n", _freq_ap, strhuff.c_str(),_huffman_code._code, _data);
    } else {
      // B:=Branch
      printf( "B:%ld times\n", _freq_ap);
      _left->DumpRecursive(depth+1);
      _right->DumpRecursive(depth+1);
    }
  }
#endif

  void  VisitRecursive( HuffmanCode hc_ ){
    if( _left == nullptr && _right == nullptr ){
      _huffman_code = hc_;
      _work->RegisterCode( hc_ , _data );
      return;
    }

    if( _left != nullptr ){
      HuffmanCode hcl = hc_;
      hcl.PushLeft();
      _left ->VisitRecursive( hcl );
    }

    if( _right != nullptr ){
      HuffmanCode hcr = hc_;
      hcr.PushRight();
      _right->VisitRecursive( hcr );
    }
  }

public:
  static void *operator new(size_t size) {
    WorkEnc& wk =  WorkEnc::GetSingleInstance();
    _ASSERT( wk._up_pool_of_branches <= sizeof( wk._pool_of_branches ) - size ,"not enough memory" );
    void* result = &wk._pool_of_branches[ wk._up_pool_of_branches ];
    wk._up_pool_of_branches += size;
    return result;
  }

  static void operator delete(void *ptr, size_t size) {
    (void)ptr; (void)size;
    return;
  }

  Branch()
    : _work( &WorkEnc::GetSingleInstance() ){}
};

static  bool compare_branch(Branch* lhs, Branch* rhs) {
  return lhs->_freq_ap < rhs->_freq_ap ? true : false;
}

void  WorkEnc::Export( std::shared_ptr< Pipe::CPipe > pipe_in_,  SerialBytes& sb  , size_t bytesize_ ){
  // export _num_of_huffman_table;
  sb.PushBitsN( 9, _h2c.size() );
  int nn = 0;
  for(const auto& [huffman, code] : _h2c) {
#ifdef VERBOSE
    printf( "nn=%d h:%sb=%ld(len=%d) c:0x%02x=%d\n",nn,huffman.ToStr().c_str(),huffman._code,huffman._len,code,code);
#endif
    huffman.Send( sb );
    sb.PushBitsU8( code );
    ++nn;
  }

  const u32 _byte_size_of_orgin = (u32)bytesize_;
  _ASSERT( _byte_size_of_orgin < (1<<MAX_SIZE_POW2) , "size exceeded" );
#ifdef VERBOSE
  WATCH(_byte_size_of_orgin);
#endif
  sb.PushBitsN(MAX_SIZE_POW2 ,_byte_size_of_orgin );
  u8 reg8 = 0x00;
  int cnt_pop = 0;
  while( pipe_in_->Pop( reg8 )){
#ifdef VERBOSE
 WATCH(reg8);
#endif
    ++cnt_pop;
    const HuffmanCode& hc = _c2h[ reg8 ];
    sb.PushBitsN(hc._len,hc._code);
  }
#ifdef VERBOSE
 WATCH(cnt_pop);
#endif
}

void  WorkEnc::RegisterCode( HuffmanCode huffman_code_ , u8 code_ ){
  _h2c[ huffman_code_ ] = code_;
  _c2h[ code_         ] = huffman_code_;
}

WorkEnc::~WorkEnc(){
  WorkEnc::instance = nullptr;
}

u32 HuffmanCode::Get() const{
  return  _code & ((1<<_len)-1);
}

void  HuffmanCode::PushLeft(){
  _code <<= 1;
  ++_len;
}

void  HuffmanCode::PushRight(){
  _code = (_code<<1)|1;
  ++_len;
}

void  HuffmanCode::Send( SerialBytes& sb ) const {
  // sending u8 len
  u8 len = _len<<3;
  for( int ii=0 ; ii<5 ; ++ii ){
    sb.PushBits( len & (1<<7) ? true : false );
    len <<= 1;
  }

  // sending u$(len) bits
  for( int sft=_len-1 ; sft >= 0 ; --sft ){
    sb.PushBits( _code & (1<<sft) ?  true : false );
  }
}

str32  HuffmanCode::ToStr() const{
  str32 result;
  for( int sft = _len-1 ; sft>=0 ; --sft ){
    result.push_back( (_code >> sft) & 1 ? '1':'0' );
  }
  return result;
}

void  CHuffmanEncoder::SetIn ( std::shared_ptr< Pipe::CPipe > pipe_in_  ){
  _pipe_in = pipe_in_;
}

void  CHuffmanEncoder::SetOut( std::shared_ptr< Pipe::CPipe > pipe_out_ ){
  _pipe_out = pipe_out_;
}

void  CHuffmanEncoder::_Output( std::shared_ptr< Pipe::CPipe > pipe_mem_huf_packed_ ){
  u8 reg8;
  while( pipe_mem_huf_packed_->Pop( reg8 ) ){
    _pipe_out->Push( reg8 );
  }
}

void  CHuffmanEncoder::Encode(){
#ifdef VERBOSE
  PASS();
#endif
  auto pipe_mem_huf_packed = make_shared< CMemBufferPipe >();

  SerialBytes sb;
  //sb.SetOut( _pipe_out );
  sb.SetOut( pipe_mem_huf_packed );

  // export _signature
  sb.PushBitsU8( SIG_GENERAL_HUFFMAN );

  // export _flat
  sb.PushBits( false );

  WorkEnc* work = new WorkEnc;

  // Build frequency of appearance table -> work->_freq_ap[0x100]
  u8 reg8 = 0x00;
  size_t org_bytesize = 0;
  while( _pipe_in->Pop( reg8 )){
    work->_freq_ap[ reg8 ]++;
    ++org_bytesize;
  }
  if( 0 == org_bytesize ){
    sb.Flush();
    _Output( pipe_mem_huf_packed );
    delete work;
    return;
  }
  _pipe_in->SeekPop(0);

  // Build leavs from huffman tree
  list< Branch* > branches;
  for( int nn=0 ; nn<0x100 ; ++nn ){
    if( 0 == work->_freq_ap[nn] ) continue;

    Branch* branch = new Branch;
    branch->_data = nn;
    branch->_freq_ap = work->_freq_ap[nn];
    branches.push_back( branch );
  }
  branches.sort(compare_branch);

#ifdef VERBOSE
  for( const Branch* it : branches ){
    it->Dump();
  }
#endif

  // Build a huffman tree
  const size_t cnt_cmp = branches.size()-1;
  for( size_t nn = 0 ; nn < cnt_cmp ; ++nn ){
    list< Branch* >::iterator it_left = branches.begin();
    list< Branch* >::iterator it_right = std::next( it_left );
#ifdef VERBOSE
    printf( "---------\n");
    (*it_left)->Dump();
    (*it_right)->Dump();
#endif
    Branch* super = new Branch;
    super->_freq_ap = (*it_left)->_freq_ap + (*it_right)->_freq_ap;
    super->_left  = *it_left;
    super->_right = *it_right;
#ifdef VERBOSE
    super->Dump();
#endif
    branches.pop_front();
    branches.pop_front();

    list< Branch* >::iterator it_insert = branches.end();
    for(
      list< Branch* >::iterator it = branches.begin();
      it != branches.end() ;
      ++it
    ){
      if( super->_freq_ap <= (*it)->_freq_ap ){
        it_insert = it;
        break;
      }
    }

    if( it_insert != branches.end() ){
      branches.insert( it_insert , super );
    } else {
      branches.push_back( super );
    }
  }

#ifdef VERBOSE
  {
    list< Branch* >::iterator it_root = branches.begin();
    (*it_root)->DumpRecursive();
  }
#endif

  list< Branch* >::iterator it_root = branches.begin();
  HuffmanCode hcz;  // HuffmanCode zero
  hcz._code = 1;
  (*it_root)->VisitRecursive(hcz);

  work->Export( _pipe_in, sb , org_bytesize );
  sb.Flush();
  if( pipe_mem_huf_packed->Size() < org_bytesize ){
    _Output( pipe_mem_huf_packed );
  } else {
    SerialBytes sb_flat;
    sb_flat.SetOut( _pipe_out );

    // export _signature
    sb_flat.PushBitsU8( SIG_GENERAL_HUFFMAN );

    // export _flat
    sb_flat.PushBits( true );

    // export _byte_size_of_orgin
    sb_flat.PushBitsN(MAX_SIZE_POW2 , org_bytesize );

    // export _reserved
    sb_flat.PushBitsN(3,0);

    _pipe_in->SeekPop(0);
    u8 cc;
    while( _pipe_in->Pop( cc ) ){
      sb_flat.PushBitsU8( cc );
    }
    sb_flat.Flush();
  }
  delete work;
#ifdef VERBOSE
  PASS();
#endif
}

// ------------------------------------------------------------------------------
// huffman decoder
// ------------------------------------------------------------------------------
#define IDX_NIL     (0xffff)
#define N_MAX       (512)
struct WorkDec {
  u16  _gen_idx_branch = 0;
  u16  _left    [ N_MAX ];
  u16  _right   [ N_MAX ];
  u8   _dec_data[ N_MAX ];

  u16 NewBranch(){
    const u16 idx = _gen_idx_branch++;
    _left    [ idx ] =
    _right   [ idx ] = IDX_NIL;
    _dec_data[ idx ] = 0;
    return idx;
  }
};

CHuffmanDecoder::DecodeResult  CHuffmanDecoder::_DoDecodeFlat(){
  _pipe_in->SeekPop(0);
  SerialBytes sb;
  sb.SetIn( _pipe_in );

  // _signature
  const u8 _signature = sb.PopBitsU8();
  if( _signature != SIG_GENERAL_HUFFMAN ){
    return CHuffmanDecoder::DECODE_ERR_INVALID_SIGNATURE;
  }

  bool _flat = sb.PopBits();   // _flat
  if( !_flat ){
    return CHuffmanDecoder::DECODE_INVALID_DATA;
  }

  const u32 _byte_size_of_orgin = sb.PopBitsN( MAX_SIZE_POW2 );

  // _reserved
  sb.PopBitsN( 3 );

  for( size_t ii=0 ; ii<_byte_size_of_orgin ; ++ii ){
    const u8 reg8 = sb.PopBitsU8();
    _pipe_out->Push( reg8 );
  }
  return  CHuffmanDecoder::DECODE_OK;
}

CHuffmanDecoder::DecodeResult  CHuffmanDecoder::Decode(){
  SerialBytes sb;
  sb.SetIn( _pipe_in );

  const u8 _signature = sb.PopBitsU8();
  if( _signature != SIG_GENERAL_HUFFMAN ){
    return CHuffmanDecoder::DECODE_ERR_INVALID_SIGNATURE;
  }

  const u8 _flat = sb.PopBits();
  if( _flat ){
    return  _DoDecodeFlat();
  }

  WorkDec* work       = new WorkDec;
  const u16 idx_root  = work->NewBranch();
  u16   idx_cur       = idx_root;
  u16*  left          = work->_left;
  u16*  right         = work->_right;
  u8*   dec_data      = work->_dec_data;

  const int _num_of_huffman_table = sb.PopBitsN(9);
  for( int nn=0 ; nn<_num_of_huffman_table ; ++nn ){
    const u8  _num_of_bits_huffman_code = sb.PopBitsN(5);
    const u32 _huffman_code             = sb.PopBitsN( _num_of_bits_huffman_code );
    const u8  _decode_data              = sb.PopBitsU8();

    for( int nc=_num_of_bits_huffman_code-1 ; nc >= 0 ; --nc ){
      u16* idx = (0==((_huffman_code>>nc)&1)) ? &left[ idx_cur ] : &right[ idx_cur ];
      if( IDX_NIL == *idx ){
        idx_cur = work->NewBranch();
        *idx = idx_cur;
      } else {
        idx_cur = *idx;
      }
    }
    dec_data[ idx_cur ] = _decode_data;
    idx_cur = idx_root;

#ifdef VERBOSE
    WATCH(_num_of_huffman_table);
    WATCH(nn);
    WATCH(_num_of_bits_huffman_code);
    WATCH(_huffman_code);
    WATCH(_decode_data);
#endif
  }

  const u32 _byte_size_of_orgin = sb.PopBitsN( MAX_SIZE_POW2 );
#ifdef VERBOSE
  WATCH( _byte_size_of_orgin );
#endif
  u32 size_of_decoded = 0;
  idx_cur = idx_root;
  while( size_of_decoded < _byte_size_of_orgin ){
    if( work->_left[ idx_cur ] == IDX_NIL && work->_right[ idx_cur ] == IDX_NIL ){
      if( _pipe_out ){
        _pipe_out->Push( work->_dec_data[ idx_cur ] );
      }
      ++size_of_decoded;
      idx_cur = idx_root;
    } else {
      idx_cur = (0 == sb.PopBits()) ? work->_left[ idx_cur ] : work->_right[ idx_cur ];
    }
  }
  delete work;
  return  CHuffmanDecoder::DECODE_OK;
}

void  CHuffmanDecoder::SetIn ( std::shared_ptr< Pipe::CPipe > pipe_in_  ){
  _pipe_in = pipe_in_;
}

void  CHuffmanDecoder::SetOut( std::shared_ptr< Pipe::CPipe > pipe_out_ ){
  _pipe_out = pipe_out_;
}
