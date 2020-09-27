
#include <bits/gthr.h>
#include <errno.h>
#include <string.h>
#include <wiiu/mem.h>
#include <wiiu/os/atomic.h>
#include <wiiu/os/condition.h>
#include <wiiu/os/debug.h>
#include <wiiu/os/mutex.h>
#include <wiiu/os/thread.h>

#define __GTHREADS_STACK_SIZE (4 * 1024 * 1024)

static_assert(sizeof(OSThread *) <= sizeof(__gthread_t), "__gthread_t overflow");
static_assert(sizeof(OSCondition) <= sizeof(__gthread_cond_t), "__gthread_cond_t overflow");
static_assert(sizeof(void *) <= sizeof(__gthread_key_t), "__gthread_cond_t overflow");
static_assert(sizeof(OSMutex) <= sizeof(__gthread_mutex_t), "__gthread_cond_t overflow");
static_assert(sizeof(u32) <= sizeof(__gthread_once_t), "__gthread_once_t overflow");
static_assert(sizeof(OSMutex) <= sizeof(__gthread_recursive_mutex_t), "__gthread_cond_t overflow");

void __gthread_mutex_init_function(__gthread_mutex_t *mutex) {
	memset(mutex, 0, sizeof(mutex));
	OSInitMutex((OSMutex *)mutex);
}

int __gthread_recursive_mutex_init_function(__gthread_recursive_mutex_t *mutex) {
	__gthread_mutex_init_function((__gthread_mutex_t *)mutex);
	return 0;
}

int __gthread_once(__gthread_once_t *once, void (*func)()) {
	uint32_t value = 0;

	if (OSCompareAndSwapAtomicEx((u32 *)once, 0, 1, &value)) {
		func();
		OSCompareAndSwapAtomic((u32 *)once, 1, 2);
	}

	while (*(u32 *)once != 2)
		OSYieldThread();

	return 0;
}

static struct {
	bool active;
	void (*dtor)(void *);
} thread_keys[OS_THREAD_SPECIFIC_MAX];

int __gthread_key_create(__gthread_key_t *keyp, void (*dtor)(void *)) {
	int key = 0;
	while (key < OS_THREAD_SPECIFIC_MAX && thread_keys[key].active)
		key++;
	if (key < OS_THREAD_SPECIFIC_MAX) {
		thread_keys[key].active = true;
		thread_keys[key].dtor = dtor;
		*(int*)keyp = key;
		return 0;
	}

	return -1;
}

int __gthread_key_delete(__gthread_key_t key) {
	thread_keys[*(int*)&key].active = false;
	thread_keys[*(int*)&key].dtor = NULL;
	return 0;
}

void *__gthread_getspecific(__gthread_key_t key) {
	if (*(int*)&key < 0)
		return NULL;

	return OSGetThreadSpecific(*(int*)&key);
}

int __gthread_setspecific(__gthread_key_t key, const void *ptr) {
	if (*(int*)&key < 0 || *(int*)&key >= OS_THREAD_SPECIFIC_MAX)
		return -1;
	OSSetThreadSpecific(*(int*)&key, ptr);
	return 0;
}

int __gthread_mutex_destroy(__gthread_mutex_t *mutex) {
	memset(mutex, 0, sizeof(*mutex));
	return 0;
}
int __gthread_recursive_mutex_destroy(__gthread_recursive_mutex_t *mutex) {
	memset(mutex, 0, sizeof(*mutex));
	return 0;
}

int __gthread_mutex_lock(__gthread_mutex_t *mutex) {
	OSLockMutex((OSMutex *)mutex);
	return 0;
}
int __gthread_mutex_trylock(__gthread_mutex_t *mutex) {
	if (!OSTryLockMutex((OSMutex *)mutex))
		return -1;
	return 0;
}
int __gthread_mutex_unlock(__gthread_mutex_t *mutex) {
	OSUnlockMutex((OSMutex *)mutex);
	return 0;
}

int __gthread_recursive_mutex_lock(__gthread_recursive_mutex_t *mutex) {
	OSLockMutex((OSMutex *)mutex);
	return 0;
}
int __gthread_recursive_mutex_trylock(__gthread_recursive_mutex_t *mutex) {
	if (!OSTryLockMutex((OSMutex *)mutex))
		return -1;
	return 0;
}

int __gthread_recursive_mutex_unlock(__gthread_recursive_mutex_t *mutex) {
	OSUnlockMutex((OSMutex *)mutex);
	return 0;
}

void __gthread_cond_init_function(__gthread_cond_t *cond) {
	OSInitCond((OSCondition *)cond);
}

int __gthread_cond_destroy(__gthread_cond_t *cond) {
	memset(cond, 0, sizeof(*cond));
	return 0;
}
int __gthread_cond_broadcast(__gthread_cond_t *cond) {
	OSSignalCond((OSCondition *)cond);
	return 0;
}
int __gthread_cond_wait(__gthread_cond_t *cond, __gthread_mutex_t *mutex) {
	OSWaitCond((OSCondition *)cond, (OSMutex *)mutex);
	return 0;
}
int __gthread_cond_wait_recursive(__gthread_cond_t *cond, __gthread_recursive_mutex_t *mutex) {
	OSWaitCond((OSCondition *)cond, (OSMutex *)mutex);
	return 0;
}

static int OSThreadEntryPointFnWrap(int argc, const char **argv) {
	void *(*func)(void *) = (void *(*)(void *))(argv[0]);
	void *args = (void *)argv[1];
	int ret = (int)func(args);
	int key;

	for (key = 0; key < OS_THREAD_SPECIFIC_MAX; key++) {
		if (thread_keys[key].active && thread_keys[key].dtor) {
			void *specific = OSGetThreadSpecific(key);
			if (specific) {
				OSSetThreadSpecific(key, NULL);
				thread_keys[key].dtor(specific);
			}
		}
	}
	for (key = 0; key < OS_THREAD_SPECIFIC_MAX; key++) {
		if (thread_keys[key].active && thread_keys[key].dtor) {
			void *specific = OSGetThreadSpecific(key);
			if (specific) {
				DEBUG_LINE();
			}
		}
	}

	return ret;
}

static void thread_deallocator(OSThread *thread, void *stack) {
	MEMFreeToExpHeap((MEMExpandedHeap *)MEMGetBaseHeapHandle(MEM_BASE_HEAP_MEM2), thread);
}

int __gthread_create(__gthread_t *_thread, void *(*func)(void *), void *args) {
	OSThreadAttributes attr = OS_THREAD_ATTRIB_AFFINITY_ANY;
	OSThread *thread = (OSThread *)MEMAllocFromExpHeapEx((MEMExpandedHeap *)MEMGetBaseHeapHandle(MEM_BASE_HEAP_MEM2),
																		  sizeof(OSThread) + __GTHREADS_STACK_SIZE + 16, 8);
	memset(thread, 0, sizeof(OSThread));
	u8 *stack = (u8 *)thread + sizeof(OSThread);
	void **argv = (void **)(stack + __GTHREADS_STACK_SIZE);
	argv[0] = (void *)func;
	argv[1] = args;
	argv[2] = NULL;

	if (!OSCreateThread(thread, OSThreadEntryPointFnWrap, 2, (char **)argv, stack + __GTHREADS_STACK_SIZE,
							  __GTHREADS_STACK_SIZE, OS_THREAD_DEF_PRIO, attr)) {
		MEMFreeToExpHeap((MEMExpandedHeap *)MEMGetBaseHeapHandle(MEM_BASE_HEAP_MEM2), thread);
		return EINVAL;
	}

	OSSetThreadDeallocator(thread, thread_deallocator);
//	OSSetThreadRunQuantum(thread, 100);

	*_thread = thread;
	OSResumeThread(thread);

	return 0;
}

int __gthread_join(__gthread_t thread, void **value_ptr) {
	if (!OSJoinThread((OSThread *)thread, (int *)value_ptr))
		return EINVAL;
	return 0;
}

int __gthread_detach(__gthread_t thread) {
	OSDetachThread((OSThread *)thread);
	return 0;
}

int __gthread_equal(__gthread_t t1, __gthread_t t2) {
	return ((OSThread *)t1)->id == ((OSThread *)t2)->id;
}

__gthread_t __gthread_self(void) {
	return OSGetCurrentThread();
}

int __gthread_yield(void) {
	OSYieldThread();
	return 0;
}

int __gthread_mutex_timedlock(__gthread_mutex_t *m, const __gthread_time_t *abs_timeout) {
	OSLockMutex((OSMutex *)m);
	return 0;
}

int __gthread_recursive_mutex_timedlock(__gthread_recursive_mutex_t *m, const __gthread_time_t *abs_time) {
	OSLockMutex((OSMutex *)m);
	return 0;
}

int __gthread_cond_signal(__gthread_cond_t *cond) {
	OSSignalCond((OSCondition *)cond);
	return 0;
}

#if 1
int __gthread_cond_timedwait(__gthread_cond_t *cond, __gthread_mutex_t *mutex, const __gthread_time_t *abs_timeout) {
	OSWaitCond((OSCondition *)cond, (OSMutex *)mutex);
	return 0;
}
#else
struct __wut_cond_timedwait_data_t
{
   OSCondition *cond;
   bool timed_out;
};

#include <chrono>
#include <wiiu/os/alarm.h>
static void
__wut_cond_timedwait_alarm_callback(OSAlarm *alarm,
                                    OSContext *context)
{
   __wut_cond_timedwait_data_t *data = (__wut_cond_timedwait_data_t *)OSGetAlarmUserData(alarm);
   data->timed_out = true;
   OSSignalCond(data->cond);
}
int
__gthread_cond_timedwait(OSCondition *cond, OSMutex *mutex,
                     const __gthread_time_t *abs_timeout)
{
	DEBUG_LINE();
   __wut_cond_timedwait_data_t data;
   data.timed_out = false;
   data.cond = cond;

   auto time = std::chrono::system_clock::now();
   auto timeout = std::chrono::system_clock::time_point(
      std::chrono::seconds(abs_timeout->tv_sec) +
      std::chrono::nanoseconds(abs_timeout->tv_nsec));

   // Already timed out!
   if (timeout <= time) {
      return ETIMEDOUT;
   }

   auto duration =
      std::chrono::duration_cast<std::chrono::nanoseconds>(timeout - time);

   // Set an alarm
   OSAlarm alarm;
   OSCreateAlarm(&alarm);
   OSSetAlarmUserData(&alarm, &data);
   OSSetAlarm(&alarm, OSNanoseconds(duration.count()),
              &__wut_cond_timedwait_alarm_callback);

   // Wait on the condition
   OSWaitCond(cond, mutex);

   OSCancelAlarm(&alarm);
   return data.timed_out ? ETIMEDOUT : 0;
}
#endif

int __gthread_active_p(void) {
	return 1;
}
