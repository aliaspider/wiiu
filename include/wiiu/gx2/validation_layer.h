#pragma once

#include <wiiu/gx2/enum.h>
#include <wiiu/gx2/surface.h>
#include <wiiu/gx2/registers.h>
#ifdef __cplusplus
extern "C" {
#endif

void GX2Invalidate_wrap(GX2InvalidateMode mode, void *buffer, u32 size, const char *function, int line, const char *file);
void GX2SetAttribBuffer_wrap(u32 index, u32 size, u32 stride, const void *buffer, const char *function, int line, const char *file);
void GX2DrawEx_wrap(GX2PrimitiveMode mode, u32 count, u32 offset, u32 numInstances, const char *function, int line, const char *file);
void GX2DrawIndexedEx_wrap(GX2PrimitiveMode mode, u32 count, GX2IndexType indexType, void *indices, u32 offset, u32 numInstances, const char *function, int line, const char *file);

void GX2SetColorBuffer_wrap(GX2ColorBuffer *colorBuffer, GX2RenderTarget target, const char *function, int line, const char *file);
void GX2SetDepthBuffer_wrap(GX2DepthBuffer *depthBuffer, const char *function, int line, const char *file);


void GX2SetViewport_wrap(float x, float y, float width, float height, float nearZ, float farZ, const char *function, int line, const char *file);
void GX2SetViewportReg_wrap(GX2ViewportReg *reg, const char *function, int line, const char *file);
void GX2SetScissor_wrap(u32 x, u32 y, u32 width, u32 height, const char *function, int line, const char *file);
void GX2SetScissorReg_wrap(GX2ScissorReg *reg, const char *function, int line, const char *file);

#ifdef __cplusplus
}
#endif

#define GX2_WRAP(fn, ...) fn##_wrap(__VA_ARGS__, __FUNCTION__, __LINE__, __FILE__)
