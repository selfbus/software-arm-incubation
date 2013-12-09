/*
 * (c) Code Red Technologies Ltd, 2008-2013
 * (c) NXP Semiconductors 2013-2013
 * Generated linker script file for LPC11C24/301
 * Created from generic_c.ld (LPCXpresso v6.1 (0 [Build 164] [2013-10-21] ))
 * By LPCXpresso v6.1.0 [Build 164] [2013-10-21]  on Mon Dec 09 10:10:48 CET 2013
 */


INCLUDE "../linker-scripts/sblib_lib.ld"
INCLUDE "../linker-scripts/sblib_mem.ld"

ENTRY(ResetISR)

SECTIONS
{

    .text_Flash2 (NOLOAD) : ALIGN(4)
    {
       FILL(0xff)
       /* First two entries, below, are for compatibility with previous releases */
      *(.text_Flash2*)
      *(.text_EEPROM*)
      *(.text.$Flash2*)
      *(.text.$EEPROM*)
      *(.rodata.$Flash2*)
      *(.rodata.$EEPROM*)
    } > EEPROM

    /* MAIN TEXT SECTION */
    .text : ALIGN(4)
    {
        FILL(0xff)
        __vectors_start__ = ABSOLUTE(.) ;
        KEEP(*(.isr_vector))

        /* Global Section Table */
        . = ALIGN(4) ;
        __section_table_start = .;
        __data_section_table = .;
        LONG(LOADADDR(.data));
        LONG(    ADDR(.data)) ;
        LONG(  SIZEOF(.data));
        __data_section_table_end = .;
        __bss_section_table = .;
        LONG(    ADDR(.bss));
        LONG(  SIZEOF(.bss));
        __bss_section_table_end = .;
        __section_table_end = . ;
        /* End of Global Section Table */


        *(.after_vectors*)

        /* Code Read Protect data */
        . = 0x000002FC ;
        PROVIDE(__CRP_WORD_START__ = .) ;
        KEEP(*(.crp))
        PROVIDE(__CRP_WORD_END__ = .) ;
        ASSERT(!(__CRP_WORD_START__ == __CRP_WORD_END__), "Linker CRP Enabled, but no CRP_WORD provided within application");
        /* End of Code Read Protect */

    } >MFlash32

    .text : ALIGN(4)
    {
         *(.text*)
        *(.rodata .rodata.*)
        . = ALIGN(4);

    } > MFlash32

    /*
     * for exception handling/unwind - some Newlib functions (in common
     * with C++ and STDC++) use this.
     */
    .ARM.extab : ALIGN(4)
    {
      *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > MFlash32
    __exidx_start = .;

    .ARM.exidx : ALIGN(4)
    {
      *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > MFlash32
    __exidx_end = .;

    _etext = .;


    /* MAIN DATA SECTION */


    .uninit_RESERVED : ALIGN(4)
    {
        KEEP(*(.bss.$RESERVED*))
        . = ALIGN(4) ;
        _end_uninit_RESERVED = .;
    } > RamLoc8


  /* Main DATA section (RamLoc8) */
  .data : ALIGN(4)
  {
     FILL(0xff)
     _data = . ;
     *(vtable)
     *(.data*)
     . = ALIGN(4) ;
     _edata = . ;
  } > RamLoc8 AT>MFlash32


    /* MAIN BSS SECTION */
    .bss : ALIGN(4)
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4) ;
        _ebss = .;
        PROVIDE(end = .);
    } > RamLoc8


    /* DEFAULT NOINIT SECTION */
    .noinit (NOLOAD): ALIGN(4)
    {
        _noinit = .;
        *(.noinit*)
         . = ALIGN(4) ;
        _end_noinit = .;
    } > RamLoc8

    PROVIDE(_pvHeapStart = .);
    PROVIDE(_vStackTop = __top_RamLoc8 - 0);
}
