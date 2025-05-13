#include <stdio.h>
#include <b8/assert.h>
#include <crt/crt.h>
#include <stdlib.h>
#include <sublibc.h>
#include <string>
#include <trace.h>
#include <fcast.h>

using namespace std;

#define SCI_CH_CLIP (13)
#define SIG_DRV     (0xfacecade)

struct DriverWork {
  u32   _sig = SIG_DRV;
  bool  _lock = false;
};

struct FileWork {
  string _str_buff;
};

static  int     clipboard_open( File* filep){
  DriverWork* dw = (DriverWork*)filep->d_priv;
  if( dw->_sig != SIG_DRV ) return -1;
  if( dw->_lock ) return -2;

  filep->f_priv = new FileWork;
  return 0;
}

static  void    clipboard_flush( string& buff_ ){
  for( const auto& it : buff_ ){
    B8_FIFO_SCI_TX( SCI_CH_CLIP ) = (u32)it;
  }
  buff_.clear();
}

static  int     clipboard_close( File* filep){
  DriverWork* dw = (DriverWork*)filep->d_priv;
  if( dw->_sig != SIG_DRV ) return -1;

  FileWork* pw = (FileWork*)filep->f_priv;
  clipboard_flush( pw->_str_buff );
  B8_FIFO_SCI_TX( SCI_CH_CLIP ) = EOF;
  dw->_lock = false;

  delete pw;
  return 0;
}

static  ssize_t clipboard_write(File* filep,const char *buffer, size_t buflen) {
  DriverWork* dw = (DriverWork*)filep->d_priv;
  if( dw->_sig != SIG_DRV ) return -1;

  FileWork* pw = (FileWork*)filep->f_priv;
  for( size_t nn=0 ; nn<buflen ; ++nn ){
    pw->_str_buff.push_back( buffer[nn] );
    if( pw->_str_buff.size() > 0x100 ){
      clipboard_flush( pw->_str_buff );
    }
  }
  return buflen;
}

static const file_operations clipboard_fops =
{
  clipboard_open, /* open  */
  clipboard_close,/* close */
  NULL,           /* read  */
  clipboard_write,/* write */
  NULL,           /* seek  */
  NULL            /* ioctl */
};

namespace clipboard {
  void  Reset(){
    static bool _is_reset = false;
    if( false == _is_reset ){
      char name[32];
      memsetz(name,sizeof(name));
      snprintf(name, sizeof(name), "/clipboard/con0");
      fs_register_driver(
        name,
        &clipboard_fops,
        0666,
        new DriverWork
      );
      _is_reset = true;
    }
  }
}