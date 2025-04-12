#include <beep8.h>
#include <sys/types.h>
#include <time.h>
#include <sys/errno.h>

unsigned sleep(unsigned seconds) {
  u64 usec = (u64)seconds;
  usec *= 1000000;
  b8OsBridgeUsr2Svc* bridge = b8OsSysCall( B8_OS_SYSCALL_SCHED_SLEEP,(u32)(usec>>32),usec&0xffffffff,0,0,0,0);
  return  bridge->errcode;
}

int usleep(useconds_t useconds) {
  b8OsBridgeUsr2Svc* bridge = b8OsSysCall( B8_OS_SYSCALL_SCHED_SLEEP,0,useconds,0,0,0,0 );
  return  bridge->errcode;
}

int getpid(void) {
  b8OsBridgeUsr2Svc* bridge = b8OsSysCall( B8_OS_SYSCALL_GET_BRIDGE,0,0,0,0,0,0);
  return bridge->pid;
}

static  int _clock_id_conv( clockid_t clk_id , u32* os_clk_id ){
  switch( clk_id ){
    case  CLOCK_REALTIME_COARSE:
      *os_clk_id = B8_OS_CLOCKID_REALTIME;
      return 0;

    case  CLOCK_MONOTONIC:
    case  CLOCK_MONOTONIC_COARSE:
    case  CLOCK_MONOTONIC_RAW:
    case  CLOCK_BOOTTIME:
      *os_clk_id = B8_OS_CLOCKID_MONOTONIC;
      return 0;

    case  CLOCK_PROCESS_CPUTIME_ID:
      *os_clk_id = B8_OS_CLOCKID_PROCESS_CPUTIME_ID;
      return 0;

    case  CLOCK_THREAD_CPUTIME_ID:
      *os_clk_id = B8_OS_CLOCKID_THREAD_CPUTIME_ID;
      return 0;
  }
  return  set_errno( EINVAL );
}

int clock_getres(clockid_t clk_id, struct timespec *res){
  u32 os_clk_id = 0;
  int ret = _clock_id_conv( clk_id , &os_clk_id );
  if( ret < 0 ) return ret;

  b8OsBridgeUsr2Svc* bridge = b8OsSysCall(
    B8_OS_SYSCALL_CLOCK_GETRES,
    os_clk_id,
    0,0,0,0,0
  );
  res->tv_sec  = bridge->tv_sec;
  res->tv_nsec = bridge->tv_nsec;
  return  bridge->errcode;
}

int clock_gettime(clockid_t clk_id, struct timespec* tp){
  u32 os_clk_id = 0;
  int ret = _clock_id_conv( clk_id , &os_clk_id );
  if( ret < 0 ) return ret;

  b8OsBridgeUsr2Svc* bridge = b8OsSysCall(
    B8_OS_SYSCALL_CLOCK_GETTIME,
    os_clk_id,
    0,0,0,0,0
  );
  tp->tv_sec  = bridge->tv_sec;
  tp->tv_nsec = bridge->tv_nsec;
  return  bridge->errcode;
}

int clock_settime(clockid_t clk_id, const struct timespec *tp){
  b8OsBridgeUsr2Svc* bridge = b8OsSysCall(
    B8_OS_SYSCALL_CLOCK_SETTIME,
    clk_id,
    tp->tv_sec,
    tp->tv_nsec,
    0,0,0);
  return  bridge->errcode;
}
