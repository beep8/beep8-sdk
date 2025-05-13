// BEEP-8 Boot loader
#define REG_0 (0)
#define REG_13SP (13)
#define REG_15PC (15)
#define REG_PSR (16)

#define USR_MODE  (16)
#define FIQ_MODE  (17)
#define IRQ_MODE  (18)
#define SVC_MODE  (19)
#define MON_MODE  (20)
#define ABT_MODE  (23)
#define UND_MODE  (27)
#define SYS_MODE  (31)
#define DISABLE_FIQ       0x00000040 /* Bit 6: FIQ disable */
#define DISABLE_IRQ       0x00000080 /* Bit 7: IRQ disable */

.file "bootloader.S"
.section    .vectors,"a",%progbits
__arm_reset:
    ldr     pc, =__bootloader
__arm_udf:
    ldr     pc, =__undef
__arm_svc:
    ldr     pc, =__svc_dispatch
__arm_pref:
    b       __arm_pref
__arm_data:
    ldr     pc, =__data_abort
    nop
__arm_irq:
    ldr     pc, =__irq_dispatch
__arm_fiq:
    b       __arm_fiq
__beep8_signature:
    nop     /* It must be nop. DON'T MODIFY */
__beep8_reserved:
    .byte 'B', 'P', '8', 0
    .fill 216, 1, 0x00

.section    .bootstart,"ax",%progbits

.global crt0_entry
.type   crt0_entry, %function

.global b8OsIrqDispatchEntry
.type   b8OsIrqDispatchEntry, %function

.global _b8OsSvcDispatch
.type   _b8OsSvcDispatch, %function

.long __sys_stack
.long __irq_stack
.long __svc_stack

__bootloader:
  ldr   sp, =__svc_stack
  bic		sp, sp, #7
  mov		r0, #(SVC_MODE|DISABLE_FIQ|DISABLE_IRQ)
  msr		cpsr_c, r0

  mov		r0, #(IRQ_MODE|DISABLE_FIQ|DISABLE_IRQ)
  msr		cpsr_c, r0
  ldr   sp, =__irq_stack
  bic		sp, sp, #7

  mov		r0, #(SYS_MODE|DISABLE_FIQ|DISABLE_IRQ)
  msr		cpsr_c, r0
  ldr   sp, =__sys_stack
  bic		sp, sp, #7

  mov		r0, #(USR_MODE)
  msr		cpsr_c, r0

  bl    crt0_entry
  hlt

__irq_dispatch:
  ldr   sp, =b8OsUsrContext + 4*REG_15PC
  str   lr,[sp]    // r14(=lr) = address of next instruction to be executed + 4. "subs pc,r14,#4"

  // store r0-r12
  ldr   lr, =b8OsUsrContext + 4*REG_0
  stmia	lr, {r0-r12}

  // store psr
  ldr   sp, =b8OsUsrContext + 4*REG_PSR
  mrs   lr,spsr    // SPSR_irq = CPSR when an IRQ is detected
  str   lr,[sp]

  mov		lr, #(SYS_MODE|DISABLE_FIQ|DISABLE_IRQ)
  msr		cpsr_c, lr

  // store r13-r14
  ldr   r0, =b8OsUsrContext + 4*REG_13SP
  stmia r0, {r13-r14}

  mov		r0, #(IRQ_MODE|DISABLE_FIQ|DISABLE_IRQ)
  msr		cpsr_c, r0

  ldr   sp, =__irq_stack
  bic		sp, sp, #7  // Force 8-byte alignment
  bl    b8OsIrqDispatchEntry
  // Won't get here

.global _b8OsIrq2Usr
_b8OsIrq2Usr:
  mov		lr, #(SYS_MODE|DISABLE_FIQ|DISABLE_IRQ)
  msr		cpsr_c, lr

  // load r13 - r14
  ldr   r0, =b8OsUsrContext + 4*REG_13SP
  ldmia r0,{r13-r14}

  mov   r0, #(IRQ_MODE|DISABLE_FIQ|DISABLE_IRQ)
  msr   cpsr_c, r0

  // load r0 - 12
  ldr   sp, =b8OsUsrContext + 4*REG_0
  ldmia sp,{r0-r12}

  // load psr
  ldr   sp, =b8OsUsrContext + 4*REG_PSR
  ldr   lr,[sp]
  msr   spsr,lr

  // load r15pc
  ldr sp, =b8OsUsrContext + 4*REG_15PC
  ldr lr,[sp]
  subs pc,lr,#4

.global b8rst
b8rst:
  ldr   pc, =__bootloader

.global _b8OsGetCPSR
_b8OsGetCPSR:
  mrs r0, CPSR
  mov pc, lr

.global b8OsSysCall
b8OsSysCall:
  svc #0
  mov pc, lr


.global crt0_data_abort
.type   crt0_data_abort, %function
__data_abort:
  ldr   sp, =__dbt_stack 
  bic		sp, sp, #7  // Force 8-byte alignment
  sub   r0, lr, #8
  mrc   p15, 0, r1, c6, c0, 0
  mrc   p15, 0, r2, c5, c0, 0

  // to System Mode
  mrs   r5, spsr
  bic   r4, r5, #0x1F
  orr   r4, r4, #0x1F              // System Mode(0x1F)
  msr   cpsr_c, r4

  mov   r3, sp

  // to DataAbort Mode 
  bic   r4, r5, #0x1F
  orr   r4, r4, #0x17              // DataAbort Mode(0x17)
  msr   cpsr_c, r4
  
  mrs   r4, spsr
  push  {r4}
  bl    crt0_data_abort
  add   sp, sp, #4

  hlt

.global crt0_undef
.type   crt0_undef, %function
__undef:
  ldr   sp, =__dbt_stack  // Intentionally sharing the stack with Data Abort
  bic		sp, sp, #7        // Force 8-byte alignment
  sub   r0, lr, #4
  mrs   r1, spsr
  bl    crt0_undef
  hlt

/*
  Software Interrupt exception
    The Software Interrupt instruction (SWI) enters Supervisor mode to request a particular supervisor (operating
    system) function. When a SWI is executed, the following actions are performed:

      R14_svc = address of next instruction after the SWI instruction
      SPSR_svc = CPSR
      CPSR[4:0] = 0b10011 // Enter Supervisor mode
      CPSR[5] = 0 // Execute in ARM state
      // CPSR[6] is unchanged
      CPSR[7] = 1 // Disable normal interrupts
      // CPSR[8] is unchanged
      CPSR[9] = CP15_reg1_EEbit // Endianness on exception entry
      PC = 0x00000008

    To return after performing the SWI operation, use the following instruction to restore the PC
    (from R14_svc) and CPSR (from SPSR_svc) and return to the instruction following the SWI:
    MOVS PC,R14
*/
__svc_dispatch:
  ldr   sp, =b8OsUsrContext + 4*REG_15PC
  str   lr,[sp]    // r14_svc (lr) = Address of next instruction after the SWI instruction

  // store r0-r12
  ldr   sp, =b8OsUsrContext + 4*REG_0
  stmia	sp, {r0-r12}

  // store psr
  ldr   sp, =b8OsUsrContext + 4*REG_PSR
  mrs   lr,spsr    // SPSR_svc = CPSR when an SVC is detected
  str   lr,[sp]

  mov		lr, #(SYS_MODE|DISABLE_FIQ|DISABLE_IRQ)
  msr		cpsr_c, lr

  // store r0-r3 as arg
  ldr   r7, =b8OsSysCallArgs + 4*0
  stmia r7, {r0-r3}

  // store r4-r6 as arg
  ldmia sp, {r4-r6}
  ldr   r7, =b8OsSysCallArgs + 4*4
  stmia r7,{r4-r6}

  // store r13-r14
  ldr   r7, =b8OsUsrContext + 4*REG_13SP
  stmia r7, {r13-r14}

  mov		r7, #(SVC_MODE|DISABLE_FIQ|DISABLE_IRQ)
  msr		cpsr_c, r7

  ldr   sp, =__svc_stack
  bic		sp, sp, #7  // Force 8-byte alignment
  bl    _b8OsSvcDispatch
  // Won't get here

.global _b8OsSvc2Usr
_b8OsSvc2Usr:
  mov		lr, #(SYS_MODE|DISABLE_FIQ|DISABLE_IRQ)
  msr		cpsr_c, lr

  // load r13 - r14
  ldr   r12, =b8OsUsrContext + 4*REG_13SP
  ldmia r12,{r13-r14}

  mov   r12, #(SVC_MODE|DISABLE_FIQ|DISABLE_IRQ)
  msr   cpsr_c, r12

  // load r0 - 12
  ldr   sp, =b8OsUsrContext + 4*REG_0
  ldmia sp,{r0-r12}

  // load psr
  ldr   sp, =b8OsUsrContext + 4*REG_PSR
  ldr   lr,[sp]
  msr   spsr,lr

  // load r15pc
  ldr   sp, =b8OsUsrContext + 4*REG_15PC
  ldr   lr,[sp]
  movs  pc,lr
  // Won't get here
