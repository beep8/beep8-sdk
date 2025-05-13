#include <stdarg.h>
#include <b8/misc.h>

extern  int _ioctl(int fd, unsigned int cmd, void* arg);

int ioctl(int fd,int request,...){
  va_list ap;
  int res;

  va_start(ap,request);
  res = _ioctl(fd,request,va_arg(ap,void *));
  va_end(ap);
  return res;
}
