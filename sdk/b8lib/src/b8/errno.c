#include <b8/os.h>
#include <sys/errno.h>

int   set_errno(int errcode){
  if( ! b8OsIsRunning() ) return 0;
  if( errcode < 0 ){
    set_errno( EINVAL );
    return -1;
  }
  b8OsBridgeUsr2Svc* bridge = b8OsSysCall(B8_OS_SYSCALL_SET_ERRNO,-errcode,0,0,0,0,0);
  return bridge->errcode;
}

int   get_errno(void){
  if( ! b8OsIsRunning() ) return 0;
  b8OsBridgeUsr2Svc* bridge = b8OsSysCall(B8_OS_SYSCALL_GET_ERRNO,0,0,0,0,0,0);
  return - bridge->errcode;
}
