#ifndef RADIANCE_H
#define RADIANCE_H


#define inf (1e10f)
#define pi (3.1415926f)
#define eps (0.01f)

#ifdef GPU_KERNEL
#define ALIGN_BEGIN
#define ALIGN_END __attribute__ ((packed))// !!!
typedef float4 Vector;
#define vec3(a,b,c) (float4)((a),(b),(c),0)
#define vec4(a,b,c,d) (float4)((a),(b),(c),(d))
#define vec2(a,b) (float2)((a),(b))
#else
#define ALIGN_END 
#define ALIGN_BEGIN
//magic for IntelliSense to work
#include <QVector3D> 
#include <math.h>
#include <CL/cl.h>
typedef cl_float4 Vector;
typedef cl_float2 float2;
#define __global
#define __constant
#define __kernel
#define __local
size_t get_global_id(unsigned int);
inline Vector vec3(float a, float b, float c) {
	Vector v;
	v.x = a;
	v.y = b;
	v.z = c;
	v.w = 0;
	return v;
}
inline Vector vec4(float a, float b, float c,float d) {
	Vector v;
	v.x = a;
	v.y = b;
	v.z = c;
	v.w = d;
	return v;
}
inline float2 vec2(float a, float b) {
	float2 v;
	v.x = a;
	v.y = b;
	return v;
}

inline float clamp(float x,float a, float b);
inline int clamp(int x, int a, int b);
inline float dot(const Vector&, const Vector);
inline float dot(const float2&, const float2);
inline Vector cross(const Vector&, const Vector);
inline Vector min(const Vector&a , const Vector& b) {
	return vec3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
}
inline float min(const float, const float);
inline Vector max(const Vector&a, const Vector&b){
	return vec3(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
}
inline float max(const float, const float);
inline float length(const Vector&);
inline Vector normalize(const Vector&v) {
	QVector3D vec(v.x, v.y, v.z);
	vec.normalize();
	return vec3(vec.x(), vec.y(), vec.z());
}
inline Vector operator * (float,const Vector&);
#define DEF_VEC_OP_HACK(op) inline Vector operator op (const Vector&, float); \
							inline float2 operator op (const float2&, float);
#define DEF_VEC_OP_HACK2(op) inline Vector operator op (const Vector&, const Vector&); \
							inline float2 operator op (const float2&, const float2&);
DEF_VEC_OP_HACK2(+)
DEF_VEC_OP_HACK2(-)
DEF_VEC_OP_HACK2(*)
DEF_VEC_OP_HACK2(/)
DEF_VEC_OP_HACK(*)
DEF_VEC_OP_HACK(/ )
DEF_VEC_OP_HACK2(+=)
DEF_VEC_OP_HACK2(-=)
DEF_VEC_OP_HACK2(*=)
DEF_VEC_OP_HACK2(/=)
DEF_VEC_OP_HACK(*= )
DEF_VEC_OP_HACK(/= )
#define DEF_VEC_OP(op) inline Vector operator op (const Vector&a, const Vector&b) { \
						return vec3(a.x op b.x,a.y op b.y,a.z op b.z); \
						}
DEF_VEC_OP(+)
DEF_VEC_OP(-)
DEF_VEC_OP(*)
DEF_VEC_OP(/)
inline Vector& operator += (Vector& lhs,const Vector& rhs) {
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	lhs.z += rhs.z;
	lhs.w += rhs.w;
	return lhs;
}
inline Vector& operator *= (Vector& lhs, const Vector& rhs) {
	lhs.x *= rhs.x;
	lhs.y *= rhs.y;
	lhs.z *= rhs.z;
	lhs.w *= rhs.w;
	return lhs;
}
inline Vector& operator /= (Vector& lhs, const Vector& rhs) {
	lhs.x /= rhs.x;
	lhs.y /= rhs.y;
	lhs.z /= rhs.z;
	lhs.w /= rhs.w;
	return lhs;
}
inline Vector& operator *= (Vector& lhs, const float rhs) {
	lhs.x *= rhs;
	lhs.y *= rhs;
	lhs.z *= rhs;
	lhs.w *= rhs;
	return lhs;
}
inline Vector& operator /= (Vector& lhs, const float rhs) {
	lhs.x /= rhs;
	lhs.y /= rhs;
	lhs.z /= rhs;
	lhs.w /= rhs;
	return lhs;
}
inline Vector& operator -= (Vector& lhs, const Vector& rhs) {
	lhs.x -= rhs.x;
	lhs.y -= rhs.y;
	lhs.z -= rhs.z;
	lhs.w -= rhs.w;
	return lhs;
}
inline Vector operator  *(const Vector& lhs, const float rhs) {
	auto v = lhs;
	v *= rhs;
	return v;
}
inline Vector operator  /(const Vector& lhs, const float rhs) {
	auto v = lhs;
	v /= rhs;
	return v;
}
#endif
typedef unsigned int Seed;

typedef ALIGN_BEGIN struct Material {
	Vector ambient;
	Vector diffuse;
	Vector specular;
	float Ni, Tr, roughness;
}ALIGN_END Material;

typedef ALIGN_BEGIN struct Sphere {
	Vector center;
	float radius;
}ALIGN_END Sphere;

typedef ALIGN_BEGIN struct Triangle {
	Vector vertex0, vertex1,vertex2;
	Vector norm, edge1,edge2;
	float2 texCoord0, texCoord1, texCoord2;
	int texId;
}ALIGN_END Triangle;

#define TYPE_SPHERE 1 
#define TYPE_TRIANGLE 2
typedef ALIGN_BEGIN struct Primitive  {
	union {
		Triangle triangle;
		Sphere sphere;
	};
	int material;
	int id;
	float area;
	int type;
} ALIGN_END Primitive;

typedef ALIGN_BEGIN struct AABB {
	Vector min, max;
}ALIGN_END AABB;


typedef ALIGN_BEGIN struct BVHNode {
	AABB box;
	int left, right;
	int begin, end;
	
}ALIGN_END BVHNode;
typedef Vector Pixel;

typedef ALIGN_BEGIN struct OptionalBuffer {
	int id;
	float depth;
	Pixel norm;
	Pixel pos;
	Pixel refl;
}ALIGN_END OptionalBuffer;

typedef ALIGN_BEGIN struct RenderContext {
	__global const Primitive * objects;
	__global const Material * materials;
	__global const BVHNode * nodes;
	__global const int * lights;
	__global int * primitivePool;
	__global OptionalBuffer * opt;
	int size;
	int width;
	int height;
	float aov;
	int samples;
	Vector origin;
	Vector dir;
	
	int objectCount;
	int lightCount;
}ALIGN_END RenderContext;

#define MODE_PREVIEW 0
#define MODE_RENDER 1



inline float vmax(Vector  v) {
	return (v.x > v.y) ? (v.x > v.z ? v.x : v.z) : (v.y > v.z ? v.y : v.z);
}

#define vassign(v,a,b,c) v = vec3((a),(b),(c))
#define v_op(op,a,b,c) vassign((c),(a).x op (b).x,(a).y op (b).y,(a).z op (b).z)
#define vadd(a,b,c) v_op(+,(a),(b),(c))
#define vsub(a,b,c) v_op(-,(a),(b),(c))
#define vmul(a,b,c) v_op(*,(a),(b),(c))
#define vdiv(a,b,c) v_op(/,(a),(b),(c))
#define vsmul(a,k,b) vassign(b,(a).x*(k),(a).y*(k),(a).z*(k))
#define vcross(a, b, v) vassign((v), (a).y * (b).z - (a).z * (b).y, (a).z * (b).x - (a).x * (b).z, (a).x * (b).y - (a).y * (b).x)
#define vmax(v) ((v).x > (v).y ? ((v).x > (v).z ? (v).x : (v).z) :((v).y > (v).z ? (v).y : (v).z))
#define vdot(a,b) ((a).x * (b).x + (a).y * (b).y + (a).z * (b).z)

inline float luminance(Vector v) {
	return 0.2126*v.x + 0.7152*v.y + 0.0722*v.z;
}
inline Vector vecRotate(const Vector x, const Vector axis, const float angle) {
	float s = sin(angle);
	float c = cos(angle);
	float oc = 1.0f - c;
	Vector u = vec3(oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s,
		oc * axis.z * axis.x + axis.y * s);
	Vector v = vec3(oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c,
		oc * axis.y * axis.z - axis.x * s);
	Vector w = vec3(oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s,
		oc * axis.z * axis.z + c);
	return vec3(vdot(u, x), vdot(v, x), vdot(w, x));
}
#endif
