OUTPUT_FORMAT("elf32-powerpc")
OUTPUT_ARCH(powerpc:common)

MEMORY {
   code (rx) : ORIGIN = 0x02000000, LENGTH = 224M
   data (rw) : ORIGIN = 0x10000000, LENGTH = 800M
   load (rx) : ORIGIN = 0xC0000000, LENGTH = 128M
}

ENTRY(__rpx_start)

SECTIONS
{
   . = ORIGIN(code);
   .syscall ALIGN(32) : {
      __code_start = .;
      *(.syscall .syscall.*)
   } > code

   .text ALIGN(32) : {
      *(.text)
      *(.text.*)
      *(.gnu.warning)
      *(.gnu.linkonce.t.*)
      *(.got)
      *(.got1)
      *(.got2)
      *(.got.plt)
      *(.plt)
      . = ALIGN(4);
      __code_end = .;
   } > code

   . = ORIGIN(data);
   .sdata2 ALIGN(32): {
      PROVIDE(__sdata2_start = .);
      *(.sdata2)
      *(.sdata2.*)
      *(.gnu.linkonce.s2.*)
      PROVIDE(__sdata2_end = .);
   } > data

   .rodata ALIGN(256): {
      *(.rodata)
      *(.rodata.*)
      *(.gnu.linkonce.r.*)

      *(.jcr)
      *(.dynamic)
      *(.eh_frame_hdr)
      KEEP(*(.eh_frame))
      *(.fixup)
      *(.gcc_except_table)
      *(.gcc_except_table.*)
      *(.tm_clone_table)
   } > data

   .data ALIGN(64): {
      *(.data)
      *(.data.*)
      *(.gnu.linkonce.d.*)

      . = ALIGN(4);
      PROVIDE(__CTOR_LIST__ = .);
      KEEP (*(SORT(.ctors.*)))
      KEEP (*(.ctors))
      PROVIDE(__CTOR_END__ = .);

      . = ALIGN(4);
      PROVIDE(__DTOR_LIST__ = .);
      KEEP (*(SORT(.dtors.*)))
      KEEP (*(.dtors))
      PROVIDE(__DTOR_END__ = .);
   } > data

   PROVIDE(__bss_start = .);
   .bss ALIGN(256) (NOLOAD) : {
      *(.dynbss)
      *(.bss)
      *(.bss.*)
      *(.gnu.linkonce.b.*)
      *(COMMON)
   } > data
   PROVIDE(__bss_end = .);

   .sdata ALIGN(32): {
      PROVIDE(__sdata_start = .);
      *(.sdata)
      *(.sdata.*)
      *(.gnu.linkonce.s.*)
      PROVIDE(__sdata_end = .);
   } > data

   .sbss ALIGN(64) (NOLOAD) : {
      PROVIDE(__sbss_start = .);
      *(.dynsbss)
      *(.sbss)
      *(.sbss.*)
      *(.gnu.linkonce.sb.*)
      *(.scommon)
      PROVIDE(__sbss_end = .);
   } > data

   .thrdata ALIGN(32) : {
      *(.tdata)
      *(.tdata.*)
      *(.gnu.linkonce.td.*)
   } > data

   .thrbss ALIGN(32) : {
      *(.tbss)
      *(.tbss.*)
      *(.gnu.linkonce.tb.*)
      *(.tcommon)
   } > data

   . = ORIGIN(load);
   .fexports ALIGN(32) : { KEEP( *(.fexports) ) } > load
   .dexports ALIGN(32) : { KEEP( *(.dexports) ) } > load

   INCLUDE "imports.ld"

   /DISCARD/ : {
      *(.abs)
      *(.comment)
      *(.gnu.attributes)
      *(.gnu.version)
      *(.gnu.version_d)
      *(.gnu.version_r)
   }
}
