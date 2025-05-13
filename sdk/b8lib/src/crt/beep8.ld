OUTPUT_FORMAT( "elf32-littlearm", "elf32-bigarm", "elf32-littlearm" )
OUTPUT_ARCH(arm)

SEARCH_DIR( . )

MEMORY {
	rom(wx)  : org = 0x00000000,	len = 0x00100000
	wram(wx) : org = 0x00100000,	len = 0x00100000
}
__wram_end	= ORIGIN(wram)  + LENGTH(wram);
__stack_top	= __wram_end  - 0x3000;
__sys_stack = __wram_end;
__svc_stack = __wram_end;
__irq_stack = __wram_end;
__dbt_stack = __wram_end;

SECTIONS {
  .vectors :
  {
    _svect = . ;
    KEEP(*(.vectors .vectors.*))
  } > rom

	.text :
	{
    _sec_text_rom_s = ABSOLUTE(LOADADDR(.text)) ;
    _bootstart = .;

		KEEP (*(.bootstart))

		libgcc.a(.text .text.* .rodata .ARM.extab .ARM.extab.*)

		. = ALIGN(8) ;

		*(.text.bootconfig .text.bootconfig.*)

		. = ALIGN(8) ;
		*(.text.builtin)
		*(.text.startup .text.startup.*)
		*(.text.unlikely .text.*_unlikely)
		*(.text.exit .text.exit.*)
		*(.text.hot .text.hot.*)
		*(.text .stub .text.* .gnu.linkonce.t.*)
		/* *(.ARM.exidx*) */

		. = ALIGN(8) ;
		KEEP (*(SORT_NONE(.init)))
		KEEP (*(SORT_NONE(.fini)))

		. = ALIGN(8) ;

    _sec_text_rom_e = .;
	} > rom


  .ARM.exidx ALIGN(8):
  {
    __exidx_start = .;
    _sectop_exidx  = ABSOLUTE(LOADADDR(.ARM.exidx)); /* ARM.exidx       */
    _sectop_exidxR = .;                              /* ARM.exidxR      */
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    _secend_exidxR = .;
    _secend_exidx  = ABSOLUTE(LOADADDR(.ARM.exidx)) + (_secend_exidxR - _sectop_exidxR);
    __exidx_end = .;
  } > rom

  .cinit :
	{
		_sectop_CINIT  = ABSOLUTE(LOADADDR(.cinit)) ;
		_sectop_CINITR = . ;

		. = ALIGN(8) ;
		__ctors = . ;
		KEEP (*crtbegin.o(.ctors))
		KEEP (*crtbegin?.o(.ctors))
		KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
		KEEP (*(SORT(.ctors.*)))
		KEEP (*(.ctors))
		KEEP (*crtbegin.o(.init_array))
		KEEP (*crtbegin?.o(.init_array))
		KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .init_array))
		KEEP (*(SORT(.init_array.*)))
		KEEP (*(.init_array))
		__ctors_end = . ;

		. = ALIGN(8) ;
		__dtors = . ;
		KEEP (*crtbegin.o(.dtors))
		KEEP (*crtbegin?.o(.dtors))
		KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
		KEEP (*(SORT(.dtors.*)))
		KEEP (*(.dtors))
		KEEP (*crtbegin.o(.fini_array))
		KEEP (*crtbegin?.o(.fini_array))
		KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .fini_array))
		KEEP (*(SORT(.fini_array.*)))
		KEEP (*(.fini_array))
		__dtors_end = . ;
		. = ALIGN(8) ;
		_secend_CINITR = . ;
		_secend_CINIT  = ABSOLUTE(LOADADDR(.cinit)) + (_secend_CINITR - _sectop_CINITR) ;
	} > rom

	.rodata :
	{
		_sec_rodata_rom_s = ABSOLUTE(LOADADDR(.rodata)) ;

    . = ALIGN(8) ;
		*(.rodata .rodata.* .gnu.linkonce.r.*)

    . = ALIGN(8);
    *(.ARM.extab* .gnu.linkonce.armextab.*)

		. = ALIGN(8) ;
		*(.eh_frame_hdr)

		KEEP (*(.eh_frame))
		*(.gcc_except_table .gcc_except_table.*)
		KEEP (*(.jcr))

		. = ALIGN(8) ;

		_sec_rodata_rom_e = . ;
	} > rom

  .data :
	{
		_sec_data_rom_s = ABSOLUTE(LOADADDR(.data)) ;
		_sec_data_ram_s = . ;

		. = ALIGN(8) ;
		*(.data .data.* .gnu.linkonce.d.*)
		*(.got.plt) *(.igot.plt) *(.got) *(.igot)

		. = ALIGN(8) ;
		_sec_data_ram_e = . ;
  } > wram AT > rom

  .bss :
	{
		/* . = ALIGN(8) ; */
		_sec_bss_ram_s = .;

		. = ALIGN(8) ;
		*(.dynbss)
		*(.bss .bss.* .gnu.linkonce.b.*)
		*(COMMON)

		. = ALIGN(8) ;
		_sec_bss_ram_e = .;
    end_of_bss = .;
	} > wram

  .note.GNU-stack :
  {
    *(.note.GNU-stack)
  } > wram

  .gnu_debuglink :
  {
    KEEP(*(.gnu_debuglink))
  } > rom

  .ARM.attributes : {
  *(.ARM.attributes)
} > rom

/DISCARD/ : { *(.gnu.lto_text .gnu.lto_text.*) *(.comment) *(.debug_frame) }
}