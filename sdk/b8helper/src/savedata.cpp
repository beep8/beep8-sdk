// Persistent key-value save storage, as a file driver over SCI channel 1.
// See savedata.h for the usage contract.
//
// Wire protocol (must match js.b8/vsavedata.js):
//   ROM -> host : "set <key>=<value>\n"  or  "get <key>\n"
//   host -> ROM : for get, the value bytes; then 0xff (EOF) for both.
#include <stdio.h>
#include <b8/assert.h>
#include <b8/syscall.h>   // usleep
#include <crt/crt.h>
#include <beep8.h>        // B8_FIFO_SCI_* (register.h)
#include <sublibc.h>
#include <string>

using namespace std;

#define SCI_CH_SAVE   (1)          // must match idx_vsavedata in js.b8/beep8.js
#define SAVE_REQ_EOL  (0x0a)
#define SAVE_RES_EOF  (0xff)
#define SIG_DRV       (0x53415645) // 'SAVE'
#define POLL_USEC     (1000)

struct DriverWork {
  u32   _sig  = SIG_DRV;
  bool  _lock = false;             // one op in flight at a time (serial)
};

struct FileWork {
  string  _req;                    // command accumulated via write()
  bool    _sent = false;
  bool    _eof  = false;
};

static  int     save_open( File* filep ){
  DriverWork* dw = (DriverWork*)filep->d_priv;
  if( dw->_sig != SIG_DRV ) return -1;
  if( dw->_lock ) return -2;
  dw->_lock = true;
  filep->f_priv = new FileWork;
  return 0;
}

static  ssize_t save_write( File* filep, const char* buffer, size_t buflen ){
  FileWork* pw = (FileWork*)filep->f_priv;
  pw->_req.append( buffer, buflen );
  return buflen;
}

static  void    save_send( FileWork* pw ){
  for( char c : pw->_req ) B8_FIFO_SCI_TX( SCI_CH_SAVE ) = (u8)c;
  B8_FIFO_SCI_TX( SCI_CH_SAVE ) = SAVE_REQ_EOL;
  pw->_sent = true;
}

static  ssize_t save_read( File* filep, char* buffer, size_t buflen ){
  FileWork* pw = (FileWork*)filep->f_priv;
  if( !pw->_sent ) save_send( pw );   // lazy: send the command on the first read (get)
  if( pw->_eof ) return 0;

  size_t got = 0;
  while( got < buflen ){
    while( got < buflen && B8_FIFO_SCI_RX_LEN( SCI_CH_SAVE ) > 0 ){
      u8 b = B8_FIFO_SCI_RX( SCI_CH_SAVE );
      if( b == SAVE_RES_EOF ){ pw->_eof = true; return (ssize_t)got; }
      buffer[ got++ ] = (char)b;
    }
    if( got > 0 ) break;
    usleep( POLL_USEC );             // yield: let the frame complete + host run
  }
  return (ssize_t)got;
}

static  int     save_close( File* filep ){
  DriverWork* dw = (DriverWork*)filep->d_priv;
  FileWork*   pw = (FileWork*)filep->f_priv;

  if( !pw->_sent ) save_send( pw );  // "set" path: no read happened, send now

  // Drain to EOF so a half-read response doesn't poison the next op.
  if( !pw->_eof ){
    for( int guard = 0 ; guard < 2000 && !pw->_eof ; ++guard ){
      while( B8_FIFO_SCI_RX_LEN( SCI_CH_SAVE ) > 0 ){
        if( B8_FIFO_SCI_RX( SCI_CH_SAVE ) == SAVE_RES_EOF ){ pw->_eof = true; break; }
      }
      if( !pw->_eof ) usleep( POLL_USEC );
    }
  }

  delete pw;
  dw->_lock = false;
  return 0;
}

static const file_operations save_fops =
{
  save_open,  /* open  */
  save_close, /* close */
  save_read,  /* read  */
  save_write, /* write */
  NULL,       /* seek  */
  NULL        /* ioctl */
};

namespace savedata {
  void  Reset(){
    static bool _is_reset = false;
    if( false == _is_reset ){
      char name[32];
      memsetz( name, sizeof(name) );
      snprintf( name, sizeof(name), "/savedata/con0" );
      fs_register_driver(
        name,
        &save_fops,
        0666,
        new DriverWork
      );
      _is_reset = true;
    }
  }
}
