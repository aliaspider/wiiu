#pragma once

#include "shaders.h"

#ifdef __cplusplus
extern "C" {
#endif

int GX2VertexShaderInfo(const GX2VertexShader *vs, char *buffer);
int GX2PixelShaderInfo(const GX2PixelShader *ps, char *buffer);


#ifdef __cplusplus
}
#endif
