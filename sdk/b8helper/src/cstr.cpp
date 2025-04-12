#include <cstr.h>

size_t  cstr_strlen( const char* sz_ ){
  if( nullptr == sz_ )  return 0;

  size_t nn = 0;
  while( *sz_++ ) ++nn;
  return nn;
}

int cstr_strcmp(const char *s1, const char *s2) {
  const unsigned char* ss1;
  const unsigned char* ss2;
  for(
    ss1 = (const unsigned char*)s1,
    ss2 = (const unsigned char*)s2
    ;
    *ss1 == *ss2 && *ss1 != '\0'
    ;
    ss1++, ss2++
  ){}
  return *ss1 - *ss2;
}
