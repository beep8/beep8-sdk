/**
 * @file os.h
 * @brief OS definitions and system call interfaces for the BEEP-8 system.
 * 
 * This file contains the definitions and system call interfaces for the BEEP-8 operating system.
 * It includes type definitions, constants for scheduling policies and clock IDs, system call
 * enumerations, error codes, and function prototypes for system operations.
 * 
 * Typically, users do not need to use this file directly. It is primarily used by the BEEP-8 
 * bootloader to manage OS-level functionalities such as process and semaphore handling, 
 * scheduling, and time management.
 * 
 */
#pragma once

#ifdef  __cplusplus
extern  "C" {
#endif

#include <stddef.h>
#include <b8/type.h>

typedef u32 b8OsPid;  // process id
#define B8_OS_INVALID_PID (0)

typedef u32 b8OsSid;  // semaphore id
#define B8_OS_INVALID_SID (0)

typedef u64 b8OsCpuCycles;

// scheduling policies
#define B8_OS_SCHED_FIFO                1  // FIFO priority scheduling policy
#define B8_OS_SCHED_RR                  2  // Round robin scheduling policy
#define B8_OS_SCHED_SPORADIC            3  // Sporadic scheduling policy
#define B8_OS_SCHED_IRQ                 4  // Irq handler scheduling policy
#define B8_OS_SCHED_OTHER               5  // Not supported

#define B8_OS_NOT_USING_IRQ (0xffff)

#define B8_OS_SEM_WAIT       (0)
#define B8_OS_SEM_TRYWAIT    (1)
#define B8_OS_SEM_TIMEDWAIT  (2)

#define B8_OS_CLOCKID_REALTIME              (0x10)
#define B8_OS_CLOCKID_MONOTONIC             (0x11)
#define B8_OS_CLOCKID_PROCESS_CPUTIME_ID    (0x12)  // Not supported
#define B8_OS_CLOCKID_THREAD_CPUTIME_ID     (0x13)  // Not supported

// Maximum value the semaphore can have.
#define SEM_VALUE_MAX (32767)
typedef enum {
  B8_OS_SYSCALL_NULL = 0,

  B8_OS_SYSCALL_GET_BRIDGE,

  /*
    in:
      [0] = B8_OS_SYSCALL_SCHED_YIELD
  */
  B8_OS_SYSCALL_SCHED_YIELD,

  /*
    in:
      [0] = B8_OS_SYSCALL_SCHED_SLEEP
      [1] = u32     SleepTime.HI [us]
      [2] = u32     SleepTime.LO [us]
  */
  B8_OS_SYSCALL_SCHED_SLEEP,

  /*
    in:
      [0] = B8_OS_SYSCALL_THREAD_CREATE
      [1] = void*   StackAddr
      [2] = size_t  StackSize
      [3] = void*   StartRoutine
      [4] = void*   Arg
      [5] = u32     SchedulingPolicy B8_OS_SCHED_*
      [6] = u32     IrqNo

    out:
      b8OsBridgeUsr2Svc::ret_pid
  */
  B8_OS_SYSCALL_THREAD_CREATE,

  /*
    in:
      [0] = B8_OS_SYSCALL_EXIT
  */
  B8_OS_SYSCALL_EXIT,

  /*
    in:
      [0] = B8_OS_SYSCALL_SET_ERRNO
      [1] = int errcode
  */
  B8_OS_SYSCALL_SET_ERRNO,

  /*
    in:
      [0] = B8_OS_SYSCALL_GET_ERRNO
  */
  B8_OS_SYSCALL_GET_ERRNO,

  /*
    in:
      [0] = B8_OS_SYSCALL_SEM_INIT
      [1] = int  pshared (DON'T CARE)
      [2] = int  value
  */
  B8_OS_SYSCALL_SEM_INIT,

  /*
    in:
      [0] = B8_OS_SYSCALL_SEM_POST
      [1] = b8OsSid  SemaphoreID
  */
  B8_OS_SYSCALL_SEM_POST,

  /*
    in:
      [0] = B8_OS_SYSCALL_SEM_WAIT
      [1] = b8OsSid  SemaphoreID
      [2] =
        B8_OS_SEM_WAIT       : sem_wait()
        B8_OS_SEM_TRYWAIT    : sem_trywait()
        B8_OS_SEM_TIMEDWAIT  : sem_timedwait()
  */
  B8_OS_SYSCALL_SEM_WAIT,

  /*
    in:
      [0] = B8_OS_SYSCALL_SEM_GETVALUE
      [1] = b8OsSid SemaphoreID

    out:
      b8OsBridgeUsr2Svc::ret_semcount
  */
  B8_OS_SYSCALL_SEM_GETVALUE,

  B8_OS_SYSCALL_CLOCK_GETRES,

  /*
    in:
      [0] = B8_OS_SYSCALL_CLOCK_GETTIME
      [1] = B8_OS_CLOCKID_*

    out:
      b8OsBridgeUsr2Svc::tv_sec  : sec
      b8OsBridgeUsr2Svc::tv_nsec : nsec
  */
  B8_OS_SYSCALL_CLOCK_GETTIME,

  /*
    in:
      [0] = B8_OS_SYSCALL_CLOCK_SETTIME
      [1] = sec
      [2] = nsec
  */
  B8_OS_SYSCALL_CLOCK_SETTIME,

  /* --- */
  B8_OS_SYSCALL_MAX,
} b8OsSysCallNum;

typedef enum {
  B8_OS_SYSCALL_THREAD_CREATE_PID=1,
  B8_OS_SYSCALL_SEM_INIT_SID=1,
  B8_OS_SYSCALL_SEM_GETVALUE_RESULT=1,
} b8OsSysCallResultIdx;

typedef enum {
  B8_OS_OK = 0,

  ERRBEGIN = 1000,
  #if 0
  ERRPERM,      // "Operation not permitted"
  ERRIO,        // "I/O error"
  ERRAGAIN,     // "Try again"
  ERRNOMEM,     // "Out of memory"
  ERRACCES,     // "Permission denied"
  ERRINVAL,     // "Invalid argument"
  ERROVFLOW,    // "The semaphore value exceeds SEM_VALUE_MAX"
  ERRMPROC,     // "Too many process"
  ERRNOSPC,     // "No space left on device"
  ERRTIMEDOUT,  // "Connection timed out"
  ERRNOTSUP,    // "Not supported"
  ERRNOSYS,     // "Function not implemented"
  ERRENFILE,    // "Too many open files in system"
  ERRBADF,      // "Bad file number"
  ERREMLINK,    // "Too many links"
  ERRNOHEAP,    // "Out of heap memory"
  #endif

  /* --- */
  ERREND
} b8OsSysErrNo;

typedef struct _b8OsBridgeUsr2Svc{
  u32       signature;
  b8OsPid   pid;
  b8OsPid   ret_pid;
  b8OsSid   ret_sid;
  int       ret_semcount;
  u64       tv_sec;
  u32       tv_nsec;
  s32       errcode;
} b8OsBridgeUsr2Svc;
extern  b8OsBridgeUsr2Svc* b8OsGetBridge(void);

typedef struct _sem_t sem_t;

extern b8OsBridgeUsr2Svc* b8OsSysCall( b8OsSysCallNum syscall,u32 arg0,u32 arg1,u32 arg2,u32 arg3,u32 arg4,u32 arg5);
typedef struct _b8OsConfig{
  void*         StackTop;
  size_t        StackSize;

  b8OsCpuCycles CpuCyclesPerSec;

  //  100Hz:10ms = 10*1000us = 10000
  // 1000Hz: 1ms =  1*1000us =  1000
  u32           UsecPerTick;

  sem_t*        sem_heap;

  // driver
  int   (*ArchDriverGetTimerIrq)(u16* irq);
  int   (*ArchDriverOnStartTimer)(u32 hz);
  int   (*ArchDriverOnStartCycleCnt)(void);
  void  (*ArchDriverGetCycle)(u32* cyccnt );
  void  (*ArchDriverGetCycleAndClear)(u32* cyccnt );
  void  (*ArchDriverPicDistributor)(u8 enable );
  void  (*ArchDriverGetClockTime)( u64* clock );
} b8OsConfig;
extern  int  b8OsReset( b8OsConfig* cfg_ );
extern  int  b8OsIsRunning(void);

#ifdef  __cplusplus
}
#endif