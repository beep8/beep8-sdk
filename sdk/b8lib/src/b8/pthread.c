#include <stdio.h>
#include <b8/pthread.h>
#include <b8/os.h>
#include <b8/errno.h>
#include <b8/assert.h>
#include <b8/sys.h>
#include <string.h>
#include <sys/errno.h>

typedef struct _Cast {
  union {
    s32     _s32;
    u32     _u32;
    void*   _p32;
  }data;
} Cast;

static  u32   _CastPtr(void* ptr ){
  Cast cast;
  cast.data._p32 = ptr;
  return cast.data._u32;
}

int pthread_create(
  pthread_t* thread,
  const pthread_attr_t* attr,
  pthread_startroutine_t startroutine,
  pthread_addr_t arg
){
  if( !attr ){
    return  EINVAL;
  }

  int policy = SCHED_RR;
  switch( attr->policy ){
    case  SCHED_FIFO:
      policy = B8_OS_SCHED_FIFO;
      break;
    case  SCHED_RR:
      policy = B8_OS_SCHED_RR;
      break;
    case  SCHED_SPORADIC:
      return  EINVAL;
    case  SCHED_IRQ:
      policy = B8_OS_SCHED_IRQ;
      break;
  }

  b8OsBridgeUsr2Svc* bridge = b8OsSysCall(
    B8_OS_SYSCALL_THREAD_CREATE,
    _CastPtr( attr->stackaddr ),
    attr->stacksize,
    _CastPtr( startroutine),
    _CastPtr( arg ),
    policy,
    attr->irq_no
  );
  *thread = bridge->ret_pid;
  return  - bridge->errcode;
}

int pthread_attr_init(pthread_attr_t* attr){
  if( !attr ){
    return  EINVAL;
  }
  attr->stackaddr = NULL;
  attr->stacksize = 0x400;
  attr->policy = 0;
  attr->detachstate = 0;
  attr->priority = SCHED_RR;
  attr->irq_no = B8_OS_NOT_USING_IRQ;
  return  0;
}

int pthread_attr_destroy(pthread_attr_t *attr){
  if(!attr){
    return  EINVAL;
  }
  memset(attr, 0, sizeof(pthread_attr_t));
  return  0;
}

int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize){
  if( !attr || stacksize < PTHREAD_STACK_MIN || (stacksize&7)) {
    return  EINVAL;
  }
  attr->stacksize = stacksize;
  return  0;
}

int pthread_attr_getstacksize(
  const pthread_attr_t *attr,
  size_t *stacksize
){
  if (!stacksize){
    return  EINVAL;
  }

  *stacksize = attr->stacksize;
  return  0;
}

int pthread_attr_setstack(
  pthread_attr_t *attr,
  void *stackaddr,
  long stacksize
){
  if (!attr || !stackaddr || stacksize < PTHREAD_STACK_MIN) {
    return  EINVAL;
  }
  attr->stackaddr = stackaddr;
  attr->stacksize = stacksize;
  return  0;
}

int pthread_attr_getstack(const pthread_attr_t *attr, void **stackaddr, long *stacksize){
  if (!attr || !stackaddr || !stacksize){
    return  EINVAL;
  }
  *stackaddr = attr->stackaddr;
  *stacksize = attr->stacksize;
  return  0;
}

int pthread_getschedparam(pthread_t thread, int* policy, struct sched_param* param){
  (void)thread;  // Mark thread as unused
  (void)policy;  // Mark policy as unused
  (void)param;   // Mark param as unused
  return  ENOSYS;
}

int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param* param){
  (void)thread;  // Mark thread as unused
  (void)policy;  // Mark policy as unused
  (void)param;   // Mark param as unused
  return  ENOSYS;  
}

int  pthread_detach(pthread_t thread){
  (void)thread;
  return  ENOSYS;
}

int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy){
  if( !attr ){
    return  EINVAL;
  }
  attr->policy = policy;
  return  0;
}

int pthread_attr_getschedpolicy(
  const pthread_attr_t* attr,
  int* policy
){
  if (!attr || !policy) {
    return  EINVAL;
  }
  *policy = attr->policy;
  return  0;
}

int pthread_attr_setschedparam(
  pthread_attr_t *attr,
  const struct sched_param *param
){
  if (!attr || !param) {
    return  EINVAL;
  }
  attr->priority = (short)param->sched_priority;
  return  0;
}

int pthread_attr_getschedparam(
  const pthread_attr_t *attr,
  struct sched_param *param
){
  if (!attr || !param) {
    return  EINVAL;
  }

  param->sched_priority = (int)attr->priority;
  return  0;
}

int pthread_attr_setdetachstate(
  pthread_attr_t* attr,
  int detachstate
){
  if(!attr || (detachstate != PTHREAD_CREATE_DETACHED && detachstate != PTHREAD_CREATE_JOINABLE)) {
    return  EINVAL;
  }
  attr->detachstate = detachstate;
  return  0;
}

int pthread_attr_getdetachstate(
  const pthread_attr_t *attr,
  int* detachstate
){
  if(!attr || !detachstate){
    return  EINVAL;
  }
  *detachstate = attr->detachstate;
  return  0;
}

void pthread_exit(pthread_addr_t value){
  (void)value;
  while(1) b8OsSysCall( B8_OS_SYSCALL_SCHED_SLEEP,0,0xffffffff,0,0,0,0 );
}

int  pthread_cancel(pthread_t thread){
  (void)thread;
  return  ENOSYS;
}

int  pthread_join(pthread_t thread, pthread_addr_t *value){
  (void)thread;
  (void)value;
  return  ENOSYS;
}

int pthread_yield(void){
  b8OsBridgeUsr2Svc* bridge = b8OsSysCall( B8_OS_SYSCALL_SCHED_YIELD,0,0,0,0,0,0);
  return  - bridge->errcode;
}

pthread_t pthread_self(void){
  b8OsBridgeUsr2Svc* bridge = b8OsSysCall( B8_OS_SYSCALL_GET_BRIDGE,0,0,0,0,0,0);
  return bridge->pid;
}
