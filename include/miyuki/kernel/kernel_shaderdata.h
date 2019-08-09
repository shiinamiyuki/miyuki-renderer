#ifndef MIYUKI_KERNEL_SHADERDATA_H
#define MIYUKI_KERNEL_SHADERDATA_H

#include "kerneldef.h"

MYK_KERNEL_NS_BEGIN

typedef struct ShaderData {
	int offset;
}ShaderData;

typedef struct ShadingPoint {
	float2 uv;
}ShadingPoint;

MYK_KERNEL_NS_END

#endif