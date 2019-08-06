#ifndef MIYUKI_KERNELDEF_H
#define MIYUKI_KERNELDEF_H

#include <math/vec4.hpp>
#define MYK_KERNEL_NS_BEGIN namespace Miyuki{namespace Kernel{
#define MYK_KERNEL_NS_END }}
#define MYK_KERNEL_FUNC  static
#define MYK_KERNEL_FUNC_INLINE  inline
MYK_KERNEL_NS_BEGIN

typedef Vec3f float3;
typedef Vec4f float4;
typedef Point2f float2;

float3 make_float3(Float x, Float y, Float z) {
	return Vec3f(x, y, z);
}
float4 make_float4(Float x, Float y, Float z, Float w) {
	return Vec4f(x, y, z, w);
}

typedef struct KernelGlobals {

}KernelGlobals;



MYK_KERNEL_NS_END
#endif