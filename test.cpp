
#include <cstdio>
#include <thread>
#include <wiiu/os/thread.h>
#include <wiiu/os/debug.h>
#include <stdexcept>

 const char *PROGRAM_NAME = "Test";
 const char *PROGRAM_VERSION = "0.0";

#define __thread __thread __attribute((tls_model("global-dynamic")))

__thread const char* thread_name = "";

int thread1_entry(void) {
   thread_name = "Thread1";
   printf("Hello from %-20s (thread_name@0x%08X)\n", thread_name, &thread_name);
   return 0;
}
int main(int argc, const char **argv) {
   thread_name = "Main Thread";
   std::thread th1 = std::thread(thread1_entry);
   th1.join();
   printf("Hello from %-20s (thread_name@0x%08X)\n", thread_name, &thread_name);
   return 0;
}
