// HTTP(S) GET file driver. See httpdrv.h for the usage contract.
//
// Wire protocol over SCI channel 2 (must match js.b8/vhttp.js):
//   ROM -> host : URL bytes, then 0x0a ('\n')
//   host -> ROM : response body bytes, then 0xff (EOF)
#include <stdio.h>
#include <b8/assert.h>
#include <b8/syscall.h>   // usleep
#include <crt/crt.h>
#include <beep8.h>        // B8_FIFO_SCI_* (register.h)
#include <sublibc.h>
#include <string>

using namespace std;

#define SCI_CH_HTTP   (2)          // must match idx_vhttp in js.b8/beep8.js
#define HTTP_REQ_EOL  (0x0a)
#define HTTP_RES_EOF  (0xff)
#define SIG_DRV       (0x48545450) // 'HTTP'
#define POLL_USEC     (1000)       // yield granularity while waiting for the host

struct DriverWork {
  u32   _sig  = SIG_DRV;
  bool  _lock = false;             // one request in flight at a time (serial)
};

struct FileWork {
  string  _req;                    // URL accumulated via write()
  bool    _sent = false;
  bool    _eof  = false;
};

static  int     http_open( File* filep ){
  DriverWork* dw = (DriverWork*)filep->d_priv;
  if( dw->_sig != SIG_DRV ) return -1;
  if( dw->_lock ) return -2;
  dw->_lock = true;
  filep->f_priv = new FileWork;
  return 0;
}

static  ssize_t http_write( File* filep, const char* buffer, size_t buflen ){
  FileWork* pw = (FileWork*)filep->f_priv;
  pw->_req.append( buffer, buflen );
  return buflen;
}

static  void    http_send( FileWork* pw ){
  for( char c : pw->_req ) B8_FIFO_SCI_TX( SCI_CH_HTTP ) = (u8)c;
  B8_FIFO_SCI_TX( SCI_CH_HTTP ) = HTTP_REQ_EOL;
  pw->_sent = true;
}

static  ssize_t http_read( File* filep, char* buffer, size_t buflen ){
  FileWork* pw = (FileWork*)filep->f_priv;
  if( !pw->_sent ) http_send( pw );   // lazy: the URL is sent on the first read
  if( pw->_eof ) return 0;

  size_t got = 0;
  while( got < buflen ){
    while( got < buflen && B8_FIFO_SCI_RX_LEN( SCI_CH_HTTP ) > 0 ){
      u8 b = B8_FIFO_SCI_RX( SCI_CH_HTTP );
      if( b == HTTP_RES_EOF ){ pw->_eof = true; return (ssize_t)got; }
      buffer[ got++ ] = (char)b;
    }
    if( got > 0 ) break;              // return what we have; stdio calls us again
    usleep( POLL_USEC );              // yield: let the frame complete + host fetch
  }
  return (ssize_t)got;
}

static  int     http_close( File* filep ){
  DriverWork* dw = (DriverWork*)filep->d_priv;
  FileWork*   pw = (FileWork*)filep->f_priv;

  // Drain to EOF so a partially-read response doesn't poison the next request.
  if( pw->_sent && !pw->_eof ){
    for( int guard = 0 ; guard < 2000 && !pw->_eof ; ++guard ){
      while( B8_FIFO_SCI_RX_LEN( SCI_CH_HTTP ) > 0 ){
        if( B8_FIFO_SCI_RX( SCI_CH_HTTP ) == HTTP_RES_EOF ){ pw->_eof = true; break; }
      }
      if( !pw->_eof ) usleep( POLL_USEC );
    }
  }

  delete pw;
  dw->_lock = false;
  return 0;
}

// Streaming device: pretend seek succeeds so stdio's r+ read/write transition
// (which calls lseek) doesn't log EBADF. The offset is ignored.
static  off_t   http_seek( File* filep, int ptr, int dir ){
  (void)filep; (void)ptr; (void)dir;
  return 0;
}

static const file_operations http_fops =
{
  http_open,  /* open  */
  http_close, /* close */
  http_read,  /* read  */
  http_write, /* write */
  http_seek,  /* seek  */
  NULL        /* ioctl */
};

namespace http {
  void  Reset(){
    static bool _is_reset = false;
    if( false == _is_reset ){
      char name[32];
      memsetz( name, sizeof(name) );
      snprintf( name, sizeof(name), "/http/con0" );
      fs_register_driver(
        name,
        &http_fops,
        0666,
        new DriverWork
      );
      _is_reset = true;
    }
  }
}
