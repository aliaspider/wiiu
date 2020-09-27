
#include <cstdio>
#include <thread>
#include <wiiu/os/thread.h>
#include <wiiu/os/debug.h>
#include <stdexcept>

 const char *PROGRAM_NAME = "Test";
 const char *PROGRAM_VERSION = "0.0";

static int count = 5;
class test {
public:
   test() { val = count++; }
   ~test() {}
   int val;
};

test testval1;
test testval2;

#define __thread
//#define __thread __thread __attribute((tls_model("global-dynamic")))
//#define __thread __thread __attribute((tls_model("local-dynamic")))

static __thread int val = 10;
static __thread int val2;
// static __thread int kk;

void exception_test(void) { throw std::out_of_range("Out of range"); }

int thread_entry(void) {
   DEBUG_VAR(OSGetThreadSpecific(OS_THREAD_SPECIFIC_MAX - 1));
   OSSetThreadName(OSGetCurrentThread(), "test thread");
   val = 2;
   //   kk++;
   //   val2 = 44;
   printf("Hello from thread! val = %i\n", val);
   //   *(int*)0 = 0;
   return 0;
}

int main(int argc, const char **argv) {
   val2 = val;
   OSSetThreadSpecific((OS_THREAD_SPECIFIC_MAX - 1), (void *)1);
   DEBUG_VAR(OSGetThreadSpecific(OS_THREAD_SPECIFIC_MAX - 1));
   OSSetThreadName(OSGetCurrentThread(), "main thread");
   printf("Hello from Main thread! val = %i\n", val);
   DEBUG_VAR(testval1.val);
   DEBUG_VAR(testval2.val);
   std::thread th1 = std::thread(thread_entry);
   //   *(int*)0 = 0;
   th1.join();
   printf("Hello from Main thread! val = %i\n", val);
//   exception_test();
//   try {
//      exception_test();
//   } catch (std::out_of_range const &err) {
//      DEBUG_STR(err.what());
//   }
   return 0;
}
