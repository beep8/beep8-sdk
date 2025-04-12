#include <b8/semaphore.h>
#include <b8/os.h>
#include <b8/errno.h>
#include <sys/errno.h>

int  sem_init(sem_t* sem,int pshared, unsigned int value){
  b8OsBridgeUsr2Svc* bridge = b8OsSysCall(
    B8_OS_SYSCALL_SEM_INIT,
    pshared,
    value,
    0,0,0,0
  );
  sem->sid = bridge->ret_sid;
  return bridge->errcode;
}

int  sem_wait(sem_t* sem){
  b8OsBridgeUsr2Svc* bridge = b8OsSysCall(
    B8_OS_SYSCALL_SEM_WAIT,
    sem->sid,
    B8_OS_SEM_WAIT,
    0,0,0,0
  );
  return bridge->errcode;
}

int sem_trywait(sem_t* sem){
  b8OsBridgeUsr2Svc* bridge = b8OsSysCall(
    B8_OS_SYSCALL_SEM_WAIT,
    sem->sid,
    B8_OS_SEM_TRYWAIT,
    0,0,0,0
  );
  return bridge->errcode;
}

int  sem_timedwait(sem_t* sem, const struct timespec* __abstime){
  b8OsBridgeUsr2Svc* bridge = b8OsSysCall(
    B8_OS_SYSCALL_SEM_WAIT,
    sem->sid,
    B8_OS_SEM_TIMEDWAIT,
    __abstime->tv_sec,
    __abstime->tv_nsec,
    0,0
  );
  return bridge->errcode;
}

int  sem_post(sem_t* sem){
  b8OsBridgeUsr2Svc* bridge = b8OsSysCall( B8_OS_SYSCALL_SEM_POST, sem->sid, 0,0,0,0,0);
  return bridge->errcode;
}

int sem_getvalue(sem_t* sem, int* sval){
  b8OsBridgeUsr2Svc* bridge = b8OsSysCall( B8_OS_SYSCALL_SEM_GETVALUE, sem->sid,0,0,0,0,0);
  *sval = bridge->ret_semcount;
  return bridge->errcode;
}

// Named semaphores are not supported.
sem_t*  sem_open(const char* name, int __oflag, ...){
  (void)name;
  (void)__oflag;
  return  SEM_FAILED;
}

int sem_destroy(sem_t* sem){
  (void)sem;
  set_errno( ENOSYS );
  return -1;
}

int sem_close (sem_t* sem){
  (void)sem;
  set_errno( ENOSYS );
  return  -1;
}

int sem_unlink(const char *name){
  (void)name;
  set_errno( ENOSYS );
  return  -1;
}
