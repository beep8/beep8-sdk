// Minimal one-way download sink over SCI. See download.h.
//
// Modeled on clipboard.cpp, but pushes each byte straight to the SCI TX FIFO
// (no internal buffering) and emits no in-band terminator: the host framing is
// a leading "<len>\n" header written by the caller, so the payload is fully
// binary-safe. Flow control (staying under the 8 KB FIFO) is the caller's job.
#include <stdio.h>
#include <b8/assert.h>
#include <crt/crt.h>
#include <stdlib.h>
#include <sublibc.h>

#define SCI_CH_DL (12)          // free SCI channel (0/1/2 = vterm/savedata/http, 13 = clip)
#define SIG_DRV   (0xda7a1000)

struct DriverWork {
  u32   _sig = SIG_DRV;
};

static  int     download_open( File* filep ){
  DriverWork* dw = (DriverWork*)filep->d_priv;
  if( dw->_sig != SIG_DRV ) return -1;
  return 0;
}

static  int     download_close( File* filep ){
  DriverWork* dw = (DriverWork*)filep->d_priv;
  if( dw->_sig != SIG_DRV ) return -1;
  return 0;
}

static  ssize_t download_write( File* filep, const char* buffer, size_t buflen ){
  DriverWork* dw = (DriverWork*)filep->d_priv;
  if( dw->_sig != SIG_DRV ) return -1;
  for( size_t nn = 0 ; nn < buflen ; ++nn ){
    B8_FIFO_SCI_TX( SCI_CH_DL ) = (u32)(u8)buffer[nn];
  }
  return buflen;
}

static const file_operations download_fops =
{
  download_open,  /* open  */
  download_close, /* close */
  NULL,           /* read  */
  download_write, /* write */
  NULL,           /* seek  */
  NULL            /* ioctl */
};

namespace download {
  void  Reset(){
    static bool _is_reset = false;
    if( false == _is_reset ){
      char name[32];
      memsetz( name, sizeof(name) );
      snprintf( name, sizeof(name), "/download/con0" );
      fs_register_driver(
        name,
        &download_fops,
        0666,
        new DriverWork
      );
      _is_reset = true;
    }
  }
}
