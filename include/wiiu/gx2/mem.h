#pragma once
#include <wiiu/types.h>
#include "enum.h"
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

void GX2Invalidate(GX2InvalidateMode mode, void *buffer, uint32_t size);

#ifdef __cplusplus
}
#endif

#ifndef GX2_DISABLE_WRAPS
#include "validation_layer.h"
#define GX2Invalidate(...) GX2_WRAP(GX2Invalidate, __VA_ARGS__)
#endif
