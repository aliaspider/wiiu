#pragma once
#include <wiiu/types.h>
#include <wiiu/gx2r/resource.h>
#include "enum.h"
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct GX2Surface
{
   GX2SurfaceDim dim;
   uint32_t width;
   uint32_t height;
   uint32_t depth;
   uint32_t mipLevels;
   GX2SurfaceFormat format;
   GX2AAMode aa;
   union{
      GX2SurfaceUse use;
      int flags;
   };
   uint32_t imageSize;
   void *image;
   uint32_t mipmapSize;
   void *mipmaps;
   GX2TileMode tileMode;
   uint32_t swizzle;
   uint32_t alignment;
   uint32_t pitch;
   uint32_t mipLevelOffset[13];
} GX2Surface;

typedef struct GX2DepthBuffer
{
   GX2Surface surface;
   uint32_t viewMip;
   uint32_t viewFirstSlice;
   uint32_t viewNumSlices;
   void *hiZPtr;
   uint32_t hiZSize;
   float depthClear;
   uint32_t stencilClear;
   uint32_t regs[7];
} GX2DepthBuffer;

typedef struct GX2ColorBuffer
{
   GX2Surface surface;
   uint32_t viewMip;
   uint32_t viewFirstSlice;
   uint32_t viewNumSlices;
   void *aaBuffer;
   uint32_t aaSize;
   uint32_t regs[5];
} GX2ColorBuffer;

typedef struct GX2Rect
{
   int32_t left;
   int32_t top;
   int32_t right;
   int32_t bottom;
} GX2Rect;

typedef struct GX2Point
{
   int32_t x;
   int32_t y;
} GX2Point;

void GX2CalcSurfaceSizeAndAlignment(GX2Surface *surface);
void GX2CalcDepthBufferHiZInfo(GX2DepthBuffer *depthBuffer, uint32_t *outSize, uint32_t *outAlignment);
void GX2CalcColorBufferAuxInfo(GX2ColorBuffer *surface, uint32_t *outSize, uint32_t *outAlignment);
void GX2SetColorBuffer(GX2ColorBuffer *colorBuffer, GX2RenderTarget target);
void GX2SetDepthBuffer(GX2DepthBuffer *depthBuffer);
void GX2InitColorBufferRegs(GX2ColorBuffer *colorBuffer);
void GX2InitDepthBufferRegs(GX2DepthBuffer *depthBuffer);
void GX2InitDepthBufferHiZEnable(GX2DepthBuffer *depthBuffer, BOOL enable);

uint32_t GX2GetSurfaceSwizzle(GX2Surface *surface);
void GX2SetSurfaceSwizzle(GX2Surface *surface, uint32_t swizzle);
void GX2CopySurface(GX2Surface *src, uint32_t srcLevel, uint32_t srcDepth, GX2Surface *dst, uint32_t dstLevel,
                    uint32_t dstDepth);
void GX2CopySurfaceEx(GX2Surface *src, uint32_t srcLevel, uint32_t srcDepth, GX2Surface *dst, uint32_t dstLevel,
                      uint32_t dstDepth, uint32_t regions, GX2Rect *srcRegion, GX2Point *dstCoords);
void GX2ClearColor(GX2ColorBuffer *colorBuffer, float red, float green, float blue, float alpha);
void GX2ClearDepthStencilEx(GX2DepthBuffer *depthBuffer, float depth, uint8_t stencil, GX2ClearFlags clearMode);
void GX2ClearBuffersEx(GX2ColorBuffer *colorBuffer, GX2DepthBuffer *depthBuffer,
                       float red, float green, float blue, float alpha, float depth,
                       uint8_t stencil, GX2ClearFlags clearMode);
void GX2SetClearDepth(GX2DepthBuffer *depthBuffer, float depth);
void GX2SetClearStencil(GX2DepthBuffer *depthBuffer, uint8_t stencil);
void GX2SetClearDepthStencil(GX2DepthBuffer *depthBuffer, float depth, uint8_t stencil);

void GX2AllocateTilingApertureEx(const GX2Surface *surface, u32 level, u32 depth, GX2EndianSwapMode mode,
                                 u32 *handle, void **ptr);
void GX2FreeTilingAperture(u32 handle);

#ifdef __cplusplus
}
#endif


#ifndef GX2_DISABLE_WRAPS
#include "validation_layer.h"

#define GX2SetColorBuffer(...) GX2_WRAP(GX2SetColorBuffer, __VA_ARGS__)
#define GX2SetDepthBuffer(...) GX2_WRAP(GX2SetDepthBuffer, __VA_ARGS__)

#endif
