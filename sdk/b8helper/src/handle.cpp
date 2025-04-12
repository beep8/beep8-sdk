#include <stdio.h>
#include <beep8.h>
#include <handle.h>

#define EXP_2_WIDTH   (3)
#define	EXP_2_HEIGHT  (10)

constexpr u32 W_TABLE = 1 << EXP_2_WIDTH;
constexpr u32 H_TABLE = 1 << EXP_2_HEIGHT;
constexpr u32 H_TABLE_MASK = H_TABLE - 1;
static  u32	_Handle = 1;
static  u32	_Table[ W_TABLE * H_TABLE * 2 ];
union	_StrongCast {
	u32   _tu;
	void* _pVoid;
};

void  Handle_Reset(){
  _Handle = 1;
  for( u32 nn=0 ; nn< sizeof( _Table ) / sizeof( _Table[0] ) ; ++nn ){
    _Table[ nn ] = 0x00000000;
  }
}

void  Handle_Dump(){
  for( u32 yy=0 ; yy<H_TABLE ; ++yy ){
    u32* pr = &_Table[ yy << (EXP_2_WIDTH+1) ];
    bool bAny = false;

    for( u32 xx=0 ; xx<W_TABLE ; ++xx ){
      if( *pr ) {
        printf( "[0x%08lx,0x%08lx] ", *(pr+0) , *(pr+1) );
        bAny = true;
      }
      pr += 2;
    }
    if( bAny )  printf("\n");
  }
}

u32		Handle_Entry( void* pPtr ) {
	const u32 hdl = _Handle++;
	u32* pw = &_Table[ (hdl & H_TABLE_MASK) << (EXP_2_WIDTH+1) ];
  u32 ii = 0;
  for( ; ii < W_TABLE ; ++ii,pw+=2 ){
    if( 0 == *pw )  break;
  }
  _ASSERT( ii < W_TABLE , "not enough work memory in handle.cpp" );

	*pw++ = static_cast< u32 >( hdl );
	_StrongCast scast;
	scast._pVoid = pPtr;
	*pw = scast._tu;
	return	hdl;
}

void* Handle_GetPointer( u32 hdl ) {
  if( 0 == hdl )  return 0;

  u32* pr;
  u32 ii;
  for(
    pr = &_Table[ (hdl & H_TABLE_MASK) << (EXP_2_WIDTH+1) ],
    ii = 0 ;

    ii < W_TABLE ;

    ++ii,
    pr+=2
  ){
		if( *pr == static_cast< u32 >( hdl ) ){
			_StrongCast scast;
			scast._tu = *(pr+1);
			return	scast._pVoid;
    }
  }
	return	0;
}

bool	Handle_IsAlive( u32 hdl ) {
	return	0 != Handle_GetPointer( hdl ) ? true : false;
}

void	Handle_Remove( u32 hdl )
{
	if( 0 == hdl )  return;

  u32* pr;
  u32 ii;
  for(
    pr = &_Table[ (hdl & H_TABLE_MASK) << (EXP_2_WIDTH+1) ],
    ii = 0 ;

    ii < W_TABLE ;

    ++ii,
    pr+=2
  ){
		if( *pr == static_cast< u32 >( hdl ) ){
      *pr = *(pr+1) = 0;
      return;
    }
  }
  _ASSERT( 0 , "invalid handle" );
}
