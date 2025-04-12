#include <beep8.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>

#define CONFIG_N_MAX_THREAD_POW2      (5)
#define CONFIG_BYTESIZE_OF_HEAP       (8*1024)
#define CONFIG_N_MAX_SEMAPHORE_POW2   (6)
#define CONFIG_BYTESIZE_OF_STACK_IDLE_THREAD  (0x100)
#define CONFIG_BYTESIZE_OF_STACK_MAIN_THREAD  (0x2000)

#define N_MAX_THREAD    (1<<CONFIG_N_MAX_THREAD_POW2)
#define N_MAX_SEMAPHORE (1<<CONFIG_N_MAX_SEMAPHORE_POW2)

#define B8_OS_BRIDGE_USR2SVC_SIGNATURE  (0xbeafface)

// arm mode
#define USR_MODE  (0x10)
#define IRQ_MODE  (0x12)
#define SVC_MODE  (0x13)

// arm register label
#define REG_0 (0)
#define REG_13SP (13)
#define REG_15PC (15)
#define REG_PSR (16)
#define REG_MAX (17)

typedef u64 b8OsUsec;         // usec

extern  void  _b8OsSvc2Usr(void);
extern  void  _b8OsIrq2Usr(void);
extern  u32   _b8OsGetCPSR(void);
extern  int   _crt_main(void);
static  void _b8OsGiveBridgeToUsrByPid( b8OsPid pid );

#define KPANIC(expr_, comment_) \
  do { \
    if (!(expr_)) { \
      b8SysPuts( "b8os panic:" );\
      b8SysPuts( __FILE__ );\
      b8SysPuts( " (");\
      b8SysPutNum( __LINE__ );\
      b8SysPuts( ") ");\
      b8SysPuts( __func__ );\
      b8SysPuts( "() ");\
      b8SysPuts( comment_ );\
      b8SysPutCR();\
      asm("hlt");\
    } \
  } while(0)

#define KWATCH(x) \
  do { \
    b8SysPuts( "b8os watch:");\
    b8SysPuts( __FILE__ );\
    b8SysPuts( " (");\
    b8SysPutNum( __LINE__ );\
    b8SysPuts( ") ");\
    b8SysPuts( __func__ );\
    b8SysPuts( "() 0x");\
    b8SysPutHex((x)); \
    b8SysPuts( " =" #x); \
    b8SysPutCR(); \
  } while(0)

// USR_MODE or IRQ_MODE or SVC_MODE
static  u32   _b8OsGetCPSRMode(void){
  return  _b8OsGetCPSR()&31;
}

typedef unsigned int  irqstate_t;
static inline irqstate_t _b8OsIrqEnable(void) {
  KPANIC( _b8OsGetCPSRMode() == USR_MODE , "not usr mode" );
  unsigned int flags;
  unsigned int temp;
  __asm__ __volatile__
    (
     "\tmrs    %0, cpsr\n"
     "\tbic    %1, %0, #128\n"
     "\tmsr    cpsr_c, %1"
     : "=r" (flags), "=r" (temp)
     :
     : "memory");
  return flags;
}

static  int _b8OsIrqUnexpectedIsr(int irq, void* arg){
  (void)arg;

  b8SysPuts( "unexpected irq=0x" );
  b8SysPutHex( irq );
  b8SysPutCR();
  b8SysHalt();
  // It won't get here
  return B8_OS_OK;
}

typedef struct _Cast {
  union {
    s32     _s32;
    u32     _u32;
    void*   _p32;
  }data;
} Cast;

static  u32   _b8OsCastPtr(void* ptr ){
  Cast cast;
  cast.data._p32 = ptr;
  return cast.data._u32;
}

static  void* _b8OsCastU32(u32 uptr ){
  Cast cast;
  cast.data._u32 = uptr;
  return cast.data._p32;
}

typedef struct _Node        Node;
typedef struct _List        List;
typedef struct _Tcb         Tcb;
typedef struct _Semaphore   Semaphore;
typedef struct _ReqSchedule ReqSchedule;

typedef enum {
  TS_NOT_YET_INIT,
  TS_READY,
} TcbStatus;

typedef enum {
  TWF_NOTHING,
  TWF_SEMAPHORE,
  TWF_YIELD,
  TWF_TIMER,
  TWF_IRQ
} TcbWaitingFor;

struct _Tcb {
  u32       reg[ REG_MAX ];
  b8OsPid   pid;
  void*     (*start_routine)(void*);
  void*     arg;
  void*     stack_addr;
  size_t    stack_size;
  b8OsSid   sid_wait;
  TcbStatus status;
  TcbWaitingFor waiting_for;
  u8        mode_when_saved;  // MODE_SVC or MODE_IRQ
  u8        scheduling_policy;
  u16       irq;
  b8OsUsec  wake_up_time;
};

struct _Semaphore {
  b8OsSid sid;
  int     semcount;
};

typedef int (*b8IrqHandler)(int irq, void* arg);

typedef struct _b8OsIrqInfo {
  b8IrqHandler  handler;  // Address of the interrupt handler
  void* arg;              // The argument provided to the interrupt handler
} b8OsIrqInfo;

#define REQ_SCHEDULE_NONE                               (0)
#define REQ_SCHEDULE_REGULAR                            (1<<1)
#define REQ_SCHEDULE_SEMAPHORE_WAIT                     (1<<2)
#define REQ_SCHEDULE_AWAKE_THREAD_WAITING_FOR_SEMAPHORE (1<<3)
#define REQ_SCHEDULE_AWAKE_THREAD_WAITING_FOR_IRQ       (1<<4)
#define REQ_SCHEDULE_YIELD                              (1<<5)
#define REQ_SCHEDULE_YIELD_TIME                         (1<<6)
#define REQ_SCHEDULE_EXIT_THREAD                        (1<<7)

struct _ReqSchedule{
  u16       req; // REQ_SCHEDULE_*
  b8OsSid   sid;
  b8OsPid   pid;
  u16       irq;
  b8OsUsec  sleep_time;
};
static  void  ReqScheduleClear(ReqSchedule* rs){
  memset( rs , 0 , sizeof(ReqSchedule));
  rs->req = REQ_SCHEDULE_NONE;
  rs->sid = B8_OS_INVALID_SID;
  rs->pid = B8_OS_INVALID_PID;
  rs->irq = B8_OS_NOT_USING_IRQ;
  rs->sleep_time = 0;
}

static  Tcb*  _b8OsGetCurrentTcb(void);
static  Tcb*  _b8OsGetTcb( b8OsPid pid );
static  int   _b8OsIrqDispatch(int irq, void* arg);
static  void  _b8OsProcessScheduler(ReqSchedule* rs);
static  b8OsBridgeUsr2Svc*  TcbGetBridge( b8OsPid pid );
static  int   _b8OsIrqAttach(int irq,b8IrqHandler isr,void* arg);
static  void  _b8OsGiveBridgeToUsr(void);

static  b8OsIrqInfo _IrqInfo[ B8_IRQ_NUM_OF_INTERRUPTS ];
static  b8OsPid     _IdlePid;
static  b8OsPid     _CurrentPid;
static  uint32_t    _AccThread;
static  uint32_t    _AccSemaphore;
static  Tcb         _TaskControlBlocks[ N_MAX_THREAD ];
static  Semaphore   _Semaphores[ N_MAX_SEMAPHORE ];
static  u8          _MemoryPool[ CONFIG_BYTESIZE_OF_HEAP ];
static  u32         _UpMemoryPool;
static  List*       _ReadyTasksList;
static  List*       _WaitingTasksList;
static  List*       _ZombieTasksList;
static  b8OsConfig  _Config;
static  size_t      _UpStackPool;
static  b8OsUsec    _AccumelatedTime;
static  u64         _UsPerCpuCycleFixed8;
static  u16         _IrqTimer;
static  u16         _IrqDispatched;
static  u64         _CycCnt;
static  u64         _UnixEpochTimeMilliseconds;
static  u64         _UnixEpochTimeCycles;
static  b8OsUsec    _UnixEpochTimeMicroseconds;
static  u32         _ClockResolutionNs;
static  u8          _IsRunning = 0;

u32 b8OsUsrContext   [ REG_MAX ];
u32 b8OsSysCallArgs  [ 1+6 ];

static  void  _b8OsSwitchBackToUsr(void){
  switch( _b8OsGetCPSRMode() ){
    case  IRQ_MODE:{
      B8_PIC_EOIR = _IrqDispatched;
      _IrqDispatched = 1023;
      _b8OsIrq2Usr();
    }break;
    case  SVC_MODE:{
      _b8OsSvc2Usr();
    }break;
    default:  KPANIC(0,"unknown mode"); break;
  }
  // It won't get here
}

static  void*   _b8OsKernelMalloc( size_t byte_ ){
  byte_ += 3;
  byte_ -= byte_ & 3;

  u32 up = _UpMemoryPool;
  _UpMemoryPool += byte_;
  KPANIC( _UpMemoryPool <= CONFIG_BYTESIZE_OF_HEAP  , "out of memory" );
  return &_MemoryPool[ up ];
}

static  void*   _b8OsStackAlloc( size_t byte_ ){
  byte_ += 7;
  byte_ -= byte_ & 7;

  if( _UpStackPool + byte_ > _Config.StackSize )  return NULL;

  _UpStackPool += byte_;
  return _Config.StackTop + _UpStackPool;
}

struct _Node {
  Node*   _next;
  Node*   _prev;
  b8OsPid pid;
};
static  void  NodeClear(Node* node){
  node->_next = node->_prev = NULL;
  node->pid = B8_OS_INVALID_PID;
}

struct _List {
  u16   _size;
  u16   _maxnum;
  Node* _node_array;
};

static  Node*   ListHead(List* list);
static  Node*   ListEnd(List* list);
static  Node*   ListBegin(List* list);
static  Node*   ListAllocNode(List* list);
static  void    ListPushBack(List* list,uint32_t x_ );
static  Node*   ListErase(List* list,uint32_t x_ );
static  Node*   ListBack(List* list);
static  size_t  ListSize(List* list);
static  List*   NewList( size_t maxnum_ );
static  Node*   ListAt(List* list,u32 nn);

/*
  [head]->[begin]->[]->...[]->[end]
*/
static  Node* ListHead(List* list){
  return  &(list->_node_array[0]);
}

static  Node* ListEnd(List* list){
  return  &(list->_node_array[ list->_maxnum+2-1 ]);
}

static  Node* ListBegin(List* list){
  return  ListHead(list)->_next;
}

static  Node* ListAllocNode(List* list){
  for( u16 nn=0 ; nn<= list->_maxnum ; ++nn ){
    Node* node = &(list->_node_array[ nn+1 ]);
    if( node->_next ) continue;
    if( node->_prev ) continue;
    return node;
  }
  return NULL;
}

Node*   ListAt(List* list,u32 nn){
  if( nn >= ListSize(list) )  return NULL;
  Node* node = ListBegin( list );
  for( u32 ii=0 ; ii<nn ; ++ii ){
    node = node->_next;
  }
  return node;
}

static  void  ListPushBack(List* list,uint32_t x_ ){
  Node* node = ListAllocNode(list);
  KPANIC(node,"failed alloc node");
  KPANIC( ListSize(list) < list->_maxnum , "too many container" );
  list->_size++;

  Node* end  = ListEnd(list);
  Node* back = ListBack(list);
  back->_next = node;
  end->_prev  = node;
  node->_prev = back;
  node->_next = end;

  node->pid = x_;
}

static  Node* ListErase(List* list,uint32_t x_ ){
  Node* it;
  for(
    it =  ListBegin( list ) ;
    it != ListEnd( list ) ;
    it = it->_next
  ){
    if(it->pid != x_ )  continue;

    Node* prev = it->_prev;
    Node* next = it->_next;
    prev->_next = next;
    next->_prev = prev;
    list->_size--;

    NodeClear( it );
    return next;
  }
  return it; // = ListEnd()
}

static  Node*  ListBack(List* list){
  return  ListEnd(list)->_prev;
}

static  size_t  ListSize(List* list){
  return  (size_t)list->_size;
}

static  List* NewList( size_t maxnum_ ){
  List* list = _b8OsKernelMalloc( sizeof( List ) );
  list->_size = 0;
  list->_maxnum = maxnum_;
  const u16 array_num = list->_maxnum+2;
  list->_node_array = _b8OsKernelMalloc( array_num * sizeof( Node ));
  for( uint16_t ii=0 ; ii < array_num ; ++ii ){
    NodeClear( &(list->_node_array[ ii ]) );
  }

  Node* head = ListHead( list );
  Node* end = ListEnd( list );

  head->_next = end;
  head->_prev = NULL;

  end->_next = NULL;
  end->_prev = head;

  return list;
}

extern  int usleep(useconds_t useconds);
static  void* _b8IdleThread( void* arg ){
  (void)arg;
  while(1){}
  return NULL;
}

static  void* _b8MainThread( void* arg ){
  b8SysPuts( "_b8MainThread:\n" );

  (void)arg;
  int ret = sem_init(_Config.sem_heap, 0, 1);
  KPANIC( ret >= 0 , "sem_init()" );

  FILE* fpo = fopen("stdout","w");
  KPANIC( fpo  , "failed open stdout" );

  FILE* fpi = fopen("stdin","r");
  KPANIC( fpi  , "failed open stdin" );

  FILE* fpe = fopen("stderr","r");
  KPANIC( fpe  , "failed open stderr" );

  setvbuf(stdout, (char *)NULL, _IONBF, 0);

  _crt_main();
  return NULL;
}

static  void* _b8OsCommonEntryPoint( void* arg ){
  Tcb* tcb_cur = _b8OsGetCurrentTcb();
  tcb_cur->start_routine( arg );
  b8OsSysCall( B8_OS_SYSCALL_EXIT, 0, 0, 0, 0, 0,0);
  KPANIC(0, "Unexpected return from b8OsSysCall");
  return NULL;
}

static  int _b8OsSetErrorInBridge( s16 errcode, b8OsPid pid ){
  b8OsBridgeUsr2Svc* bridge = TcbGetBridge( pid );

  if( errcode == B8_OS_OK ){
    bridge->errcode = B8_OS_OK;
    return bridge->errcode;
  }

  if( bridge->errcode == B8_OS_OK ){
    bridge->errcode = errcode;
  }
  return bridge->errcode;
}

static  int _b8OsSetError( s16 errcode ){
  KPANIC( _b8OsGetCPSRMode() == SVC_MODE , "it must be svc mode" );
  KPANIC( errcode <= 0 , "errcode must be negative or zero" );

  b8OsBridgeUsr2Svc* bridge = TcbGetBridge( _CurrentPid );

  if( errcode == B8_OS_OK ){
    bridge->errcode = B8_OS_OK;
    _b8OsGiveBridgeToUsr();
    return bridge->errcode;
  } else {
    b8SysPuts( "errcode=" );
    b8SysPutNum( errcode );
    b8SysPutCR();
    KPANIC( errcode < 0 , "unknown err code" );
  }

  if( bridge->errcode == B8_OS_OK ){
    bridge->errcode = errcode;
  }

  _b8OsGiveBridgeToUsr();
  return bridge->errcode;
}

static  void  TcbClear( Tcb* tcb ){
  memset( tcb , 0 , sizeof(Tcb) );
  tcb->stack_size = 0x100;
  tcb->status = TS_NOT_YET_INIT;
  tcb->sid_wait = B8_OS_INVALID_SID;
  tcb->mode_when_saved = 0x00;
  tcb->irq = B8_OS_NOT_USING_IRQ;
  tcb->waiting_for = TWF_NOTHING;
  tcb->wake_up_time = 0;
}

static  b8OsBridgeUsr2Svc*  TcbGetBridgeAddr( Tcb* tcb ){
  b8OsBridgeUsr2Svc* bridge = tcb->stack_addr;
  return  bridge;
}

static  b8OsBridgeUsr2Svc*  TcbGetBridge( b8OsPid pid ){
  Tcb* tcb =_b8OsGetTcb( pid );
  KPANIC( tcb , "invalid tcb" );
  b8OsBridgeUsr2Svc* bridge = TcbGetBridgeAddr( tcb );
  KPANIC( bridge->signature == B8_OS_BRIDGE_USR2SVC_SIGNATURE , "invalid bridge" );
  return bridge;
}

static  void  TcbInit( Tcb* tcb ){
  KPANIC( tcb->status == TS_NOT_YET_INIT , "already initialized" );
  tcb->reg[ REG_0    ] = _b8OsCastPtr( tcb->arg );
  tcb->reg[ REG_13SP ] = _b8OsCastPtr( tcb->stack_addr );

  Cast cast;
  cast.data._p32 = _b8OsCommonEntryPoint;
  tcb->reg[ REG_15PC ] = cast.data._u32 + 4;
  tcb->mode_when_saved = IRQ_MODE;
  tcb->reg[ REG_PSR  ] = USR_MODE;
  tcb->status = TS_READY;
}

static  void  UsrContext2Tcb( Tcb* tcb ){
  tcb->mode_when_saved = _b8OsGetCPSRMode();
  for( u8 reg=REG_0 ; reg < REG_MAX ; ++reg ){
    tcb->reg[ reg ] = b8OsUsrContext[ reg ];
  }
}

static  void  Tcb2UsrContext( Tcb* tcb ){
  for( u8 reg=REG_0 ; reg < REG_MAX ; ++reg ){
    if( reg == REG_15PC ){
      const u32 cpsr_mode =_b8OsGetCPSRMode();
      if( cpsr_mode != tcb->mode_when_saved ){
        if(
          tcb->mode_when_saved == SVC_MODE &&
          cpsr_mode == IRQ_MODE
        ){
          b8OsUsrContext[ reg ] = tcb->reg[ reg ] + 4;
        } else if (
          tcb->mode_when_saved == IRQ_MODE &&
          cpsr_mode == SVC_MODE
        ){
          b8OsUsrContext[ reg ] = tcb->reg[ reg ] - 4;
        } else {
          KPANIC(0,"unknown cpsr_mode");
        }
      } else {
        b8OsUsrContext[ reg ] = tcb->reg[ reg ];
      }
    } else {
      b8OsUsrContext[ reg ] = tcb->reg[ reg ];
    }
  }
}

static  b8OsPid _b8OsAllocTcb(void){
  for( size_t nn=0 ; nn<N_MAX_THREAD ; ++nn ){
    if( _TaskControlBlocks[ nn ].pid != B8_OS_INVALID_PID )  continue;

    TcbClear( &_TaskControlBlocks[ nn ] );
    _TaskControlBlocks[ nn ].pid = (_AccThread<<16) | (u32)nn;
    ++_AccThread;
    return  _TaskControlBlocks[ nn ].pid;
  }
  return  B8_OS_INVALID_PID;
}

static  void  SemaphoreClear( Semaphore* sem ){
  memset( sem , 0 , sizeof(Semaphore) );
  sem->sid = B8_OS_INVALID_SID;
}

static  Semaphore*  _b8OsGetSemaphore( b8OsSid sid ){
  if( sid == B8_OS_INVALID_SID ) return NULL;
  const u32 idx = sid & 0x3fff;
  if( _Semaphores[ idx ].sid != sid ) return NULL;
  return &_Semaphores[ idx ];
}

static  b8OsSid _b8OsAllocSemaphore(int value){
  for( size_t nn=0 ; nn<N_MAX_SEMAPHORE ; ++nn ){
    if( _Semaphores[ nn ].sid != B8_OS_INVALID_SID )  continue;

    Semaphore* sem = &_Semaphores[ nn ];
    SemaphoreClear( sem );
    sem->semcount = value;
    sem->sid = (_AccSemaphore<<14) | (u32)nn;
    ++_AccSemaphore;
    return sem->sid;
  }
  _b8OsSetError(-EINVAL);
  return  B8_OS_INVALID_SID;
}

static  void  _b8OsSemaphoreGetValue( b8OsSid sid , int* value ){
  Semaphore* sem = _b8OsGetSemaphore( sid );
  if( NULL == sem ){
    _b8OsSetError(-EINVAL);
    return;
  }
  *value = sem->semcount;
}

static  void  _b8OsSemaphoreWait( b8OsSid sid , u8 wait_type , b8OsUsec wake_up_time ){
  Semaphore* sem = _b8OsGetSemaphore( sid );
  if( NULL == sem ){
    _b8OsSetError(-EINVAL);
    return;
  }

  Tcb* tcb_cur = _b8OsGetCurrentTcb();
  tcb_cur->wake_up_time = wake_up_time;
  if( sem->semcount > 0 ){
    sem->semcount--;
    tcb_cur->sid_wait = B8_OS_INVALID_SID;
    _b8OsSetError( 0 );
    return;
  } else {
    if( wait_type == B8_OS_SEM_TRYWAIT ){
      _b8OsSetError(-EAGAIN);
      return;
    }

    sem->semcount--;
    tcb_cur->sid_wait = sid;
    _b8OsSetError( 0 );

    // block thread to wait for semaphore
    ReqSchedule rs;
    ReqScheduleClear( &rs );
    rs.req = REQ_SCHEDULE_SEMAPHORE_WAIT;
    _b8OsProcessScheduler( &rs );
    _b8OsSwitchBackToUsr();
    // It won't get here
  }
}

static  void  _b8OsSemaphorePost( b8OsSid sid ){
  Semaphore* sem = _b8OsGetSemaphore( sid );
  if( NULL == sem ){
    _b8OsSetError(-EINVAL);
    return;
  }

  if (sem->semcount >= SEM_VALUE_MAX){
    _b8OsSetError(-EOVERFLOW);
    return;
  }

  sem->semcount++;
  // There must be some task waiting for the semaphore.
  if(sem->semcount <= 0){
    ReqSchedule rs;
    ReqScheduleClear( &rs );
    rs.sid = sid;
    rs.req = REQ_SCHEDULE_AWAKE_THREAD_WAITING_FOR_SEMAPHORE;
    _b8OsProcessScheduler( &rs );
    _b8OsSwitchBackToUsr();
  }
}

static  Tcb*  _b8OsGetTcb( b8OsPid pid ){
  if( pid == B8_OS_INVALID_PID ) return NULL;
  const u32 idx = pid & 0xffff;
  if( _TaskControlBlocks[ idx ].pid != pid ) return NULL;
  return &_TaskControlBlocks[ idx ];
}

static  int  _b8OsThreadCreate(
  b8OsPid*  ppid ,
  void*     StackAddr,
  size_t    StackSize,
  void*     (*StartRoutine)(void*),
  void*     Arg,
  u32       SchedulingPolicy,
  u32       IrqNo
){
  if( IrqNo != B8_OS_NOT_USING_IRQ ){
    int ret = _b8OsIrqAttach(IrqNo,_b8OsIrqDispatch,NULL);
    if( ret < 0 ) return ret;
  }

  *ppid = _b8OsAllocTcb();
  Tcb* tcb = _b8OsGetTcb( *ppid );
  if( NULL == tcb ){
    return  _b8OsSetError(-EAGAIN);
  }
  tcb->start_routine = StartRoutine;
  tcb->arg = Arg;
  if( NULL == StackAddr){
    StackAddr = _b8OsStackAlloc( StackSize );
    if( NULL == StackAddr ){
      return  _b8OsSetError(-ENOMEM);
    }
  }
  StackAddr -= sizeof( b8OsBridgeUsr2Svc );
  tcb->stack_addr = StackAddr;
  tcb->stack_size = StackSize;
  tcb->scheduling_policy = SchedulingPolicy;
  tcb->irq = IrqNo;

  ListPushBack( _ReadyTasksList , *ppid );

  b8OsBridgeUsr2Svc* bridge = TcbGetBridgeAddr( tcb );
  bridge->signature = B8_OS_BRIDGE_USR2SVC_SIGNATURE;
  bridge->pid = *ppid;
  bridge->errcode = B8_OS_OK;
  bridge->ret_pid = B8_OS_INVALID_PID;
  bridge->ret_sid = B8_OS_INVALID_SID;
  bridge->ret_semcount = 0;

  return B8_OS_OK;
}

static  Tcb*  _b8OsGetCurrentTcb(void){
  Tcb* tcb_cur = _b8OsGetTcb( _CurrentPid );
  KPANIC( tcb_cur , "no current tcb" );
  return tcb_cur;
}

int  b8OsReset( b8OsConfig* cfg_ ){
  if( cfg_->StackSize < CONFIG_BYTESIZE_OF_STACK_IDLE_THREAD + CONFIG_BYTESIZE_OF_STACK_MAIN_THREAD + 0x400 ){
    return -ENOMEM;
  }

  if( NULL == cfg_->sem_heap )                    return -EINVAL;
  if( NULL == cfg_->ArchDriverGetTimerIrq )       return -EINVAL;
  if( NULL == cfg_->ArchDriverOnStartTimer)       return -EINVAL;
  if( NULL == cfg_->ArchDriverOnStartCycleCnt )   return -EINVAL;
  if( NULL == cfg_->ArchDriverGetCycle )          return -EINVAL;
  if( NULL == cfg_->ArchDriverGetCycleAndClear )  return -EINVAL;
  if( NULL == cfg_->ArchDriverPicDistributor )    return -EINVAL;
  if( NULL == cfg_->ArchDriverGetClockTime )      return -EINVAL;

  _CycCnt = 0;

  int ret = cfg_->ArchDriverGetTimerIrq( &_IrqTimer );
  if( ret < 0 ) return ret;

  b8SysPuts( "\nb8Os Build:" __DATE__ " Time:" __TIME__ "\n" );
  b8SysPuts( "CPU:" );
  b8SysPutNum( cfg_->CpuCyclesPerSec );
  b8SysPuts( " Hz\n" );

  _UsPerCpuCycleFixed8 = (256*1000000) / cfg_->CpuCyclesPerSec;

  _ClockResolutionNs = (u32)((u64)1000000000 / cfg_->CpuCyclesPerSec);
  KPANIC( _ClockResolutionNs >= 1 , "Not support 1GHz" );

  for( int nn=0 ; nn< B8_IRQ_NUM_OF_INTERRUPTS ; ++nn ){
    _IrqInfo[ nn ].handler = _b8OsIrqUnexpectedIsr;
    _IrqInfo[ nn ].arg = NULL;
  }
  _b8OsIrqEnable();

  memset( b8OsUsrContext , 0 , sizeof(b8OsUsrContext ) );
  _Config = *cfg_;

  _Config.ArchDriverGetClockTime( &_UnixEpochTimeMilliseconds );
  _UnixEpochTimeMicroseconds = _UnixEpochTimeMilliseconds * 1000;
  _UnixEpochTimeCycles = (_UnixEpochTimeMilliseconds/1000) * _Config.CpuCyclesPerSec;

  Cast cast;
  cast.data._p32 = _Config.StackTop;
  if( cast.data._u32 & 7 ){
    _Config.StackSize -= 4;
  }
  cast.data._u32 += 7;
  cast.data._u32 -= (cast.data._u32 & 7);
  _Config.StackTop = cast.data._p32;

  _UpStackPool = 0;

  _AccThread = 1;
  _UpMemoryPool = 0;
  for( size_t nn=0 ; nn<N_MAX_THREAD ; ++nn ){
    _TaskControlBlocks[ nn ].pid = B8_OS_INVALID_PID;
  }

  _AccSemaphore = 1;
  for( size_t nn=0 ; nn<N_MAX_SEMAPHORE ; ++nn ){
    _Semaphores[ nn ].sid = B8_OS_INVALID_SID;
  }

  _ReadyTasksList = NewList( N_MAX_THREAD );
  _WaitingTasksList = NewList( N_MAX_THREAD );
  _ZombieTasksList = NewList( N_MAX_THREAD );

  _AccumelatedTime = 0;
  ret = cfg_->ArchDriverOnStartCycleCnt();
  if( ret < 0 ) return ret;

  ret = _b8OsThreadCreate( &_IdlePid,NULL,CONFIG_BYTESIZE_OF_STACK_IDLE_THREAD, _b8IdleThread , NULL, B8_OS_SCHED_RR , B8_OS_NOT_USING_IRQ );
  if( ret < 0 ) return ret;

  _CurrentPid = _IdlePid;

  b8OsPid main_th;
  ret = _b8OsThreadCreate( &main_th,NULL,CONFIG_BYTESIZE_OF_STACK_MAIN_THREAD, _b8MainThread , NULL, B8_OS_SCHED_RR , B8_OS_NOT_USING_IRQ );
  if( ret < 0 ) return ret;

  b8SysPuts( "b8os heap:" );
  b8SysPutHex( _UpMemoryPool );
  b8SysPutCR();

  ret = _b8OsIrqAttach(_IrqTimer,_b8OsIrqDispatch,NULL);
  if( ret < 0 ) return ret;

  ret = cfg_->ArchDriverOnStartTimer(100);
  if( ret < 0 ) return ret;

  cfg_->ArchDriverPicDistributor(1);
  _IsRunning = 1;

  while(1){}
  // It won't get here
}

static  void _b8OsGiveBridgeToUsrByPid( b8OsPid pid ){
  b8OsBridgeUsr2Svc* bridge = TcbGetBridge( pid );
  KPANIC( bridge->signature == B8_OS_BRIDGE_USR2SVC_SIGNATURE, "invalid bridge" );
  b8OsUsrContext[ REG_0 ] = _b8OsCastPtr( bridge );
}

static  void _b8OsGiveBridgeToUsr(void){
  _b8OsGiveBridgeToUsrByPid( _CurrentPid );
}

static  void _B8_OS_SYSCALL_NULL(void){
  _b8OsGiveBridgeToUsr();
  _b8OsSwitchBackToUsr();
  // It won't get here
}

static  void _B8_OS_SYSCALL_GET_BRIDGE(void){
  _b8OsGiveBridgeToUsr();
  _b8OsSwitchBackToUsr();
  // It won't get here
}

static  void _B8_OS_SYSCALL_SCHED_YIELD(void){
  // block thread to wait for nothing
  ReqSchedule rs;
  ReqScheduleClear( &rs );
  rs.req = REQ_SCHEDULE_YIELD;
  _b8OsProcessScheduler( &rs );
  _b8OsSwitchBackToUsr();
  // It won't get here
}

static  void  _B8_OS_SYSCALL_SCHED_SLEEP(void){
  ReqSchedule rs;
  ReqScheduleClear( &rs );
  rs.req = REQ_SCHEDULE_YIELD_TIME;
  const u64 hi = b8OsSysCallArgs[1];
  const u64 lo = b8OsSysCallArgs[2];
  const u64 sleep_time = (hi<<32) | lo;
  rs.sleep_time = sleep_time;
  _b8OsProcessScheduler( &rs );
  _b8OsSwitchBackToUsr();
  // It won't get here
}

static  void _B8_OS_SYSCALL_THREAD_CREATE(void){
  const size_t StackSize = (size_t)b8OsSysCallArgs[2];
  if( StackSize < 0x200 ){
    _b8OsSetError(-EINVAL);
    _b8OsGiveBridgeToUsr();
    return;
  }

  b8OsPid pid;
  _b8OsThreadCreate(
    &pid,
    _b8OsCastU32( b8OsSysCallArgs[1] ), // void*  StackAddr
    (size_t)b8OsSysCallArgs[2],         // size_t StackSize
    _b8OsCastU32( b8OsSysCallArgs[3] ), // void*  StartRoutine
    _b8OsCastU32( b8OsSysCallArgs[4] ), // void*  arg
    b8OsSysCallArgs[5],                 // u32    SchedulingPolicy
    b8OsSysCallArgs[6]                  // u32    IrqNo
  );

  b8OsBridgeUsr2Svc* bridge = TcbGetBridge( _CurrentPid );
  bridge->ret_pid = pid;
  _b8OsGiveBridgeToUsr();
}

static  void _B8_OS_SYSCALL_EXIT(void){
  ReqSchedule rs;
  ReqScheduleClear( &rs );
  rs.req = REQ_SCHEDULE_EXIT_THREAD;
  rs.pid = _CurrentPid;
  _b8OsProcessScheduler( &rs );
}

static  void _B8_OS_SYSCALL_SET_ERRNO(void){
  const int errcode = b8OsSysCallArgs[1];
  _b8OsSetError( errcode );
  _b8OsGiveBridgeToUsr();
  _b8OsSwitchBackToUsr();
}

static  void _B8_OS_SYSCALL_GET_ERRNO(void){
  _b8OsGiveBridgeToUsr();
  _b8OsSwitchBackToUsr();
}

static  void _B8_OS_SYSCALL_SEM_INIT(void){
  const b8OsSid sid = _b8OsAllocSemaphore( b8OsSysCallArgs[2] );
  b8OsBridgeUsr2Svc* bridge = TcbGetBridge( _CurrentPid );
  bridge->ret_sid = sid;
  _b8OsGiveBridgeToUsr();
}

static  void _B8_OS_SYSCALL_SEM_POST(void){
  const b8OsSid sid = b8OsSysCallArgs[1];
  _b8OsSemaphorePost( sid );
  _b8OsGiveBridgeToUsr();
}

static  void _B8_OS_SYSCALL_SEM_WAIT(void){
  b8OsUsec wake_up_time = 0xffffffffffffffff;
  const b8OsSid sid = b8OsSysCallArgs[1];
  const u32 wait_type = b8OsSysCallArgs[2];
  if( wait_type == B8_OS_SEM_TIMEDWAIT ){
    const u32 nsec = b8OsSysCallArgs[4];
    if( nsec >= 1000000000 ){
      _b8OsSetError(-EINVAL);
      _b8OsGiveBridgeToUsr();
      _b8OsSwitchBackToUsr();
    }

    const u64 sec = (u64)b8OsSysCallArgs[3];
    wake_up_time = sec*1000000 + nsec/1000;
  }
  _b8OsSemaphoreWait( sid , wait_type , wake_up_time );
  _b8OsGiveBridgeToUsr();
  _b8OsSwitchBackToUsr();
}

static  void _B8_OS_SYSCALL_SEM_GETVALUE(void){
  const b8OsSid sid = b8OsSysCallArgs[1];
  int value = 0;
  _b8OsSemaphoreGetValue( sid , &value );
  b8OsBridgeUsr2Svc* bridge = TcbGetBridge( _CurrentPid );
  bridge->ret_semcount = value;
  _b8OsGiveBridgeToUsr();
}

static  void  _b8OsChkClockId(void){
  const u32 clkid = b8OsSysCallArgs[1];
  if( clkid == B8_OS_CLOCKID_PROCESS_CPUTIME_ID ||
      clkid == B8_OS_CLOCKID_THREAD_CPUTIME_ID
  ){
    _b8OsSetError(-EOPNOTSUPP); 
    _b8OsGiveBridgeToUsr();
    _b8OsSwitchBackToUsr();
    // It won't get here
  }
}

static  void  _B8_OS_SYSCALL_CLOCK_GETRES(void){
  _b8OsChkClockId();
  b8OsBridgeUsr2Svc* bridge = TcbGetBridge( _CurrentPid );
  bridge->tv_sec  = 0;
  bridge->tv_nsec = _ClockResolutionNs;
  _b8OsGiveBridgeToUsr();
}

static  void  _B8_OS_SYSCALL_CLOCK_GETTIME(void){
  _b8OsChkClockId();

  const u32 clkid = b8OsSysCallArgs[1];
  u32 cyccnt;
  _Config.ArchDriverGetCycle( &cyccnt );
  static  const u64 ns = 1000000000UL;
  const u64 BaseCycle = (clkid == B8_OS_CLOCKID_REALTIME) ? _UnixEpochTimeCycles : 0;
  const u64 _CurCycCnt = BaseCycle + _CycCnt + (u64)cyccnt;
  b8OsBridgeUsr2Svc* bridge = TcbGetBridge( _CurrentPid );
  bridge->tv_sec = _CurCycCnt / _Config.CpuCyclesPerSec;
  bridge->tv_nsec = (u32) (((_CurCycCnt % _Config.CpuCyclesPerSec) * ns) / _Config.CpuCyclesPerSec);

  _b8OsGiveBridgeToUsr();
}

static  void  _B8_OS_SYSCALL_CLOCK_SETTIME(void){
  _b8OsSetError(-EPERM);
  _b8OsSwitchBackToUsr();
}

typedef void  (*B8_OS_SYSCALL_FUNC)(void);
static  const B8_OS_SYSCALL_FUNC  _b8OsSysCallTbl[ B8_OS_SYSCALL_MAX ] = {
  _B8_OS_SYSCALL_NULL,
  _B8_OS_SYSCALL_GET_BRIDGE,
  _B8_OS_SYSCALL_SCHED_YIELD,
  _B8_OS_SYSCALL_SCHED_SLEEP,
  _B8_OS_SYSCALL_THREAD_CREATE,
  _B8_OS_SYSCALL_EXIT,
  _B8_OS_SYSCALL_SET_ERRNO,
  _B8_OS_SYSCALL_GET_ERRNO,
  _B8_OS_SYSCALL_SEM_INIT,
  _B8_OS_SYSCALL_SEM_POST,
  _B8_OS_SYSCALL_SEM_WAIT,
  _B8_OS_SYSCALL_SEM_GETVALUE,
  _B8_OS_SYSCALL_CLOCK_GETRES,
  _B8_OS_SYSCALL_CLOCK_GETTIME,
  _B8_OS_SYSCALL_CLOCK_SETTIME,
};

// Called only from bootloader.s / __svc_dispatch:
void  _b8OsSvcDispatch(void){
  _b8OsGiveBridgeToUsrByPid( _CurrentPid );
  const b8OsSysCallNum syscall = b8OsSysCallArgs[ 0 ];
  if( syscall >= B8_OS_SYSCALL_MAX ){
    _b8OsSetError(-EINVAL);
    _b8OsSwitchBackToUsr();
    // It won't get here
  }
  _b8OsSysCallTbl[ syscall ]();
  _b8OsSwitchBackToUsr();
  // It won't get here
}

static  int   _b8OsIrqDispatch(int irq, void* arg){
  _IrqDispatched = irq;
  (void)arg;
  ReqSchedule rs;
  ReqScheduleClear( &rs );
  if( irq == _IrqTimer ){
    rs.req |= REQ_SCHEDULE_REGULAR;
  } else {
    rs.req |= REQ_SCHEDULE_AWAKE_THREAD_WAITING_FOR_IRQ;
  }

  _b8OsProcessScheduler( &rs );
  _b8OsSwitchBackToUsr();
  // It won't get here
  return B8_OS_OK;
}

static  b8OsPid _b8OsPickupThreadYield( void ){
  Node* it;
  for(
    it =  ListBegin( _WaitingTasksList ) ;
    it != ListEnd( _WaitingTasksList )   ;
    it =  it->_next
  ){
    Tcb* tcb = _b8OsGetTcb( it->pid );
    KPANIC(tcb,"not found");
    if( tcb->waiting_for != TWF_YIELD ) continue;
    return it->pid;
  }
  return  B8_OS_INVALID_PID;
}

static  b8OsPid _b8OsPickupThreadWaitingForTimer( void ){
  Node* it;
  for(
    it =  ListBegin( _WaitingTasksList ) ;
    it != ListEnd( _WaitingTasksList )   ;
    it =  it->_next
  ){
    Tcb* tcb = _b8OsGetTcb( it->pid );
    KPANIC(tcb,"not found");
    if( tcb->waiting_for != TWF_TIMER ) continue;
    if( _AccumelatedTime < tcb->wake_up_time ) continue;
    return it->pid;
  }
  return  B8_OS_INVALID_PID;
}

static  b8OsPid _b8OsPickupThreadWaitingForSemaphore( b8OsSid sid ){
  Node* it;
  for(
    it =  ListBegin( _WaitingTasksList ) ;
    it != ListEnd( _WaitingTasksList )   ;
    it =  it->_next
  ){
    Tcb* tcb = _b8OsGetTcb( it->pid );
    KPANIC(tcb,"not found");
    if( tcb->waiting_for != TWF_SEMAPHORE ) continue;
    if( tcb->sid_wait != sid )  continue;
    tcb->sid_wait = B8_OS_INVALID_SID;
    return it->pid;
  }
  return  B8_OS_INVALID_PID;
}

static  b8OsPid _b8OsPickupThreadWaitingForSemaphoreTimeOut( void ){
  Node* it;
  for(
    it =  ListBegin( _WaitingTasksList ) ;
    it != ListEnd( _WaitingTasksList )   ;
    it =  it->_next
  ){
    Tcb* tcb = _b8OsGetTcb( it->pid );
    KPANIC(tcb,"not found");
    if( tcb->waiting_for != TWF_SEMAPHORE ) continue;
    if( _UnixEpochTimeMicroseconds + _AccumelatedTime < tcb->wake_up_time )  continue;
    return it->pid;
  }
  return  B8_OS_INVALID_PID;
}

static  b8OsPid _b8OsPickThreadWaitingForIrq(void){
  Node* it;
  for(
    it =  ListBegin( _WaitingTasksList ) ;
    it != ListEnd( _WaitingTasksList )   ;
    it =  it->_next
  ){
    Tcb* tcb = _b8OsGetTcb( it->pid );
    KPANIC(tcb,"not found");
    if( tcb->waiting_for != TWF_IRQ ) continue;
    if(tcb->irq != _IrqDispatched)    continue;
    return it->pid;
  }
  return  B8_OS_INVALID_PID;
}

static  Tcb*  _b8OsWaitCurrentPid( TcbWaitingFor waiting_for ){
  ListErase( _ReadyTasksList , _CurrentPid );
  ListPushBack( _WaitingTasksList , _CurrentPid );
  Tcb* tcb = _b8OsGetTcb( _CurrentPid );
  KPANIC( tcb , "not found current tcb" );
  tcb->waiting_for = waiting_for;
  return tcb;
}

static  void  _b8OsSwitchPidAndBackToUsr( b8OsPid pid_pickup ){
  KPANIC( pid_pickup != B8_OS_INVALID_PID , "no tcb" );
  _CurrentPid = pid_pickup;
  Tcb* tcb_cur = _b8OsGetTcb( _CurrentPid );
  KPANIC(tcb_cur,"invalid _CurrentPid" );
  if( tcb_cur->status == TS_NOT_YET_INIT ){
    TcbInit( tcb_cur );
  }
  Tcb2UsrContext( tcb_cur );
  _b8OsSwitchBackToUsr();
  // It won't get here
}

static  void  _b8OsAwakePid( b8OsPid pid ){
  Tcb* tcb_wakeup = _b8OsGetTcb( pid );
  KPANIC( tcb_wakeup , "invalid tcb_wakeup" );
  tcb_wakeup->waiting_for = TWF_NOTHING;

  ListErase( _WaitingTasksList , pid );
  ListErase( _ReadyTasksList   , pid );

  ListPushBack( _ReadyTasksList , pid );

  _b8OsSwitchPidAndBackToUsr( pid );
  // It won't get here.
}

static  void  _b8OsProcessScheduler(ReqSchedule* rs){
  if( rs->req == REQ_SCHEDULE_NONE ) return;

  u32 cyccnt;
  _Config.ArchDriverGetCycleAndClear( &cyccnt );
  _CycCnt += (u64)cyccnt;
  const b8OsUsec  dt = ((u64)cyccnt * _UsPerCpuCycleFixed8 )>>8;
  _AccumelatedTime += dt;

  Tcb* tcb_cur = _b8OsGetCurrentTcb();
  KPANIC(tcb_cur ,"invalid tcb_cur" );
  if(tcb_cur->status == TS_READY ){
    UsrContext2Tcb( tcb_cur );
  }

  // Check if there is any pid waiting for irq.
  if( rs->req & REQ_SCHEDULE_AWAKE_THREAD_WAITING_FOR_IRQ){
    b8OsPid pid_pickup = _b8OsPickThreadWaitingForIrq();
    if( pid_pickup != B8_OS_INVALID_PID ){
      _b8OsAwakePid( pid_pickup );
      // It won't get here
    }
  }

  if( rs->req & REQ_SCHEDULE_REGULAR ){
    // Check if there is any pid waiting for semaphore.
    {
      b8OsPid pid_pickup = _b8OsPickupThreadWaitingForSemaphoreTimeOut();
      if( pid_pickup != B8_OS_INVALID_PID ){
        Tcb* tcb = _b8OsGetTcb( pid_pickup );
        KPANIC( tcb , "invalid tcb" );

        Semaphore* sem = _b8OsGetSemaphore( tcb->sid_wait );
        sem->semcount++;
        tcb->sid_wait = B8_OS_INVALID_SID;

        _b8OsSetErrorInBridge( -ETIMEDOUT , pid_pickup );
        _b8OsGiveBridgeToUsrByPid( pid_pickup );

        _b8OsAwakePid( pid_pickup );
        // It won't get here
      }
    }

    // Check if there is any pid waiting for timer.
    {
      b8OsPid pid_pickup = _b8OsPickupThreadWaitingForTimer();
      if( pid_pickup != B8_OS_INVALID_PID ){
        _b8OsAwakePid( pid_pickup );
        // It won't get here
      }
    }
  }

  if( rs->req & REQ_SCHEDULE_SEMAPHORE_WAIT  ){
    _b8OsWaitCurrentPid( TWF_SEMAPHORE );

  // yield
  } else if( rs->req & REQ_SCHEDULE_YIELD ){
    _b8OsWaitCurrentPid(
      _b8OsGetTcb( _CurrentPid )->irq == B8_OS_NOT_USING_IRQ ?
      TWF_YIELD : TWF_IRQ
    );

  } else if( rs->req & REQ_SCHEDULE_YIELD_TIME ){
    Tcb* tcb_yield = _b8OsWaitCurrentPid( TWF_TIMER );
    tcb_yield->wake_up_time = _AccumelatedTime + rs->sleep_time;

  // wake up a task that is waiting for semaphore.
  } else if( rs->req & REQ_SCHEDULE_AWAKE_THREAD_WAITING_FOR_SEMAPHORE ){
    _b8OsAwakePid( _b8OsPickupThreadWaitingForSemaphore( rs->sid ) );
    // It won't get here

  // exit
  } else if( rs->req & REQ_SCHEDULE_EXIT_THREAD ){
    ListErase( _ReadyTasksList , rs->pid );
    ListErase( _WaitingTasksList , rs->pid );
    ListPushBack( _ZombieTasksList , rs->pid );
  }

  // wake up TWF_YIELD
  if( ListSize( _WaitingTasksList ) > 0 ){
    b8OsPid pid = _b8OsPickupThreadYield();
    if( pid != B8_OS_INVALID_PID ){
      _b8OsAwakePid( pid );
      // It won't get here.
    }
  }

  static int _debug_sel = 0;
  if( ListSize( _ReadyTasksList ) > 1 ){
    const u32 sel = 1 + (_debug_sel % (ListSize( _ReadyTasksList )-1) );
    Node* node_pick = ListAt( _ReadyTasksList , sel );
    if( node_pick ){
      _debug_sel++;
      _b8OsSwitchPidAndBackToUsr( node_pick->pid );
      // It won't get here
    }
  }

  // pick up idle thread
  Node* node_pick = ListAt( _ReadyTasksList , 0 );
  KPANIC( node_pick , "not found idle pid" );
  KPANIC( node_pick->pid == _IdlePid , "invalid idle pid" );
  _b8OsSwitchPidAndBackToUsr( node_pick->pid );
  // It won't get here
}

b8OsBridgeUsr2Svc* b8OsGetBridge(void){
  KPANIC( _b8OsGetCPSRMode() == USR_MODE , "it must be usr mode" );
  return  b8OsSysCall(B8_OS_SYSCALL_GET_BRIDGE,0,0,0,0,0,0);
}

static  int _b8OsIrqAttach(int irq,b8IrqHandler isr,void* arg){
  if( irq >= B8_IRQ_NUM_OF_INTERRUPTS ){
    return  _b8OsSetError( -EINVAL );
  }

  if(isr == NULL ){
    isr = _b8OsIrqUnexpectedIsr;
    arg = NULL;
  }

  if( _IrqInfo[ irq ].handler != _b8OsIrqUnexpectedIsr ){
    return  _b8OsSetError( -EINVAL );
  }

  _IrqInfo[ irq ].handler = isr;
  _IrqInfo[ irq ].arg     = arg;

  return B8_OS_OK;
}

#define B8_MIF_ADDR             (0xffffd000)
#define B8_MIF_DATAABORT_ADDR   _B8_REG(B8_MIF_ADDR + 0x00)

void  b8OsIrqDispatchEntry(void){
  const u32 irq = B8_PIC_IAR;
  B8_SYS_ASSERT( irq != 0 , "irq == 0" );
  if( irq == B8_IRQ_DABT ){
    b8SysPuts( "DATA ABORT" );
    b8SysPutCR();

    b8SysPuts( "MIF_STATUS=" );
    b8SysPutHex( B8_MIF_DATAABORT_ADDR );
    b8SysPutCR();

    b8SysHalt();
    return;

  } else if( irq == B8_IRQ_UNDF ){
    b8SysPuts( "UNDEFINED INSTRUCTION" );
    b8SysPutCR();
    b8SysHalt();
    return;

  } else if( irq >= B8_IRQ_NUM_OF_INTERRUPTS ){
    b8SysPuts( "invalid irq=0x" );
    b8SysPutHex( irq );
    b8SysPutCR();
    b8SysHalt();
    return;
  }
  b8IrqHandler isr = _IrqInfo[ irq ].handler;
  isr( irq , _IrqInfo[ irq ].arg );
}

int  b8OsIsRunning(void){
  return  _IsRunning;
}