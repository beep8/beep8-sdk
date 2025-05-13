#include <b8/type.h>
#include <b8/assert.h>
#include <stdio.h>

using	namespace	std;
namespace	{
  union	ForceCast {
    u32 	aU32;
    u32* 	pU32;
    u8*		pU8;
    char*	pChar;
    s32		aS32;
    void*	pVoid;
  };

  union	ForceCast16 {
    u16		aU16;
    s16		aS16;
  };

  static	ForceCast 	fc;
  static	ForceCast16	fc_16;

  inline	void	SAFE_ASSERT() {
    _ASSERT( 4 == sizeof( void* ) , "sizeof( void* ) isn't 4" );
  }

}	// local namespace

u32		ForceCast_u32( void* a ) {
	SAFE_ASSERT();
	fc.pVoid = a;
	return	fc.aU32;
}

u8*		ForceCast_pu8( u32 a ) {
	SAFE_ASSERT();
	fc.aU32  = a;
	return	fc.pU8;
}

u32		ForceCast_u32( u8* a ) {
	SAFE_ASSERT();
	fc.pU8 = a;
	return	fc.aU32;
}

u32		ForceCast_u32( char* a ) {
	SAFE_ASSERT();
	fc.pChar = a;
	return	fc.aU32;
}

u32		ForceCast_u32( u32* a ) {
	SAFE_ASSERT();
	fc.pU32 = a;
	return	fc.aU32;
}

u32		ForceCast_u32( s16 a ) {
	SAFE_ASSERT();
	fc_16.aS16 = a;
	u32 lu32 = (u32)( fc_16.aU16 );
	return	lu32;
}

s16		ForceCast_s16( u32 a ) {
	SAFE_ASSERT();
	fc.aU32 = a;
	return	(s16)fc.aS32;
}

s32		ForceCast_s32( u32 a ) {
	fc.aU32 = a;
	return	fc.aS32;
}