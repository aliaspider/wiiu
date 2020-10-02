/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2017 Ash Logan (QuarkTheAwesome)
 *
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

/* TODO: Program exceptions don't seem to work. Good thing they almost never happen. */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <malloc.h>
#include <inttypes.h>
#include <wiiu/os.h>
#include <wiiu/sysapp.h>
#include <wiiu/vpad.h>
#include <wiiu/gx2.h>
#include "exception_handler.h"

/*	Settings */
#define NUM_STACK_TRACE_LINES 30
#define VISIBLE_STACK_TRACE_LINES 5

/*	Externals
	From the linker scripts.
*/

void __code_start();
void __code_end();

#define TEXT_START (unsigned int)&__code_start
#define TEXT_END (unsigned int)&__code_end

void test_os_exceptions(void);
void exception_print_symbol(uint32_t addr);

typedef struct _framerec
{
   struct _framerec* up;
   void* lr;
} frame_rec, *frame_rec_t;

#define dsisr exception_specific0
#define dar exception_specific1

__attribute__((weak)) const char *PROGRAM_NAME;
__attribute__((weak)) const char *PROGRAM_VERSION;

/*	Some bitmasks for determining DSI causes.
	Taken from the PowerPC Programming Environments Manual (32-bit).
*/

/* Set if the EA is unmapped. */
#define DSISR_TRANSLATION_MISS 0x40000000
/* Set if the memory accessed is protected. */
#define DSISR_TRANSLATION_PROT 0x8000000
/* Set if certain instructions are used on uncached memory (see manual) */
#define DSISR_BAD_CACHING 0x4000000
/* Set if the offending operation is a write, clear for a read. */
#define DSISR_WRITE_ATTEMPTED 0x2000000
/* Set if the memory accessed is a DABR match */
#define DSISR_DABR_MATCH 0x400000
/*	ISI cause bitmasks, same source */
#define SRR1_ISI_TRANSLATION_MISS 0x40000000
#define SRR1_ISI_TRANSLATION_PROT 0x8000000
/*	PROG cause bitmasks, guess where from */
/* Set on floating-point exceptions */
#define SRR1_PROG_IEEE_FLOAT 0x100000
/* Set on an malformed instruction (can't decode) */
#define SRR1_PROG_BAD_INSTR 0x80000
/* Set on a privileged instruction executing in userspace */
#define SRR1_PROG_PRIV_INSTR 0x40000
/* Set on a trap instruction */
#define SRR1_PROG_TRAP 0x20000
/* Clear if srr0 points to the address that caused the exception (yes, really) */
#define SRR1_PROG_SRR0_INACCURATE 0x10000

#define buf_add(...) wiiu_exception_handler_pos += sprintf(exception_msgbuf + wiiu_exception_handler_pos, __VA_ARGS__)
size_t wiiu_exception_handler_pos = 0;
char* exception_msgbuf;

void net_print_exp(const char *str);

static void disasm_printfcallback(const char* fmt, ...)
{
   va_list args;
   va_start(args, fmt);
   wiiu_exception_handler_pos += vsprintf(exception_msgbuf + wiiu_exception_handler_pos, fmt,  args);
   va_end(args);
}

static void print_and_abort()
{
   DEBUG_LINE();
//   GX2ResetGPU();
   puts(exception_msgbuf);
   DEBUG_STR(OSGetThreadName(OSGetCurrentThread()));
   DEBUG_VAR(MEM2_avail());
#if 1
   OSScreenInit();
   GX2SetTVEnable(GX2_ENABLE);
	GX2SetDRCEnable(GX2_ENABLE);

   int lines = 1;
   int i;
   char* ptr = exception_msgbuf;
   while(*ptr)
   {
      if(*ptr == '\n') {
         *ptr = '\0';
         lines ++;
      }
      ptr++;
   }

   OSScreenSetBufferEx(SCREEN_DRC, (u8*)0xF4000000);
   OSScreenClearBufferEx(SCREEN_DRC, 0);
   ptr = exception_msgbuf;
   for(i = 0; i < lines; i++)
   {
      OSScreenPutFontEx(SCREEN_DRC, -4, i - 1, ptr);
      ptr += strlen(ptr) + 1;
   }
   OSScreenFlipBuffersEx(SCREEN_DRC);
   OSScreenClearBufferEx(SCREEN_DRC, 0);
   ptr = exception_msgbuf;
   for(i = 0; i < lines; i++)
   {
      OSScreenPutFontEx(SCREEN_DRC, -4, i - 1, ptr);
      ptr += strlen(ptr) + 1;
   }
   OSScreenPutFontEx(SCREEN_DRC, 65 - 25, 17, "Press Any Button to Exit.");
   OSScreenFlipBuffersEx(SCREEN_DRC);

   OSScreenSetBufferEx(SCREEN_TV, (u8*)0xF4000000 + OSScreenGetBufferSizeEx(SCREEN_DRC));
   OSScreenClearBufferEx(SCREEN_TV, 0);
   ptr = exception_msgbuf;
   for(i = 0; i < lines; i++)
   {
      OSScreenPutFontEx(SCREEN_TV, -4, i - 1, ptr);
      ptr += strlen(ptr) + 1;
   }
   OSScreenFlipBuffersEx(SCREEN_TV);

   VPADSetSamplingCallback(0, NULL);
   OSTime time = OSGetSystemTime();
   while (true)
   {
      OSSleepTicks(ms_to_ticks(1));
      VPADStatus vpad = {};
      VPADReadError error = 0;
      VPADRead(0, &vpad, 1, &error);
      if(!error && vpad.trigger)
         break;
      if((time + sec_to_ticks(1)) < OSGetSystemTime())
      {
         OSScreenFlipBuffersEx(SCREEN_DRC);
         time += sec_to_ticks(1);
      }
   }
#else
   OSFatal(exception_msgbuf);
#endif
   __attribute__((noreturn)) void __shutdown_program(void);
   __shutdown_program();
}
void exception_print_lineinfo(uint32_t addr)
{
   if (addr >= TEXT_START && addr < TEXT_END)
      addr -= (TEXT_START - 0x02000000);

   buf_add("info line *0x%08" PRIX32 "\n", addr);
}

BOOL exception_cb(OSContext* ctx, OSExceptionType type, OSExceptionCallbackFn prev_cb)
{
   /*	No message buffer available, fall back onto MEM1 */
   if (!exception_msgbuf || !OSEffectiveToPhysical(exception_msgbuf))
      exception_msgbuf = (char*)0xF4000000 + OSScreenGetBufferSizeEx(SCREEN_DRC) + OSScreenGetBufferSizeEx(SCREEN_TV);

   buf_add("   ");
   /*	First up, the pretty header that tells you wtf just happened */
   if (type == OS_EXCEPTION_TYPE_DSI)
   {
      /*	Exception type and offending instruction location */
      buf_add("DSI: Instr at %08" PRIX32, ctx->srr0);

      if (ctx->dsisr & DSISR_DABR_MATCH)
         buf_add(" matched");
      else
         buf_add(" bad");

      /*	Was this a read or a write? */
      if (ctx->dsisr & DSISR_WRITE_ATTEMPTED)
         buf_add(" write to");
      else
         buf_add(" read from");

      /*	So why was it bad?
         Other causes (DABR) don't have a message to go with them. */
      if (ctx->dsisr & DSISR_TRANSLATION_MISS)
         buf_add(" unmapped memory at");
      else if (ctx->dsisr & DSISR_TRANSLATION_PROT)
         buf_add(" protected memory at");
      else if (ctx->dsisr & DSISR_BAD_CACHING)
         buf_add(" uncached memory at");

      buf_add(" %08" PRIX32 "\n", ctx->dar);
   }
   else if (type == OS_EXCEPTION_TYPE_ISI)
   {
      buf_add("ISI: Bad execute of");
      if (ctx->srr1 & SRR1_ISI_TRANSLATION_PROT)
         buf_add(" protected memory at");
      else if (ctx->srr1 & SRR1_ISI_TRANSLATION_MISS)
         buf_add(" unmapped memory at");

      buf_add(" %08" PRIX32 "\n", ctx->srr0);
   }
   else if (type == OS_EXCEPTION_TYPE_PROGRAM)
   {
      buf_add("PROG:");
      if (ctx->srr1 & SRR1_PROG_BAD_INSTR)
         buf_add(" Malformed instruction at");
      else if (ctx->srr1 & SRR1_PROG_PRIV_INSTR)
         buf_add(" Privileged instruction in userspace at");
      else if (ctx->srr1 & SRR1_PROG_IEEE_FLOAT)
         buf_add(" Floating-point exception at");
      else if (ctx->srr1 & SRR1_PROG_TRAP)
         buf_add(" Trap conditions met at");
      else
         buf_add(" Out-of-spec error (!) at");

      if (ctx->srr1 & SRR1_PROG_SRR0_INACCURATE)
         buf_add(" %08" PRIX32 "-ish\n", ctx->srr0);
      else
         buf_add(" %08" PRIX32 "\n", ctx->srr0);
   }

   /*	Add register dump
      There's space for two more regs at the end of the last line...
      Any ideas for what to put there? */
   buf_add( \
         "   r0  %08X r1  %08X r2  %08X r3  %08X r4  %08X\n"
         "   r5  %08X r6  %08X r7  %08X r8  %08X r9  %08X\n"
         "   r10 %08X r11 %08X r12 %08X r13 %08X r14 %08X\n"
         "   r15 %08X r16 %08X r17 %08X r18 %08X r19 %08X\n"
         "   r20 %08X r21 %08X r22 %08X r23 %08X r24 %08X\n"
         "   r25 %08X r26 %08X r27 %08X r28 %08X r29 %08X\n"
         "   r30 %08X r31 %08X lr  %08X sr1 %08X dsi %08X\n"
         "   ctr %08X cr  %08X xer %08X %25s\n",
         ctx->gpr[0],  ctx->gpr[1],  ctx->gpr[2],  ctx->gpr[3],  ctx->gpr[4],
         ctx->gpr[5],  ctx->gpr[6],  ctx->gpr[7],  ctx->gpr[8],  ctx->gpr[9],
         ctx->gpr[10], ctx->gpr[11], ctx->gpr[12], ctx->gpr[13], ctx->gpr[14],
         ctx->gpr[15], ctx->gpr[16], ctx->gpr[17], ctx->gpr[18], ctx->gpr[19],
         ctx->gpr[20], ctx->gpr[21], ctx->gpr[22], ctx->gpr[23], ctx->gpr[24],
         ctx->gpr[25], ctx->gpr[26], ctx->gpr[27], ctx->gpr[28], ctx->gpr[29],
         ctx->gpr[30], ctx->gpr[31], ctx->lr,      ctx->srr1,    ctx->dsisr,
         ctx->ctr,     ctx->cr,      ctx->xer, OSGetThreadName(OSGetCurrentThread()));

   buf_add("   ");
   DisassemblePPCRange((void*)ctx->srr0, (void*)ctx->srr0, (void*)disasm_printfcallback, (void*)OSGetSymbolNameEx, 0);

   /*	Stack trace!
      First, let's print the PC... */
   exception_print_symbol(ctx->srr0);

   int i;
   if (ctx->gpr[1])
   {
      /*	Then the addresses off the stack.
         Code borrowed from Dimok's exception handler. */
      frame_rec_t p = (frame_rec_t)ctx->gpr[1];
      if ((unsigned int)p->lr != ctx->lr)
         exception_print_symbol(ctx->lr);

      for (i = 0; i < VISIBLE_STACK_TRACE_LINES && p->up; p = p->up, i++)
         exception_print_symbol((unsigned int)p->lr);
   }
   else
      buf_add("Stack pointer invalid. Could not trace further.\n");

   buf_add("MEM: 0x%08X/0x%08X/0x%08X\n", MEM1_avail(), MEM2_avail(), MEMBucket_avail());

   if(PROGRAM_NAME && PROGRAM_VERSION)
      buf_add("%s (%s)\n", PROGRAM_NAME, PROGRAM_VERSION);
   else if (PROGRAM_NAME)
      buf_add("%s\n", PROGRAM_NAME);

   for (; i < VISIBLE_STACK_TRACE_LINES + 3; i++)
      buf_add("\n");

   /* again */
   exception_print_lineinfo(ctx->srr0);
   if (ctx->gpr[1])
   {
      frame_rec_t p = (frame_rec_t)ctx->gpr[1];
      if ((unsigned int)p->lr != ctx->lr)
         exception_print_lineinfo(ctx->lr);

      for (i = 0; i < NUM_STACK_TRACE_LINES && p->up; p = p->up, i++)
         exception_print_lineinfo((unsigned int)p->lr);
   }
   buf_add("\n\n");

   if (ctx->srr1 & SRR1_PROG_TRAP)
      ctx->srr0 += 4;
   else if (!(ctx->dsisr & DSISR_DABR_MATCH))
      ctx->srr0 = (u32)print_and_abort;

   if (!prev_cb || prev_cb(ctx))
      return TRUE;

   OSFatal(exception_msgbuf);
   return FALSE;
}
static OSExceptionCallbackFn previous_dsi_cb;
static OSExceptionCallbackFn previous_isi_cb;
static OSExceptionCallbackFn previous_prog_cb;
static OSExceptionCallbackFn previous_br_cb;

BOOL exception_dsi_cb(OSContext* ctx)
{
	return exception_cb(ctx, OS_EXCEPTION_TYPE_DSI, previous_dsi_cb);
}

BOOL exception_isi_cb(OSContext* ctx)
{
   return exception_cb(ctx, OS_EXCEPTION_TYPE_ISI, previous_isi_cb);
}

BOOL exception_prog_cb(OSContext* ctx)
{
	return exception_cb(ctx, OS_EXCEPTION_TYPE_PROGRAM, previous_prog_cb);
}

BOOL exception_br_cb(OSContext* ctx)
{
	return exception_cb(ctx, OS_EXCEPTION_TYPE_BREAKPOINT, previous_br_cb);
}

void exception_print_symbol(uint32_t addr)
{
   /*	Check if addr is within this RPX's .text */
   if (addr >= TEXT_START && addr < TEXT_END)
   {
      char symbolName[256];
      OSGetSymbolNameEx(addr, symbolName, sizeof(symbolName));

#if 0
      buf_add("%08" PRIX32 "(%08" PRIX32 "):%s\n",
            addr, addr - (TEXT_START - 0x02000000), symbolName);
#else
      buf_add("%08" PRIX32":%s\n", addr - (TEXT_START - 0x02000000), symbolName);
#endif
   }
   /*	Check if addr is within the system library area... */
   else if ((addr >= 0x01000000 && addr < 0x01800000) ||
         /*	Or the rest of the app executable area.
            I would have used whatever method JGeckoU uses to determine
            the real lowest address, but *someone* didn't make it open-source :/ */
         (addr >= 0x01800000 && addr < 0x1000000))
   {
      char *seperator = NULL;
      char symbolName[64];

      OSGetSymbolNameEx(addr, symbolName, sizeof(symbolName));

      /*	Extract RPL name and try and find its base address */
      seperator = strchr(symbolName, '|');

      if (seperator)
      {
         void* libAddr = NULL;
         /*	Isolate library name; should end with .rpl
            (our main RPX was caught by another test case above) */
         *seperator = '\0';
         /*	Try for a base address */
         OSDynLoad_Acquire(symbolName, &libAddr);

         /*	Special case for coreinit; which has broken handles */
         if (!strcmp(symbolName, "coreinit.rpl"))
         {
            void* PPCExit_addr;
            OSDynLoad_FindExport(libAddr, 0, "__PPCExit", &PPCExit_addr);
            libAddr = PPCExit_addr - 0x180;
         }

         *seperator = '|';

         /*	We got one! */
         if (libAddr)
         {
            buf_add("%08" PRIX32 "(%08" PRIX32 "):%s\n", addr, addr - (unsigned int)libAddr, symbolName);
            OSDynLoad_Release(libAddr);
            return;
         }
      }
      /*	Ah well. We can still print the basics. */
      buf_add("%08" PRIX32 "(        ):%s\n", addr, symbolName);
   }

   /*	Check if addr is in the HBL range
      TODO there's no real reason we couldn't find the symbol here,
      it's just laziness and arguably uneccesary bloat */
   else if (addr >= 0x00800000 && addr < 0x01000000)
      buf_add("%08" PRIX32 "(%08" PRIX32 "):<unknown:HBL>\n", addr, addr - 0x00800000);
   /*	If all else fails, just say "unknown" */
   else
      buf_add("%08" PRIX32 "(        ):<unknown>\n", addr);
}

/*	void setup_os_exceptions(void)
	Install and initialize the exception handler.
*/
void setup_os_exceptions(void)
{
   exception_msgbuf = malloc(4096* 8);
   *exception_msgbuf = '\0';
   previous_dsi_cb = OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OS_EXCEPTION_TYPE_DSI, exception_dsi_cb);
   previous_isi_cb = OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OS_EXCEPTION_TYPE_ISI, exception_isi_cb);
   previous_prog_cb = OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OS_EXCEPTION_TYPE_PROGRAM, exception_prog_cb);
   previous_br_cb = OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OS_EXCEPTION_TYPE_BREAKPOINT, exception_br_cb);
   test_os_exceptions();
}

void deinit_os_exceptions(void)
{
   if(*exception_msgbuf)
      print_and_abort();
   free(exception_msgbuf);
   exception_msgbuf = NULL;
   OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OS_EXCEPTION_TYPE_DSI, previous_dsi_cb);
   OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OS_EXCEPTION_TYPE_ISI, previous_isi_cb);
   OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OS_EXCEPTION_TYPE_PROGRAM, previous_prog_cb);
   OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OS_EXCEPTION_TYPE_BREAKPOINT, previous_br_cb);
}

/*	void test_os_exceptions(void)
	Used for debugging. Insert code here to induce a crash.
*/
void test_os_exceptions(void)
{
   /*Write to 0x00000000; causes DSI */
#if 0
   __asm__ volatile (
         "li %r3, 0 \n" \
         "stw %r3, 0(%r3) \n"
         );
   DCFlushRange((void*)0, 4);
#endif

   /* Trap instruction, causes PROG. */
#if 0
   __asm__ volatile (
         ".int 0x0FE00016"
         );
#endif

   /* Jump to 0; causes ISI */
#if 0
   void (*testFunc)() = (void(*)())0;
   testFunc();
#endif
}
