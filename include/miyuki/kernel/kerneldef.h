#ifndef MIYUKI_KERNELDEF_H
#define MIYUKI_KERNELDEF_H

#include <math/vec4.hpp>
#include <math/func.h>
#include <cmath>
#include <algorithm>

#ifdef _MSC_VER 

#define MYK_KERNEL_NS_BEGIN namespace Miyuki{namespace Kernel{ __pragma(pack(push, 1))
#define MYK_KERNEL_NS_END }}  __pragma(pack(pop))

#define KNL_PACKED 

#endif

#define MYK_KERNEL_FUNC  static
#define MYK_KERNEL_FUNC_INLINE  static inline


MYK_KERNEL_NS_BEGIN

typedef _Vec3f float3;
typedef _Vec4f float4;
typedef _Vec<Float, 2> float2;
typedef _Vec<int, 2> int2;

MYK_KERNEL_FUNC_INLINE
float2 make_float2(Float x, Float y) {
	return float2(x, y);
}

MYK_KERNEL_FUNC_INLINE
float3 make_float3(Float x, Float y, Float z) {
	return float3(x, y, z);
}

MYK_KERNEL_FUNC_INLINE
float4 make_float4(Float x, Float y, Float z, Float w) {
	return float4(x, y, z, w);
}

MYK_KERNEL_FUNC_INLINE
Float dot(const float3& v1, const float3& v2) {
	return Vec3f::dot(v1, v2);
}
MYK_KERNEL_FUNC_INLINE
Float dot(const float4& v1, const float4& v2) {
	return Vec3f::dot(v1, v2);
}

MYK_KERNEL_FUNC_INLINE
float3 cross(const float3& v1, const float3& v2) {
	return Vec3f::cross(v1, v2);
}

MYK_KERNEL_FUNC_INLINE
float3 normalize(const float3& v) {
	return v.normalized(); 
}

inline float max(float a, float b) {
	return b > a ? b : a;
}
inline float min(float a, float b) {
	return a < b ? a : b;
}
inline float2 max(float2 a, float2 b) {
	return make_float2(max(a.x, b.x), max(a.y, b.y));
}
inline float2 min(float2 a, float2 b) {
	return make_float2(min(a.x, b.x), min(a.y, b.y));
}
inline float3 max(float3 a, float3 b) {
	return make_float3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));
}
inline float3 min(float3 a, float3 b) {
	return make_float3(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z));
}
inline float4 max(float4 a, float4 b) {
	return make_float4(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z), max(a.w, b.w));
}
inline float4 min(float4 a, float4 b) {
	return make_float4(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z), min(a.w, b.w));
}

#define MYK_KERNEL_PANIC(fmt, ...) printf("MYK_KERNEL_PANIC() called at %s:%d: " fmt, __FILE__,__LINE__, __VA_ARGS__ )



typedef float3 ShadingResult;
struct Shader;
typedef struct ShaderProgram {
	struct Shader** program;
	int length;
}ShaderProgram;



MYK_KERNEL_NS_END
#endif