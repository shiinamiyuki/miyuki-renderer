#ifndef MIYUKI_KERNEL_LIGHT_H
#define MIYUKI_KERNEL_LIGHT_H

#include "kerneldef.h"

#include "generated.light.h"

MYK_KERNEL_NS_BEGIN

typedef struct LightSamplingRecord {
	float2 u;
	float3 p; 
	float3 Le;
	float3 wi; 
	float pdf
}LightSamplingRecord;


MYK_KERNEL_NS_END

#endif 