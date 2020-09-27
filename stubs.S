#define FIMPORT_BEGIN(lib) \
.section .fimport_##lib, "ax", @0x80000002; \
.align 2; \
1: \
.long 1; \
.long 0x00000000; \
.string #lib; \
.align 3

#define FIMPORT(name) \
.section %S.name, "ax", @0x80000002; \
.align 2; \
.global name; \
.type name, @function; \
name:; \
.long 1b; \
.long 0x0; \
.previous

#define DIMPORT_BEGIN(lib) \
.section .dimport_##lib, "a", @0x80000002; \
.align 2; \
1: \
.long 1; \
.long 0x00000000; \
.string #lib; \
.align 3

#define DIMPORT(name) \
.section %S.name, "a", @0x80000002; \
.align 2; \
.global name; \
.type name, @function; \
name:; \
.long 1b; \
.long 0x0; \
.previous


#include "imports.h"

/*.section ".text"
.globl SC0x25_KernelCopyData
SC0x25_KernelCopyData:
li r0, 0x2500
sc
blr
*/
