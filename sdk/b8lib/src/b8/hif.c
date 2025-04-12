#include <beep8.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define B8_HIF_SCI_CH (15)
static  sem_t             _sem_touch;
static  b8HifEvents       _touch_events;
static  b8HifMouseStatus  _mouse_status;
static  u16               _latest_identifier = 0xffff;

static  u8  _b8HifRecvSci(void){
  while( 0 == B8_FIFO_SCI_RX_LEN( B8_HIF_SCI_CH ) ) usleep(7000);
  return  (u8)B8_FIFO_SCI_RX( B8_HIF_SCI_CH );
}

static  void  _b8HifEventPushBack( b8HifEvent* ev ){
  if( sem_wait(&_sem_touch) < 0 ) return;
  if (_touch_events.num < B8_HIF_MAX_TOUCH_EVENTS) {
    _touch_events.events[_touch_events.num++] = *ev;
  }
  sem_post(&_sem_touch);
}

int b8HifGetEvents(b8HifEvents* result) {
  if (0 == result){
    set_errno( EINVAL );
    return -1;
  }
  result->num = 0;

  int ret = sem_wait(&_sem_touch);
  if (ret < 0) return ret;

  result->num = _touch_events.num;
  if( result->num > 0 ){
    memcpy(result->events, _touch_events.events, result->num * sizeof(b8HifEvent));
  }

  _touch_events.num = 0;

  ret = sem_post(&_sem_touch);
  if( ret < 0 ) return ret;

  return 0;
}

static  void* _b8HifRecvThread(void* arg ){
  (void)arg;

  sem_wait( &_sem_touch );
  memset( &_touch_events, 0x00 , sizeof( b8HifEvents ) );
  _touch_events.num = 0;
  sem_post( &_sem_touch );

  B8_HIF_TOUCH_CONNECT = B8_HIF_SCI_CH;
  B8_HIF_TOUCH_CTRL = 1;

  while(1){
    const b8HifEventType type = (b8HifEventType)_b8HifRecvSci();

    switch( type ){
      case  B8_HIF_EV_TOUCH_START:
      case  B8_HIF_EV_TOUCH_MOVE:
      case  B8_HIF_EV_TOUCH_CANCEL:
      case  B8_HIF_EV_TOUCH_END:

      case  B8_HIF_EV_MOUSE_DOWN:
      case  B8_HIF_EV_MOUSE_MOVE:
      case  B8_HIF_EV_MOUSE_UP:
      case  B8_HIF_EV_MOUSE_HOVER_MOVE:
      {
        b8HifEvent ev;
        ev.type = type;
        ev.identifier = _b8HifRecvSci();
        if( type == B8_HIF_EV_TOUCH_START ){
          _latest_identifier = ev.identifier;
        }

        const u16 xlo = _b8HifRecvSci();
        const u16 xhi = _b8HifRecvSci();
        ev.xp = (xhi<<8) | xlo;

        const u16 ylo = _b8HifRecvSci();
        const u16 yhi = _b8HifRecvSci();
        ev.yp = (yhi<<8) | ylo;

        switch( type ){
          case  B8_HIF_EV_MOUSE_DOWN:
          case  B8_HIF_EV_MOUSE_MOVE:
          case  B8_HIF_EV_MOUSE_UP:
            _mouse_status.mouse_x = ev.xp;
            _mouse_status.mouse_y = ev.yp;
            break;

          case  B8_HIF_EV_TOUCH_START:
          case  B8_HIF_EV_TOUCH_MOVE:
          case  B8_HIF_EV_TOUCH_END:{
            if( _latest_identifier == ev.identifier ){
              _mouse_status.mouse_x = ev.xp;
              _mouse_status.mouse_y = ev.yp;
              _mouse_status.is_dragging = (type == B8_HIF_EV_TOUCH_END) ? 0:1;
            }
          }break;

          case  B8_HIF_EV_MOUSE_HOVER_MOVE:
            _mouse_status.mouse_x = ev.xp;
            _mouse_status.mouse_y = ev.yp;
            break;

          default:  break;
        }

        if( type == B8_HIF_EV_MOUSE_DOWN ){
          _mouse_status.is_dragging = 1;
        } else if ( type == B8_HIF_EV_MOUSE_UP ){
          _mouse_status.is_dragging = 0;
        }

        _b8HifEventPushBack( &ev );
      }break;

      default:break;
    }
  }
  return NULL;
}

/**
 * @brief This is a system function intended to be called only from crt0.c.
 * 
 * The function b8HifReset is designed with the assumption that it will only be called from crt0.c. 
 * This function is part of the system's internal workings and not intended to be directly 
 * accessed or manipulated by the user. 
 *
 * @warning Do not call this function directly.
 * 
 * @return On success, the function returns 0. On failure, it returns a non-zero error code.
 */
int  b8HifReset(void){
  static u8 _once = 0;
  if( 0 == _once ) {
    _once = 1;
    _mouse_status.mouse_x = _mouse_status.mouse_y = 0;
    _mouse_status.is_dragging = 0;

    int ret = sem_init(&_sem_touch,0,1);
    if( ret < 0 ) return ret;

    pthread_t pid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 0x1000);
    return  pthread_create(
      &pid,
      &attr,
      _b8HifRecvThread,
      NULL
    );
  }
  return 0;
}

const b8HifMouseStatus* b8HifGetMouseStatus(void){
  return  &_mouse_status;
}
