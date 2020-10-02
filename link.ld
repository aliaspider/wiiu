OUTPUT_FORMAT("elf32-powerpc")
OUTPUT_ARCH(powerpc:common)

MEMORY {
   code (rx) : ORIGIN = 0x02000000, LENGTH = 224M
   data (rw) : ORIGIN = 0x10000000, LENGTH = 800M
   load (rx) : ORIGIN = 0xC0000000, LENGTH = 128M
}

ENTRY(__rpx_start)

PHDRS {
   hdr_text    PT_LOAD  FLAGS(0x01 | 0x04);
   hdr_data    PT_LOAD  FLAGS(0x02 | 0x04);
   hdr_sdata   PT_LOAD  FLAGS(0x02 | 0x04);
   hdr_tdata   PT_LOAD  FLAGS(0x02 | 0x04);
   hdr_load    PT_LOAD  FLAGS(0x01 | 0x04);
}

SECTIONS
{
   . = ORIGIN(code);
   __code_start = .;
   .syscall : {
      *(.syscall .syscall.*)
   } > code : hdr_text

   .text : {
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
   } > code
   __code_end = .;

   . = ORIGIN(data);
   __SDATA2_START__ = .;
   .sdata2 : {
      *(.sdata2)
      *(.sdata2.*)
      *(.gnu.linkonce.s2.*)
   } > data : hdr_data

   .rodata : {
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

   .data : {
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

   .bss (NOLOAD) : {
      *(.dynbss)
      *(.bss)
      *(.bss.*)
      *(.gnu.linkonce.b.*)
      *(COMMON)
   } > data

   __SDATA_START__ = .;
   .sdata : {
      *(.sdata)
      *(.sdata.*)
      *(.gnu.linkonce.s.*)
   } > data : hdr_sdata

   .sbss (NOLOAD) : {
      *(.dynsbss)
      *(.sbss)
      *(.sbss.*)
      *(.gnu.linkonce.sb.*)
      *(.scommon)
   } > data

   .thrdata : {
      *(.tdata)
      *(.tdata.*)
      *(.gnu.linkonce.td.*)
   } > data : hdr_tdata

   .thrbss (NOLOAD) : {
      *(.tbss)
      *(.tbss.*)
      *(.gnu.linkonce.tb.*)
      *(.tcommon)
   } > data

   . = ORIGIN(load);
   .fexports ALIGN(32) : { KEEP( *(.fexports) ) } > load : hdr_load
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
