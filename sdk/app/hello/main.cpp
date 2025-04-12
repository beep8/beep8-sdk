#include <stdio.h>
#include <beep8.h>
int main(void){
  //uint32_t* p = (uint32_t*) 7;
  uint32_t* p = NULL;
  *p = 0xdeadbeaf;
  //int x = *p;
  //printf("hello beep8 @2024_0924a x=%08x\n",x);
  return 0;
}
