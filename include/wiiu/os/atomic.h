#pragma once
#include <wiiu/types.h>

#ifdef __cplusplus
extern "C" {
#endif

u64 OSGetAtomic64(u64 *ptr);
u64 OSSetAtomic64(u64 *ptr, u64 value);
BOOL OSCompareAndSwapAtomic64(u64 *ptr, u64 compare, u64 value);
BOOL OSCompareAndSwapAtomicEx64(u64 *ptr, u64 compare, u64 value, u64 *old);
u64 OSSwapAtomic64(u64 *ptr, u64 value);
s64 OSAddAtomic64(s64 *ptr, s64 value);
u64 OSAndAtomic64(u64 *ptr, u64 value);
u64 OSOrAtomic64(u64 *ptr, u64 value);
u64 OSXorAtomic64(u64 *ptr, u64 value);
BOOL OSTestAndClearAtomic64(u64 *ptr, u32 bit);
BOOL OSTestAndSetAtomic64(u64 *ptr, u32 bit);

BOOL OSCompareAndSwapAtomic(u32 *ptr, u32 compare, u32 value);
BOOL OSCompareAndSwapAtomicEx(u32 *ptr, u32 compare, u32 value, u32 *old);
u32 OSSwapAtomic(u32 *ptr, u32 value);
s32 OSAddAtomic(s32 *ptr, s32 value);
u32 OSAndAtomic(u32 *ptr, u32 value);
u32 OSOrAtomic(u32 *ptr, u32 value);
u32 OSXorAtomic(u32 *ptr, u32 value);
BOOL OSTestAndClearAtomic(u32 *ptr, u32 bit);
BOOL OSTestAndSetAtomic(u32 *ptr, u32 bit);

#ifdef __cplusplus
}
#endif
