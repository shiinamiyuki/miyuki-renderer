#ifndef MIYUKI_KERNEL_KERNEL_GLOBALS_H
#define MIYUKI_KERNEL_KERNEL_GLOBALS_H

#include "kerneldef.h"


MYK_KERNEL_NS_BEGIN

struct Primitive;
struct Scene;
struct Material;
struct MeshInstance;

typedef struct Camera {

}Camera;

typedef struct KernelGlobals {
	ShaderProgram program;
	struct Scene* scene;
	struct Material* materials;
	struct MeshInstance* instances;
}KernelGlobals;




MYK_KERNEL_NS_END

#endif