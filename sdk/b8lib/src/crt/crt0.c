#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <b8/register.h>
#include <b8/type.h>
#include <b8/sys.h>
#include <b8/misc.h>
#include <b8/irq.h>
#include <b8/pic.h>
#include <b8/os.h>
#include <b8/assert.h>
#include <b8/dwt.h>
#include <b8/semaphore.h>
#include <b8/ppu.h>
#include <b8/hif.h>
#include <b8/pthread.h>
#include <crt/crt.h>
#include <sys/time.h>

union	fc32 {
  u32 	aU32;
  u32* 	pU32;
};

extern  int set_errno(int errcode);
extern  int get_errno(void);
static sem_t    sem_heap;

static  void  fs_register_driver_init(void);
static  FsDriver* fs_get_driver( int fd );
void  _crt0_puts( const char* str );
static  void  _crt0_br(void);
static  int   _stdout_register(void);
static  int   _stdin_register(void);
static  int   _stderr_register(void);

#define BEEP8_BOOT_LOADER_VERSION (0x20211219)

extern  int main(int argc_, char** argv_);

#define MEM_IS_4BYTE_ALIGNED(p)   (0 == ((u32)(p) & 0x03))

#define MEMCLR_4TIMES( dst, len, size )                 \
    for ( ; len >= (size*4); len -= (size*4) ){         \
        *dst++ = 0; *dst++ = 0; *dst++ = 0; *dst++ = 0; \
    }

#define MEMCLR_1TIMES( dst, len, size )   \
    for ( ; len >= size; len -= size ) {  \
        *dst++ = 0;                       \
    }

#define MEMCLR_BYTE( dst, len ) {                \
    char* __dst_c = (char*)dst;                  \
    MEMCLR_4TIMES( __dst_c, len, sizeof(char) )  \
    MEMCLR_1TIMES( __dst_c, len, sizeof(char) )  \
}

#define MEMCLR_LONG( dst, len ) {                \
    long* __dst_l = (long*)dst;                  \
    MEMCLR_4TIMES( __dst_l, len, sizeof(long) )  \
    MEMCLR_1TIMES( __dst_l, len, sizeof(long) )  \
    MEMCLR_BYTE  ( __dst_l, len )                \
}

#define MEMCLR( dst, len ) {                     \
    size_t __tmp = len;                          \
    if (!MEM_IS_4BYTE_ALIGNED(dst)) {            \
        MEMCLR_BYTE( dst, __tmp );               \
    } else {                                     \
        MEMCLR_LONG( dst, __tmp );               \
    }                                            \
}

#define USR_MODE  (16)
#define FIQ_MODE  (17)
#define IRQ_MODE  (18)
#define SVC_MODE  (19)
#define MON_MODE  (20)
#define ABT_MODE  (23)
#define UND_MODE  (27)
#define SYS_MODE  (31)

void  _crt0_puts( const char* str ){
  const char* pp = str;
  while( *pp ){
    B8_FIFO_SCI_TX(0) = (u32)*pp;
    ++pp;
  }
}

static  void  _crt0_br(void){
  _crt0_puts( "\n" );
}

void  _crt0_puthex( u32 data ){
  for( s16 sft=28 ; sft>=0 ; sft-=4 ){
    B8_FIFO_SCI_TX(0) = "0123456789abcdef"[ (data >> sft) & 0xf ];
  }
}

void  _crt0_puthex_u8( u8 data ){
  for( s16 sft=4 ; sft>=0 ; sft-=4 ){
    B8_FIFO_SCI_TX(0) = "0123456789abcdef"[ (data >> sft) & 0xf ];
  }
}

void _dump_cpsr(uint32_t cpsr) {
  _crt0_puts( "CPSR:" );

  char buff[16];
  int index = 0;
  buff[index++] = (cpsr & (1 << 31)) ? 'N' : 'n';  // Negative Flag (N)
  buff[index++] = (cpsr & (1 << 30)) ? 'Z' : 'z';  // Zero Flag (Z)
  buff[index++] = (cpsr & (1 << 29)) ? 'C' : 'c';  // Carry Flag (C)
  buff[index++] = (cpsr & (1 << 28)) ? 'V' : 'v';  // Overflow Flag (V)
  buff[index++] = (cpsr & (1 << 27)) ? 'Q' : 'q';  // Saturation Flag (Q)
  buff[index++] = (cpsr & (1 << 24)) ? 'J' : 'j';  // Jazelle state (J)
  buff[index++] = (cpsr & (1 << 7))  ? 'I' : 'i';  // IRQ Mask Flag (I)
  buff[index++] = (cpsr & (1 << 6))  ? 'F' : 'f';  // FIQ Mask Flag (F)
  buff[index++] = (cpsr & (1 << 5))  ? 'T' : 't';  // Thumb State Flag (T)
  buff[index++] = (cpsr & (1 << 4))  ? 'E' : 'e';  // Endianness Flag (E)
  buff[index] = '\0';
  _crt0_puts(buff);

  _crt0_puts( " Mode=0x" );
  const uint8_t mode = cpsr & 0x1f;
  _crt0_puthex_u8( mode );
  _crt0_puts( " " );
  char* strmode = "";
  switch( mode ){
    case  USR_MODE: strmode = "USR"; break;
    case  FIQ_MODE: strmode = "FIQ"; break;
    case  IRQ_MODE: strmode = "IRQ"; break;
    case  SVC_MODE: strmode = "SVC"; break;
    case  MON_MODE: strmode = "MON"; break;
    case  ABT_MODE: strmode = "ABT"; break;
    case  UND_MODE: strmode = "UND"; break;
    case  SYS_MODE: strmode = "SYS"; break;
  }
  _crt0_puts( strmode );
  _crt0_puts( "\n" );
}

extern int _secend_C[];
extern int _sectop_CR[];
extern int _secend_CR[];
extern int _secend_B[];
extern int _sectop_R[];
extern int _secend_R[];
extern int _sectop_CINIT[];
extern int _secend_CINIT[];
extern int _sectop_CINITR[];
extern int _secend_CINITR[];

extern int _sec_text_rom_s[];
extern int _sec_text_rom_e[];

extern int _sec_rodata_rom_s[];
extern int _sec_rodata_rom_e[];

extern int _sec_data_rom_s[];
extern int _sec_data_ram_s[];
extern int _sec_data_ram_e[];

extern int _sec_bss_ram_s[];
extern int _sec_bss_ram_e[];

#define ADDR( x ) ((uint32_t)(x))

#define END_P           ((uint32_t)_secend_P)
#define TOP_PR          ((uint32_t)_sectop_PR)
#define END_PR          ((uint32_t)_secend_PR)
#define END_C           ((uint32_t)_secend_C)
#define TOP_CR          ((uint32_t)_sectop_CR)
#define END_CR          ((uint32_t)_secend_CR)

#define TOP_CINIT       ((uint32_t)_sectop_CINIT)
#define END_CINIT       ((uint32_t)_secend_CINIT)
#define TOP_CINITR      ((uint32_t)_sectop_CINITR)
#define END_CINITR      ((uint32_t)_secend_CINITR)

static  void  _crt0_memcpy( u32 src , u32 dst , u32 bytesize ){
  u32* psrc = (u32*)src;
  u32* pdst = (u32*)dst;
  for( u32 nw=0 ; nw<bytesize/sizeof(u32) ; ++nw ){
    *pdst++ = *psrc++;
  }
}

#define OS_TMR_CH  (3)
#define OS_TICK_HZ  (100)

static  int ArchDriverGetTimerIrq(u16* irq){
  *irq = B8_IRQ_TMR0 + OS_TMR_CH;
  return 0;
}

static  int ArchDriverOnStartTimer(u32 hz){
  B8_TMR_MODE( OS_TMR_CH )   = B8_TMR_MODE_PERIODIC;
  B8_TMR_CNT(  OS_TMR_CH )   = 0;
  B8_TMR_PER(  OS_TMR_CH )   = (b8SysGetCpuClock() / hz) >> 8;
  B8_TMR_CTRL( OS_TMR_CH )   = B8_ENABLE;
  return 0;
}

static  int ArchDriverOnStartCycleCnt(void){
  B8_DWT_CYCCNT = 0;
  B8_DWT_CTRL = 1;
  return 0;
}

static  void  ArchDriverGetCycle(u32* cyccnt ){
  *cyccnt = B8_DWT_CYCCNT;
}

static  void  ArchDriverGetCycleAndClear(u32* cyccnt ){
  *cyccnt = B8_DWT_CYCCNT;
  B8_DWT_CYCCNT = 0;
}

static  void  ArchDriverPicDistributor (u8 enable ){
  B8_PIC_DCR = enable;
}

static  void  ArchDriverGetClockTime( u64* clock ){
  *clock = (((u64)B8_INF_CAL_H)<<32)| ((u64)B8_INF_CAL_L);
}

void  _crt0_puthex_str( uint32_t x_ , const char* sz_ ){
  _crt0_puts("0x");
  _crt0_puthex( x_ );
  _crt0_puts(" // " );
  _crt0_puts( sz_ );
  _crt0_puts( "\n" );
}

void _crt0_hr(void){
  for( uint32_t ii=0 ; ii<32 ; ++ ii ) B8_FIFO_SCI_TX(0) = '=';
  B8_FIFO_SCI_TX(0) = '\n';
}

void _crt0_stop(void){
  _crt0_puts("\nYour BEEP-8 application will be stopped.\n");
}

void crt0_data_abort(uint32_t fault_instruction, uint32_t fault_address, uint32_t fault_status , uint32_t sp_usr, uint32_t cpsr ){
  _crt0_hr();
  if (fault_address == 0x00000000) {
    _crt0_puts("NULL pointer access detected. " );
  } else {
    _crt0_puts("Illegal memory access detected. " );
  }
  _crt0_stop();

  _crt0_puts(
    fault_status & (1 << 3) ?
      "WRITE" :
      "READ"
  );
  _crt0_puts(" operation caused a fault.\n" );

  _crt0_puthex_str( fault_instruction , "Faulting instruction address" );
  _crt0_puthex_str( fault_address     , "Faulting data address" );
  _crt0_puthex_str( fault_status      , "Fault status" );
  _crt0_puthex_str( sp_usr            , "User mode sp");
  _dump_cpsr( cpsr );

  _crt0_puts("You can refer to the linker-generated .map file\nto locate the symbol associated with this address.\n");
}

void crt0_undef(uint32_t pc, uint32_t cpsr) {
  _crt0_hr();
  _crt0_puts("Undefined Instruction Exception. ");
  _crt0_stop();

  union fc32 fc;
  fc.aU32 = pc;
  _crt0_puthex_str( *fc.pU32 ,  "unknown instruction" );
  _crt0_puthex_str( pc,         "PC (Faulting instruction address) Fault status" );
  _crt0_puthex_str( cpsr,       "CPSR" );
  _dump_cpsr( cpsr );
}

void  crt0_test(void){
  _crt0_puts( "crt0_test\n" );
}

void section_dump( const char* name_ , uint32_t start_ , uint32_t end_ ) {
  _crt0_puts( name_ );
  _crt0_puts( "  [0x" );
  _crt0_puthex( start_ );
  _crt0_puts( " - 0x" );
  _crt0_puthex( end_ );
  _crt0_puts( "] size:0x" );
  _crt0_puthex( end_ - start_ );
  _crt0_br();
}

unsigned int crt0_entry(void){
  _crt0_memcpy(
    ADDR(_sec_data_rom_s),
    ADDR(_sec_data_ram_s),
    ADDR(_sec_data_ram_e) - ADDR(_sec_data_ram_s)
  );
  MEMCLR(
    ADDR( _sec_bss_ram_s ),
    ADDR( _sec_bss_ram_e ) -
    ADDR( _sec_bss_ram_s )
  );

  _crt0_puts( "BEEP-8 bootloader ver " );
  _crt0_puthex( BEEP8_BOOT_LOADER_VERSION );
  _crt0_puts( ". Copyright (C) 2021 BEEP-8" );
  _crt0_br();

  section_dump( ".text  " , ADDR( _sec_text_rom_s ), ADDR( _sec_text_rom_e ) );
  section_dump( ".cinit " , ADDR( TOP_CINIT  ), ADDR( END_CINIT ) );
  section_dump( ".rodata" , ADDR( _sec_rodata_rom_s ), ADDR( _sec_rodata_rom_e ) );
  section_dump( ".data  " , ADDR( _sec_data_ram_s ), ADDR( _sec_data_ram_e ) );
  section_dump( ".bss   " , ADDR( _sec_bss_ram_s ), ADDR( _sec_bss_ram_e ) );

  fs_register_driver_init();
  _stdout_register();
  _stdin_register();
  _stderr_register();

  static u8 OsStack[64*1024];
  b8OsConfig cfg;
  memset( &cfg , 0 , sizeof(cfg) );
  cfg.StackTop = OsStack;
  cfg.StackSize = sizeof(OsStack);
  cfg.CpuCyclesPerSec = b8SysGetCpuClock();
  cfg.UsecPerTick  = 1000;
  cfg.sem_heap = &sem_heap;
  cfg.ArchDriverGetTimerIrq       = ArchDriverGetTimerIrq;
  cfg.ArchDriverOnStartTimer      = ArchDriverOnStartTimer;
  cfg.ArchDriverOnStartCycleCnt   = ArchDriverOnStartCycleCnt;
  cfg.ArchDriverGetCycle          = ArchDriverGetCycle;
  cfg.ArchDriverGetCycleAndClear  = ArchDriverGetCycleAndClear;
  cfg.ArchDriverPicDistributor    = ArchDriverPicDistributor;
  cfg.ArchDriverGetClockTime      = ArchDriverGetClockTime;
  int ret = b8OsReset( &cfg );
  trace(ret);
  B8_SYS_ASSERT( ret >= 0 , "failed b8OsReset();" );
  b8SysHalt();
  // It wont't get here
  return 0;
}

static  size_t  _strlen( const char* sz_ ){
  if( NULL == sz_ )  return 0;

  size_t nn = 0;
  while( *sz_++ ) ++nn;
  return nn;
}

size_t strlen(const char *s){
  return  _strlen( s );
}

/*
  File system driver for Beep8
*/
#define N_MAX_FS_DRIVER (32)
int _id_driver;

#define N_MAX_FILES     (N_MAX_FS_DRIVER)
File  _files[ N_MAX_FILES ];

#define	STDIN   (0)
#define	STDOUT  (1)
#define	STDERR  (2)

static  FsDriver _fs_driver[ N_MAX_FS_DRIVER ];
static  FsDriver* fs_get_driver( int fd ){
  if( fd < 0 )  return 0;
  if( fd >= N_MAX_FS_DRIVER ) return 0;
  if( 0 == strlen( _fs_driver[fd]._path ) ) return 0;
  return  &_fs_driver[ fd ];
}

static  File* file_get( int fd ){
  if( fd < 0 )  return 0;
  if( fd >= N_MAX_FILES ) return 0;
  return  &_files[ fd ];
}

static  void  fs_register_driver_init(void){
  _id_driver = STDERR+1;

  FsDriver* pfd = &_fs_driver[0];
  for( size_t nn=0 ; nn<N_MAX_FS_DRIVER ; ++nn,++pfd ){
    MEMCLR( pfd , sizeof( *pfd ) );
  }

  File* pfile = &_files[ 0 ];
  for( size_t nn=0 ; nn<N_MAX_FILES ; ++nn,++pfile ){
    MEMCLR( pfile , sizeof( *pfile ) );
  }
}

static  int _fs_set_driver(
  int id_driver_,
  const char* path,
  const file_operations* fops,
  mode_t mode,
  void* priv
){
  if( id_driver_ >= N_MAX_FS_DRIVER ){
    set_errno( ENOSYS );
    return -1;
  }

  const size_t len_path = strlen( path );
  if( len_path == 0 || len_path >= N_MAX_PATH -1 ){
    set_errno( EINVAL );
    return -1;
  }

  FsDriver* fdrv = &_fs_driver[ id_driver_ ];
  strcpy( fdrv->_path , path );
  fdrv->_fops = fops;
  fdrv->_mode = mode;
  fdrv->_priv = priv;
  return 0;
}

int fs_register_driver(
  const char* path,
  const file_operations* fops,
  mode_t mode,
  void* priv
){
  int id_driver = _id_driver++;
  _fs_set_driver( id_driver, path, fops, mode, priv );
  return 0;
}

// stdout driver
static ssize_t stdout_write(File* filep,const char *buffer, size_t len) {
  (void)filep;
  const char* pp = buffer;

  for( size_t nn=0 ; nn<len ; ++nn ){
    const uint32_t code = (uint32_t) *pp++;
    B8_FIFO_SCI_TX(0) = code;
  }
  return len;
}

static const file_operations stdout_fops =
{
  NULL,         /* open  */
  NULL,         /* close */
  NULL,         /* read  */
  stdout_write, /* write */
  NULL,         /* seek  */
  NULL          /* ioctl */
};

static  int _stdout_register( void ){
  const int ret = _fs_set_driver(
    STDOUT,
    "stdout",
    &stdout_fops,
    0666,
    NULL
  );
  if( ret < 0 ) return ret;
  return 0;
}

static  int _stderr_register( void ){
  const int ret = _fs_set_driver(
    STDERR,
    "stderr",
    &stdout_fops, // same as stdout
    0666,
    NULL
  );
  if( ret < 0 ) return ret;
  return 0;
}

// stdin driver
static  ssize_t stdin_read(File* filep, char* buffer, size_t buflen){
  (void)filep;
  size_t up=0;
  size_t nn;
  for( nn=0 ; nn<buflen ; ++nn ){
    if( B8_FIFO_SCI_RX_LEN(0) > 0 ){
      const u32 kcode = B8_FIFO_SCI_RX(0);
      buffer[up++] = (char)kcode;
      if( kcode == '\n' )  return up;
    } else {
      break;
    }
  }

  if( 0 == up ){
    buffer[0] = EOF;
    return 1;
  }

  return up;
}

static const file_operations stdin_fops =
{
  NULL,           /* open  */
  NULL,           /* close */
  stdin_read,     /* read  */
  NULL,           /* write */
  NULL,           /* seek  */
  NULL            /* ioctl */
};

static  int _stdin_register( void ){
  const int ret = _fs_set_driver(
    STDIN,
    "stdin",
    &stdin_fops,
    0666,
    NULL
  );
  if( ret < 0 ) return ret;
  return 0;
}

// porting newlib
char *__env[1] = { 0 };
char **environ = __env;

extern u32 __stack_top;

static void* internal_sbrk(ptrdiff_t incr, int* err) {
  static char* heap_end = NULL;
    char* prev_heap_end;
  extern char   end_of_bss asm ("end_of_bss");  // end of .bss @beep8.ld

  if (heap_end == NULL) {
    heap_end = &end_of_bss;
  }

  prev_heap_end = heap_end;

  if (heap_end + incr >= (char*)&__stack_top) {
    *err = -ENOMEM;
    return (void*) -1;
  }

  heap_end += incr;
  return (void*) prev_heap_end;
}

caddr_t __attribute__((weak))
_sbrk(int incr) {
  int err;
  void* result = internal_sbrk(incr, &err);

  if (result == (void*) -1) {
    errno = err;
  }

  return result;
}

void* _sbrk_r(struct _reent *reent_ptr, ptrdiff_t incr) {
  int err;
  void* result = internal_sbrk(incr, &err);
  if (result == (void*) -1) {
    reent_ptr->_errno = err;
  }
  return result;
}

extern  pthread_t pthread_self(void);

pthread_t last_thread_id = 0;
int lock_count = 0;

void  __malloc_lock(struct _reent* _r){
  (void)_r;
  pthread_t current_thread_id = pthread_self();

  if (last_thread_id != current_thread_id) {
      sem_wait(&sem_heap);
      last_thread_id = current_thread_id;
  }

  lock_count++;
}

void  __malloc_unlock(struct _reent* _r) {
  (void)_r;
  if (last_thread_id == pthread_self()) {
    lock_count--;

    if (lock_count == 0) {
        last_thread_id = 0;
        sem_post(&sem_heap);
    }
  }
}

int _kill(int pid, int sig){
  (void)pid;(void)sig;
  set_errno( EINVAL );
  return -1;  // Always fails
}

int _gettimeofday(struct timeval * tv, struct timezone * tz){
  (void)tv; (void)tz;
  return -1;  // Always fails
}

void _exit(int status){
  (void)status;
  pthread_exit( NULL );
}

int _getpid(void){
  return  (int)pthread_self();
}

int _execve (char* name,char **argv,char **env) {
  (void)name; (void)argv; (void)env;
  set_errno( ENOTSUP );
  return -1; // Always fails
}

int _fork(void){
  set_errno( ENOTSUP );
  return -1;  // Always fails
}

int _open_r(struct _reent *r, const char *buf, int flags, int mode) {
  (void)r;(void)flags;(void)mode;
  for( int fd=0 ; fd < N_MAX_FS_DRIVER ; ++fd ){
    if( fd >= N_MAX_FILES ){
      set_errno( ENFILE );
      return -1;
    }

    FsDriver* driver = fs_get_driver( fd );
    if( 0 == driver ) continue;

    if( 0 != strcmp( driver->_path , buf ) ) continue;

    File* pfile = file_get( fd );
    if( !pfile ){
      set_errno( EBADF );
      return -1;
    }

    pfile->f_priv = 0;
    pfile->d_priv = driver->_priv;
    pfile->used   = 1;
    pfile->mode   = mode;

    if( driver->_fops->open ){
      int ret = (*driver->_fops->open)(pfile);
      if( ret < 0 ){
        set_errno( -ret );
        return -1;
      }
    }
    return fd;
  }
  return -1;
}

int _open(const char* buf, int flags, int mode) {
  struct _reent re;
  MEMCLR( &re, sizeof(re) ); // Since beep8 does not support recursion, 're' is zero-filled.
  return _open_r(&re, buf, flags, mode);
}

int _close( int file) {
  FsDriver* fs = fs_get_driver( file );
  if( !fs ){
    set_errno(EINVAL);
    return -1;
  }

  File* pfile = file_get( file );
  if( !pfile ){
    set_errno(EINVAL);
    return -1;
  }

  if( 0 == pfile->used ){
    set_errno(EINVAL);
    return -1;
  }

  if( fs->_fops->close ){
    int ret = (*fs->_fops->close)( pfile );
    if( ret < 0 ) return -1;
  }

  pfile->used = 0;
  pfile->f_priv = 0;
  return 0;
}

int _lseek( int file,int ptr,int dir) {
  FsDriver* fs = fs_get_driver( file );
  if( 0 == fs ){
    set_errno(EBADF);
    return -1;
  }

  if( 0 == fs->_fops->seek){
    set_errno(EBADF);
    return -1;
  }

  File* pfile =  file_get( file );
  if( 0 == pfile ){
    set_errno(EBADF);
    return -1;
  }
  return  (*fs->_fops->seek)(pfile,ptr,dir);
}

_ssize_t _write_r(struct _reent *r, int fd, const void *buf, size_t nbytes) {
  (void)r;
  FsDriver* fs = fs_get_driver( fd );
  if( 0 == fs ){
    set_errno(EBADF);
    return -1;
  }

  File* pfile = file_get( fd );
  if( 0 == pfile ){
    set_errno(EBADF);
    return -1;
  }

  if( pfile->used == 0 ){
    set_errno(EBADF);
    return -1;
  }

  if( 0 == fs->_fops->write)  return 0;

  return  (*fs->_fops->write)(pfile,buf,nbytes);
}


int _fstat(int file, struct stat* st) {
  if( (STDOUT == file) || (STDERR == file) ){
    st->st_mode = S_IFCHR;
    return  0;
  } else {
    set_errno(EBADF);
    return  -1;
  }
}

int _isatty(int file) {
  if( (file == STDOUT ) || (file == STDERR) ) {
    return  1;
  } else {
    set_errno(EBADF);
    return  -1;
  }
}

int _link(char *old,char *new_) {
  (void)old;  (void)new_;
  set_errno(ENOSYS);
  return -1;
}

_ssize_t _read_r(struct _reent *r, int fd, void *buf, size_t nbytes) {
  (void)r;
  FsDriver* fs = fs_get_driver( fd );
  if( 0 == fs ){
    set_errno(EINVAL);
    return -1;
  }

  File* pfile = file_get( fd );
  if( 0 == pfile ){
    set_errno(EBADF);
    return -1;
  }

  if( 0 == fs->_fops->read )  return 0;

  return  (*fs->_fops->read)(pfile,buf,nbytes);
}

int _read(int file,char* buf,int len) {
  struct _reent re;
  MEMCLR( &re, sizeof(re) ); // Since beep8 does not support recursion, 're' is zero-filled.
  return  _read_r(&re, file, buf, len);
}

/**
 * @brief GCC built-in memory barrier for synchronization.
 * 
 * This function is a GCC built-in that acts as a memory barrier, ensuring
 * that memory operations are completed in the correct order. It is used to
 * prevent reordering of memory accesses by the compiler or CPU in multi-core systems.
 * 
 * @note Since BEEP-8 assumes a single core, this function is implemented
 * as an empty function and does nothing.
 * 
 * @note This function is a GCC-specific extension and is not part of the
 * standard C library.
 */
void __sync_synchronize(){
  // Since BEEP-8 assumes a single core, __sync_synchronize() will be an empty function and do nothing.
}

int _ioctl(int fd, unsigned int cmd, void* arg){
  FsDriver* fs = fs_get_driver( fd );
  if( 0 == fs ){
    set_errno(EBADF);
    return -1;
  }
  File* pfile = file_get( fd );
  if( 0 == pfile ){
    set_errno(EBADF);
    return -1;
  }

  if( 0 == fs->_fops->ioctl )  return 0;

  return  (*fs->_fops->ioctl)(pfile,cmd,arg);
}

extern void (*__ctors)(void);
extern void (*__ctors_end)(void);
extern int  b8HifReset(void);

int _crt_main(void){
  ioctl(STDOUT,0);
  set_errno(0);

  b8SysSetupIrqWait( B8_IRQ_UNDF );

  void (**ppFunc)(void) = (void*) &__ctors_end;
  while ((u32)ppFunc > (u32)&__ctors){
      --ppFunc;
      (*ppFunc)();
  }

  b8HifReset();
  b8PpuReset();

  return  main(0,0);
}
