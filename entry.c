
#include <stdlib.h>
#include <stdio.h>
#include <fat.h>
#include <iosuhax.h>
#include <wiiu/ios.h>
#include <wiiu/os/thread.h>
#include <wiiu/os/memory.h>
#include <wiiu/os/debug.h>
#include <wiiu/os/systeminfo.h>
#include <wiiu/sysapp.h>
#include <sys/socket.h>

#include "fs_utils.h"
#include "sd_fat_devoptab.h"
#include "exception_handler.h"

void wiiu_log_init(void);
void wiiu_log_deinit(void);
int main(int argc, char **argv);

void __eabi(void) {}

extern void (*const __CTOR_LIST__[])(void);
extern void (*const __CTOR_END__[])(void);
extern void (*const __DTOR_LIST__[])(void);
extern void (*const __DTOR_END__[])(void);

void __init(void) {
	void (*const *ctor)(void) = __CTOR_LIST__;
	while (ctor < __CTOR_END__)
		(*ctor++)();
}

void __fini(void) {
	void (*const *dtor)(void) = __DTOR_LIST__;
	while (dtor < __DTOR_END__)
		(*dtor++)();
}

/* libiosuhax related */

// just to be able to call async
void someFunc(void *arg) { (void)arg; }

static int mcp_hook_fd = -1;

int MCPHookOpen(void) {
	// take over mcp thread
	mcp_hook_fd = IOS_Open("/dev/mcp", 0);

	if (mcp_hook_fd < 0)
		return -1;

	IOS_IoctlAsync(mcp_hook_fd, 0x62, (void *)0, 0, (void *)0, 0, (void *)someFunc, (void *)0);
	// let wupserver start up
	OSSleepTicks(ms_to_ticks(1));

	if (IOSUHAX_Open("/dev/mcp") < 0) {
		IOS_Close(mcp_hook_fd);
		mcp_hook_fd = -1;
		return -1;
	}

	return 0;
}

void MCPHookClose(void) {
	if (mcp_hook_fd < 0)
		return;

	// close down wupserver, return control to mcp
	IOSUHAX_Close();
	// wait for mcp to return
	OSSleepTicks(ms_to_ticks(1));
	IOS_Close(mcp_hook_fd);
	mcp_hook_fd = -1;
}

static int iosuhaxMount = 0;

static void fsdev_init(void) {

	iosuhaxMount = 0;
	if (!OSIsHLE()) {
		int res = IOSUHAX_Open(NULL);

		if (res < 0)
			res = MCPHookOpen();

		if (res >= 0) {
			iosuhaxMount = 1;
			fatInitDefault();
			return;
		}
	}
	mount_sd_fat("sd");
}
static void fsdev_exit(void) {
	if (iosuhaxMount) {
		fatUnmount("sd:");
		fatUnmount("usb:");

		if (mcp_hook_fd >= 0)
			MCPHookClose();
		else
			IOSUHAX_Close();
		return;
	}
	unmount_sd_fat("sd");
}

__attribute__((noreturn)) void __shutdown_program(void) {
	fsdev_exit();
	memoryRelease();
	wiiu_log_deinit();
	SYSRelaunchTitle(0, 0);
	exit(0);
}

__attribute__((noreturn))
void __rpx_start(int argc, char **argv) {
	setup_os_exceptions();
	socket_lib_init();
	wiiu_log_init();
	DEBUG_LINE();
	memoryInitialize();
	fsdev_init();

	DEBUG_VAR(iosuhaxMount);
	DEBUG_VAR(mcp_hook_fd);
	DEBUG_VAR2(MEM1_avail());
	DEBUG_VAR2(MEM2_avail());
	DEBUG_VAR2(MEMBucket_avail());
	__init();
	main(argc, argv);
	__fini();

	deinit_os_exceptions();
	__shutdown_program();
}

__attribute__((noreturn)) void abort(void) {
	printf("Abort called\n");
	DEBUG_VAR(MEM2_avail());
	fflush(stdout);
	*(u32*)0 = 0;
	__shutdown_program();
}
