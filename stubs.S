#define IMPORT_BEGIN(lib) \
.section .fimport_##lib, "ax", @0x80000002; \
.align 4; \
.long 1; \
.long 0x00000000; \
.string #lib; \
.align 4

#define IMPORT(name) \
.global name; \
.type name, @function; \
name:; \
.long 0x0; \
.long 0x0

#define IMPORT_END() \
.align 4


#include "imports.h"

/*.section ".text"
.globl SC0x25_KernelCopyData
SC0x25_KernelCopyData:
li r0, 0x2500
sc
blr
*/
