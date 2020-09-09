
#include <stdlib.h>
#include <string.h>
#include <wiiu/os/debug.h>
#include <cxxabi.h>

void* OSGetSymbolNameEx(u32 addr, char* out, int out_size)
{
   void* ret = OSGetSymbolName(addr, out, out_size);

   char* mangled = out;
   while(*mangled && *mangled != '|')
      mangled++;

   if(*mangled++ == '|')
   {
      int status = 0;
      char* demangled = abi::__cxa_demangle(mangled, NULL, 0, &status);
      if(demangled && status == 0)
      {
         strncpy(out, demangled, out_size);
         free(demangled);
      }
      else
         strncpy(out, mangled, out_size);
   }

   return ret;
}
