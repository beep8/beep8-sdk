#include <beep8.h>
#include <sys/errno.h>

int b8TmrSetup( u32 tmr_ch , u32 cycval ){
  if( tmr_ch >= B8_TMR_NUM ){
    return  set_errno( EINVAL );
  }

  int ret = b8SysSetupIrqWait( B8_IRQ_TMR0 + tmr_ch );
  if( ret < 0 ) return ret;

  B8_TMR_MODE( tmr_ch )   = B8_TMR_MODE_PERIODIC;
  B8_TMR_CNT( tmr_ch )    = 0;
  B8_TMR_PER( tmr_ch )    = cycval;;
  B8_TMR_CTRL( tmr_ch )   = B8_ENABLE;

  return 0;
}

int b8TmrWait( u32 tmr_ch  ){
  if( tmr_ch >= B8_TMR_NUM ){
    return  set_errno( EINVAL );
  }

  int ret = b8SysIrqWait( B8_IRQ_TMR0 + tmr_ch );
  if( ret < 0 ) return ret;
  return 0;
}
