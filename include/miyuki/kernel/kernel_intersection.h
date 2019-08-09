#ifndef MIYUKI_KERNEL_INTERSECTION_H
#define MIYUKI_KERNEL_INTERSECTION_H

#include "kerneldef.h"
#include "kernel_globals.h"

MYK_KERNEL_NS_BEGIN

typedef struct CoordinateSystem {
	float3 local_x, local_y, normal;
}CoordinateSystem;

MYK_KERNEL_FUNC_INLINE
void create_coordinate_system(CoordinateSystem* frame, float3 N) {
	frame->normal = N;
	frame->local_x = normalize(cross((abs(N.x) > 0.1) ? make_float3(0, 1, 0) : make_float3(1, 0, 0), N));
	frame->local_y = normalize(cross(N, frame->local_x));
}

struct Primitive;
struct Material;

typedef struct Intersection {
	struct Primitive* primitive;
	struct Material* material;
	float distance;
	float2 uv, texture_uv;
	float3 p;
	float3 wo;
	int geom_id, prim_id;
	CoordinateSystem local_frame;
}Intersection;

MYK_KERNEL_FUNC_INLINE
void init_intersection(Intersection* isct) {
	isct->geom_id = -1;
	isct->prim_id = -1;
	isct->primitive = NULL;
}

typedef struct Ray {
	float3 o, d;
	float t_min, t_max;
	float time;
}Ray;

MYK_KERNEL_FUNC
bool intersect(KernelGlobals* globals, const Ray* ray, Intersection* isct);

MYK_KERNEL_FUNC
bool  occluded(KernelGlobals* globals, const Ray* ray);

MYK_KERNEL_FUNC
bool intersect_primitive(KernelGlobals* globals, const Ray* ray, const Primitive* primitive);

MYK_KERNEL_NS_END


#endif 