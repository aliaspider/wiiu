
#include <errno.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <pthread.h>

#include <wiiu/os/debug.h>
#include <wiiu/os/thread.h>
#include <wiiu/os/mutex.h>
#include <wiiu/os/condition.h>
#include <wiiu/os/atomic.h>

pthread_t pthread_self (void)
{
   return (pthread_t)OSGetCurrentThread();
}

int pthread_setname_np (pthread_t target_thread, const char *name) {
   OSSetThreadName((OSThread*)target_thread, name);
}

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) {
   OSInitMutex((OSMutex *)(*mutex = (pthread_mutex_t)calloc(1, sizeof(OSMutex))));
   return 0;
}

static void check_mutex(pthread_mutex_t *mutex) {
   if (*mutex == _PTHREAD_MUTEX_INITIALIZER)
      pthread_mutex_init(mutex, NULL);
}

int pthread_mutex_destroy(pthread_mutex_t *mutex) {
   if (*mutex != _PTHREAD_MUTEX_INITIALIZER)
      free((OSMutex *)*mutex);
   *mutex = 0;
   return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex) {
   check_mutex(mutex);
   OSLockMutex((OSMutex *)*mutex);
   return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex) {
   check_mutex(mutex);
   OSUnlockMutex((OSMutex *)*mutex);
   return 0;
}
int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr) {
   OSInitCond((OSCondition *)(*cond = (pthread_cond_t)malloc(sizeof(OSCondition))));
   return 0;
}
int pthread_cond_destroy(pthread_cond_t *cond) {
   free((void *)*cond);
   *cond = 0;
   return 0;
}

int pthread_cond_signal(pthread_cond_t *cond) {
   // OSSignalCond is actually a broadcast,
   // this is fine tough because pthread allows that.
   OSSignalCond((OSCondition *)*cond);
   return 0;
}
int pthread_cond_broadcast(pthread_cond_t *cond) {
   OSSignalCond((OSCondition *)*cond);
   return 0;
}
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
   OSWaitCond((OSCondition *)*cond, (OSMutex *)*mutex);
   return 0;
}

int pthread_once(pthread_once_t *once_control, void (*init_routine)(void)) {
   if (!OSTestAndSetAtomic((u32*)&once_control->is_initialized, 1)) {
      init_routine();
      once_control->init_executed = 1;
   }
   while (!once_control->init_executed)
      OSYieldThread();
   return 0;
}

int pthread_join(pthread_t pthread, void **value_ptr) {
   return !OSJoinThread((OSThread *)pthread, (int *)value_ptr);
}

static void thread_deallocator(OSThread *thread, void *stack) {
   free(stack);
   free(thread);
}

int pthread_create(pthread_t *pthread, const pthread_attr_t *pthread_attr, void *(*start_routine)(void *), void *arg) {
   OSThread *thread = calloc(1, sizeof(OSThread));

   int stack_size = 4 * 1024 * 1024;
   if (pthread_attr && pthread_attr->stacksize)
      stack_size = pthread_attr->stacksize;

   void *stack_addr = NULL;
   if (pthread_attr && pthread_attr->stackaddr)
      stack_addr = (u8 *)pthread_attr->stackaddr + pthread_attr->stacksize;
   else
      stack_addr = (u8 *)memalign(8, stack_size) + stack_size;

   OSThreadAttributes attr = OS_THREAD_ATTRIB_AFFINITY_ANY;
      attr = OS_THREAD_ATTRIB_AFFINITY_CPU0 | OS_THREAD_ATTRIB_AFFINITY_CPU2;
   if (pthread_attr && pthread_attr->detachstate)
      attr |= OS_THREAD_ATTRIB_DETACHED;

   if (!OSCreateThread(thread, (OSThreadEntryPointFn)start_routine, (int)arg, NULL, stack_addr, stack_size, OS_THREAD_DEF_PRIO, attr))
      return EINVAL;

   OSSetThreadDeallocator(thread, thread_deallocator);
   *pthread = (pthread_t)thread;
   OSResumeThread(thread);

   return 0;
}
#if 0 // unused
//#define _POSIX_TIMEOUTS 1
//#define _POSIX_SPIN_LOCKS
//#define _UNIX98_THREAD_MUTEX_ATTRIBUTES 1
int pthread_mutex_trylock(pthread_mutex_t *mutex) {
   check_mutex(mutex);
   OSTryLockMutex((OSMutex *)*mutex);
   return 0;
}

int pthread_mutexattr_init(pthread_mutexattr_t *attr) {
   attr->is_initialized = true;
   attr->recursive = true;
#ifdef _UNIX98_THREAD_MUTEX_ATTRIBUTES
   attr->type = PTHREAD_MUTEX_NORMAL;
#endif
   return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *attr) {
   memset(attr, 0x00, sizeof(attr));
   return 0;
}

int pthread_mutexattr_getpshared(_CONST pthread_mutexattr_t *attr, int *pshared);
int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared);

int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, _CONST struct timespec *abstime) {
   OSWaitCond((OSCondition*)*cond, (OSMutex*)*mutex);
   return 0;
}

int pthread_detach(pthread_t pthread) {
   OSDetachThread((OSThread *)pthread);
   return 0;
}
#ifdef _UNIX98_THREAD_MUTEX_ATTRIBUTES
int pthread_mutexattr_gettype(_CONST pthread_mutexattr_t *attr, int *kind) {
   *kind = attr->type;
   return 0;
}
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int kind) {
   attr->type = kind;
   return 0;
}
#endif
#endif
