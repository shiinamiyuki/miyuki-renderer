#ifndef MIYUKI_KERNEL_KERNEL_GLOBALS_H
#define MIYUKI_KERNEL_KERNEL_GLOBALS_H

#include "kerneldef.h"
#include "kernel_func.h"

MYK_KERNEL_NS_BEGIN

struct Primitive;
struct Scene;
struct Material;
struct MeshInstance;

typedef struct Camera {
	Mat4x4 transform;
	int2 dimension;
	float3 position;
	float3 direction;
	float fov;
	float lens_radius;
	float focal_distance;
}Camera;

typedef struct KernelGlobals {
	Camera camera;
	ShaderProgram program;
	struct Scene* scene;
	struct Material* materials;
	struct MeshInstance* instances;
}KernelGlobals;




MYK_KERNEL_NS_END

#endif