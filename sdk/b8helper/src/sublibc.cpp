#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cstdarg>
#include <b8/type.h>
#include <b8/register.h>
#include <sublibc.h>
#include <malloc.h>

  void  print_mallinfo(){
  struct mallinfo mi;
  mi = mallinfo();
  printf("Total non-mmapped bytes (arena):       %d\n", mi.arena);
  printf("# of free chunks (ordblks):            %d\n", mi.ordblks);
  printf("# of free fastbin blocks (smblks):     %d\n", mi.smblks);
  printf("# of mapped regions (hblks):           %d\n", mi.hblks);
  printf("Bytes in mapped regions (hblkhd):      %d\n", mi.hblkhd);
  printf("Max. total allocated space (usmblks):  %d\n", mi.usmblks);
  printf("Free bytes held in fastbins (fsmblks): %d\n", mi.fsmblks);
  printf("Total allocated space (uordblks):      %d\n", mi.uordblks);
  printf("Total free space (fordblks):           %d\n", mi.fordblks);
  printf("Topmost releasable block (keepcost):   %d\n", mi.keepcost);
}

void* mallocz( size_t bytesize_ ){
  void* pp = malloc( bytesize_ );
  if(!pp) return pp;
  memsetz( pp , bytesize_ );
  return pp;
}

std::string fs(const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    std::vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    return std::string(buffer);
}