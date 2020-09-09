#pragma once

#ifdef __cplusplus
extern "C" {
#endif

static inline void *dlopen(const char *__file, int __mode) { return NULL;}
static inline int dlclose(void *__handle) { return 0;}
static inline void *dlsym(void *__handle, const char *__name) { return NULL;}
extern char *dlerror(void) { return NULL; }
extern int dlinfo(void *__handle, int __request, void *__arg) { return 0; }
#define RTLD_LAZY 0x00001
#define RTLD_NOW 0x00002
#define RTLD_BINDING_MASK 0x3
#define RTLD_NOLOAD 0x00004
#define RTLD_DEEPBIND 0x00008
#define RTLD_GLOBAL 0x00100
#define RTLD_LOCAL 0
#define RTLD_NODELETE 0x01000

#ifdef __cplusplus
}
#endif
