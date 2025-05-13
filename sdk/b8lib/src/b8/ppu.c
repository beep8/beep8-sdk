#include <beep8.h>

#define CHKOVL() _ASSERT( cmd_->sp < cmd_->tail , "ppu cmd overflow" )

union	fc32 {
  u32 	aU32;
  u32* 	pU32;
  b8PpuJmp* pJmp;
};

void  b8PpuCmdSetBuff( b8PpuCmd* cmd_ , u32* buff_ , u32 bytesize_ ){
  cmd_->sp = cmd_->buff = buff_;
  cmd_->bytesize = bytesize_;
  cmd_->tail = cmd_->sp + bytesize_ / sizeof(u32);
}
void  b8PpuCmdPush( b8PpuCmd* cmd_ , u32 word_ ){
  CHKOVL();
  *( cmd_->sp++ ) = word_;
}

b8PpuRect* b8PpuRectAlloc( b8PpuCmd* cmd_ ){
  b8PpuRect* pp = (b8PpuRect*)cmd_->sp;
  cmd_->sp = (u32*)(pp+1);
  CHKOVL();
  pp->code = B8_PPU_CMD_RECT;
  return pp;
}

b8PpuRect* b8PpuRectAllocZ( b8PpuCmd* cmd_ , u32 otz_ ){
  b8PpuRect* pp = b8PpuRectAlloc( cmd_ );
  b8PpuPushFrontOT( cmd_ , otz_ , pp );
  return pp;
}

b8PpuRect* b8PpuRectAllocZPB( b8PpuCmd* cmd_ , u32 otz_ ){
  b8PpuRect* pp = b8PpuRectAlloc( cmd_ );
  b8PpuPushBackOT( cmd_ , otz_ , pp );
  return pp;
}

b8PpuSprite* b8PpuSpriteAlloc( b8PpuCmd* cmd_ ){
  b8PpuSprite* pp = (b8PpuSprite*)cmd_->sp;
  cmd_->sp = (u32*)(pp+1);
  CHKOVL();
  pp->code = B8_PPU_CMD_SPRITE;
  pp->vfp = pp->hfp = 0;
  return pp;
}

b8PpuSprite* b8PpuSpriteAllocZ( b8PpuCmd* cmd_ , u32 otz_ ){
  b8PpuSprite* pp = b8PpuSpriteAlloc( cmd_ );
  b8PpuPushFrontOT( cmd_ , otz_ , pp );
  return pp;
}

b8PpuSprite* b8PpuSpriteAllocZPB( b8PpuCmd* cmd_ , u32 otz_ ){
  b8PpuSprite* pp = b8PpuSpriteAlloc( cmd_ );
  b8PpuPushBackOT( cmd_ , otz_ , pp );
  return pp;
}

b8PpuSetpal* b8PpuSetpalAlloc( b8PpuCmd* cmd_ ){
  b8PpuSetpal* pp = (b8PpuSetpal*)cmd_->sp;
  cmd_->sp = (u32*)(pp+1);
  CHKOVL();
  pp->code = B8_PPU_CMD_SETPAL;
  pp->palsel = 0;
  pp->wmask = 0xffff;
  pp->pidx0 = 0; pp->pidx1 = 1; pp->pidx2 = 2; pp->pidx3 = 3;
  pp->pidx4 = 4; pp->pidx5 = 5; pp->pidx6 = 6; pp->pidx7 = 7;
  pp->pidx8 = 8; pp->pidx9 = 9; pp->pidx10=10; pp->pidx11= 11;
  pp->pidx12=12; pp->pidx13=13; pp->pidx14=14; pp->pidx15= 15;
  return pp;
}

b8PpuSetpal* b8PpuSetpalAllocZ( b8PpuCmd* cmd_ , u32 otz_ , u8 flush ){
  if( flush ){
    b8PpuFlush* pp = b8PpuFlushAlloc( cmd_ );
    pp->pal = 1;
    b8PpuPushFrontOT( cmd_ , otz_ , pp );
  }

  b8PpuSetpal* pp = b8PpuSetpalAlloc( cmd_ );
  b8PpuPushFrontOT( cmd_ , otz_ , pp );
  return pp;
}

b8PpuSetpal* b8PpuSetpalAllocZPB( b8PpuCmd* cmd_ , u32 otz_ , u8 flush ){
  b8PpuSetpal* setpal = b8PpuSetpalAlloc( cmd_ );
  b8PpuPushBackOT( cmd_ , otz_ , setpal );

  if( flush ){
    b8PpuFlush* pf = b8PpuFlushAlloc( cmd_ );
    pf->pal = 1;
    b8PpuPushBackOT( cmd_ , otz_ , pf );
  }

  return setpal;
}

void  b8PpuExec( b8PpuCmd* cmd_ ){
  CHKOVL();
  __asm("nop");
  B8_PPU_EXEC = (B8_PPU_EXEC_START<<24) | (u32) cmd_->buff;
  __asm("nop");
}

void  b8PpuEnableVblankInterrupt( void ){
  B8_PPU_INTCTRL = 1;
}

b8PpuScissor* b8PpuScissorAlloc( b8PpuCmd* cmd_ ){
  b8PpuScissor* pp = (b8PpuScissor*)cmd_->sp;
  cmd_->sp = (u32*)(pp+1);
  CHKOVL();
  pp->code = B8_PPU_CMD_SCISSOR;
  return pp;
}

b8PpuScissor* b8PpuScissorAllocZ( b8PpuCmd* cmd_ , u32 otz_ ){
  b8PpuScissor* pp = b8PpuScissorAlloc( cmd_ );
  b8PpuPushFrontOT( cmd_ , otz_ , pp );
  return pp;
}

b8PpuScissor* b8PpuScissorAllocZPB( b8PpuCmd* cmd_ , u32 otz_ ){
  b8PpuScissor* pp = b8PpuScissorAlloc( cmd_ );
  b8PpuPushBackOT( cmd_ , otz_ , pp );
  return pp;
}

b8PpuBg* b8PpuBgAlloc( b8PpuCmd* cmd_ ){
  b8PpuBg* pp = (b8PpuBg*)cmd_->sp;
  cmd_->sp = (u32*)(pp+1);
  pp->code = B8_PPU_CMD_BG;
  pp->vwrap = pp->uwrap = B8_PPU_BG_WRAP_CLAMP;
  return pp;
}

b8PpuBg* b8PpuBgAllocZ( b8PpuCmd* cmd_ , u32 otz_ ){
  b8PpuBg* pp = b8PpuBgAlloc( cmd_ );
  b8PpuPushFrontOT( cmd_ , otz_ , pp );
  return pp;
}

b8PpuBg* b8PpuBgAllocZPB( b8PpuCmd* cmd_ , u32 otz_ ){
  b8PpuBg* pp = b8PpuBgAlloc( cmd_ );
  b8PpuPushBackOT( cmd_ , otz_ , pp );
  return pp;
}

b8PpuPoly* b8PpuPolyAlloc( b8PpuCmd* cmd_ ){
  b8PpuPoly* pp = (b8PpuPoly*)cmd_->sp;
  cmd_->sp = (u32*)(pp+1);
  CHKOVL();
  pp->code = B8_PPU_CMD_POLY;
  return pp;
}

b8PpuPoly* b8PpuPolyAllocZ( b8PpuCmd* cmd_ , u32 otz_ ){
  b8PpuPoly* pp = b8PpuPolyAlloc( cmd_ );
  b8PpuPushFrontOT( cmd_ , otz_ , pp );
  return pp;
}

b8PpuPoly* b8PpuPolyAllocZPB( b8PpuCmd* cmd_ , u32 otz_ ){
  b8PpuPoly* pp = b8PpuPolyAlloc( cmd_ );
  b8PpuPushBackOT( cmd_ , otz_ , pp );
  return pp;
}

b8PpuLine* b8PpuLineAlloc( b8PpuCmd* cmd_ ){
  b8PpuLine* pp = (b8PpuLine*)cmd_->sp;
  cmd_->sp = (u32*)(pp+1);
  CHKOVL();
  pp->code = B8_PPU_CMD_LINE;
  return pp;
}

b8PpuLine* b8PpuLineAllocZ( b8PpuCmd* cmd_ , u32 otz_ ){
  b8PpuLine* pp = b8PpuLineAlloc( cmd_ );
  b8PpuPushFrontOT( cmd_ , otz_ , pp );
  return pp;
}

b8PpuLine* b8PpuLineAllocZPB( b8PpuCmd* cmd_ , u32 otz_ ){
  b8PpuLine* pp = b8PpuLineAlloc( cmd_ );
  b8PpuPushBackOT( cmd_ , otz_ , pp );
  return pp;
}

b8PpuViewoffset* b8PpuViewoffsetAlloc( b8PpuCmd* cmd_ ){
  b8PpuViewoffset* pp = (b8PpuViewoffset*)cmd_->sp;
  cmd_->sp = (u32*)(pp+1);
  CHKOVL();
  pp->code = B8_PPU_CMD_VIEWOFFSET;
  return pp;
}

b8PpuViewoffset* b8PpuViewoffsetAllocZ( b8PpuCmd* cmd_ , u32 otz_ ){
  b8PpuViewoffset* pp = b8PpuViewoffsetAlloc( cmd_ );
  b8PpuPushFrontOT( cmd_ , otz_ , pp );
  return pp;
}

b8PpuViewoffset* b8PpuViewoffsetAllocZPB( b8PpuCmd* cmd_ , u32 otz_ ){
  b8PpuViewoffset* pp = b8PpuViewoffsetAlloc( cmd_ );
  b8PpuPushBackOT( cmd_ , otz_ , pp );
  return pp;
}

b8PpuNop* b8PpuNopAlloc( b8PpuCmd* cmd_ ){
  b8PpuNop* pp = (b8PpuNop*)cmd_->sp;
  cmd_->sp = (u32*)(pp+1);
  CHKOVL();
  pp->code = B8_PPU_CMD_NOP;
  return pp;
}

b8PpuNop* b8PpuNopAllocZ( b8PpuCmd* cmd_ , u32 otz_ ){
  b8PpuNop* pp = b8PpuNopAlloc( cmd_ );
  b8PpuPushFrontOT( cmd_ , otz_ , pp );
  return pp;
}

b8PpuNop* b8PpuNopAllocZPB( b8PpuCmd* cmd_ , u32 otz_ ){
  b8PpuNop* pp = b8PpuNopAlloc( cmd_ );
  b8PpuPushBackOT( cmd_ , otz_ , pp );
  return pp;
}

b8PpuFlush* b8PpuFlushAlloc( b8PpuCmd* cmd_ ){
  b8PpuFlush* pp = (b8PpuFlush*)cmd_->sp;
  cmd_->sp = (u32*)(pp+1);
  CHKOVL();
  pp->code = B8_PPU_CMD_FLUSH;
  pp->img = pp->pal = 0;
  return pp;
}

b8PpuFlush* b8PpuFlushAllocZ( b8PpuCmd* cmd_ , u32 otz_ ){
  b8PpuFlush* pp = b8PpuFlushAlloc( cmd_ );
  b8PpuPushFrontOT( cmd_ , otz_ , pp );
  return pp;
}

b8PpuFlush* b8PpuFlushAllocZPB( b8PpuCmd* cmd_ , u32 otz_ ){
  b8PpuFlush* pp = b8PpuFlushAlloc( cmd_ );
  b8PpuPushBackOT( cmd_ , otz_ , pp );
  return pp;
}

b8PpuHalt* b8PpuHaltAlloc( b8PpuCmd* cmd_ ){
  b8PpuHalt* pp = (b8PpuHalt*)cmd_->sp;
  cmd_->sp = (u32*)(pp+1);
  CHKOVL();
  pp->code = B8_PPU_CMD_HALT;
  return pp;
}

b8PpuHalt* b8PpuHaltAllocZ( b8PpuCmd* cmd_ , u32 otz_ ){
  b8PpuHalt* pp = b8PpuHaltAlloc( cmd_ );
  b8PpuPushFrontOT( cmd_ , otz_ , pp );
  return pp;
}

b8PpuHalt* b8PpuHaltAllocZPB( b8PpuCmd* cmd_ , u32 otz_ ){
  b8PpuHalt* pp = b8PpuHaltAlloc( cmd_ );
  b8PpuPushBackOT( cmd_ , otz_ , pp );
  return pp;
}

b8PpuEnable* b8PpuEnableAlloc( b8PpuCmd* cmd_ ){
  b8PpuEnable* pp = (b8PpuEnable*)cmd_->sp;
  cmd_->sp = (u32*)(pp+1);
  CHKOVL();
  pp->code = B8_PPU_CMD_ENABLE;
  pp->cul = 0;
  return pp;
}

b8PpuEnable* b8PpuEnableAllocZ( b8PpuCmd* cmd_ , u32 otz_ ){
  b8PpuEnable* pp = b8PpuEnableAlloc( cmd_ );
  b8PpuPushFrontOT( cmd_ , otz_ , pp );
  return pp;
}

b8PpuEnable* b8PpuEnableAllocZPB( b8PpuCmd* cmd_ , u32 otz_ ){
  b8PpuEnable* pp = b8PpuEnableAlloc( cmd_ );
  b8PpuPushBackOT( cmd_ , otz_ , pp );
  return pp;
}

b8PpuLoadimg* b8PpuLoadimgAlloc( b8PpuCmd* cmd_ ){
  b8PpuLoadimg* pp = (b8PpuLoadimg*)cmd_->sp;
  cmd_->sp = (u32*)(pp+1);
  CHKOVL();
  pp->code = B8_PPU_CMD_LOADIMG;
  return pp;
}

b8PpuLoadimg* b8PpuLoadimgAllocZ( b8PpuCmd* cmd_ , u32 otz_ ){
  b8PpuLoadimg* pp = b8PpuLoadimgAlloc( cmd_ );
  b8PpuPushFrontOT( cmd_ , otz_ , pp );
  return pp;
}

b8PpuLoadimg* b8PpuLoadimgAllocZPB( b8PpuCmd* cmd_ , u32 otz_ ){
  b8PpuLoadimg* pp = b8PpuLoadimgAlloc( cmd_ );
  b8PpuPushBackOT( cmd_ , otz_ , pp );
  return pp;
}

b8PpuJmp* b8PpuJmpAlloc( b8PpuCmd* cmd_ , u32* cpuaddr_ ){
  b8PpuJmp* pp = (b8PpuJmp*)cmd_->sp;
  cmd_->sp = (u32*)(pp+1);
  CHKOVL();
  pp->code = B8_PPU_CMD_JMP;

  union fc32 fc;
  fc.pU32 = cpuaddr_;
  pp->cpuaddr4 = fc.aU32>>2;

  return pp;
}

void  b8PpuClearOT( b8PpuCmd* cmd_ , u32* ot_ , u32* ot_prev_ , u32 num_ ){
  cmd_->ot = ot_;
  cmd_->ot_prev = ot_prev_;
  cmd_->otnum = num_;
  b8PpuJmpAlloc( cmd_ , &ot_[ num_ - 1 ] );

  cmd_->addr_halt = (u32*)b8PpuHaltAlloc( cmd_ );

  for( u32 otz=0 ; otz < num_ ; ++otz ){
    b8PpuJmp* jmp = (b8PpuJmp*) &ot_[ otz ];
    jmp->code = B8_PPU_CMD_JMP;

    union fc32 fc_prev;
    if( 0 == otz ){
      fc_prev.pU32 = cmd_->addr_halt;
    } else {
      fc_prev.pU32 = &ot_[ otz - 1];
    }
    jmp->cpuaddr4 = fc_prev.aU32>>2;

    union fc32 fc;
    fc.pU32 = ot_ + otz;
    cmd_->ot_prev[ otz ] = fc.aU32;
  }
}

void  b8PpuPushFrontOT( b8PpuCmd* cmd_ , u32 otz_ , void* prim_ ){
  _ASSERT( otz_ < cmd_->otnum , "invalid otz_" );

  *( cmd_->sp++ ) = *(cmd_->ot + otz_);
  CHKOVL();

  union fc32 fc_prim;
  fc_prim.pU32 = prim_;

  union fc32 fc_ot;
  fc_ot.pJmp = (b8PpuJmp*)(cmd_->ot + otz_);
  fc_ot.pJmp->cpuaddr4 = fc_prim.aU32>>2;
}

void  b8PpuPushBackOT( b8PpuCmd* cmd_ , u32 otz_ , void* prim_ ){
  _ASSERT( otz_ < cmd_->otnum , "invalid otz_" );

  union fc32 fc_jmp;
  fc_jmp.aU32 = cmd_->ot_prev[ otz_ ];

  union fc32 fc_jmp_back;
  fc_jmp_back.pU32 = cmd_->sp;
  *( cmd_->sp++ ) = *fc_jmp.pU32;
  CHKOVL();

  union fc32 fc_prim;
  fc_prim.pU32 = prim_;
  fc_jmp.pJmp->cpuaddr4 = fc_prim.aU32>>2;

  cmd_->ot_prev[ otz_ ] = fc_jmp_back.aU32;
}

void  b8PpuReset( void ){
  b8SysSetupIrqWait( B8_IRQ_VBLK );
  b8PpuEnableVblankInterrupt();
}

void  b8PpuVsyncWait( void ){
  b8SysIrqClearAndWait( B8_IRQ_VBLK );
}

void  b8PpuGetResolution( u32* ww, u32* hh ){
  const u32 res = B8_PPU_RESOLUTION;
  *ww = (res >> 16);
  *hh = res & 0xffff;
}
