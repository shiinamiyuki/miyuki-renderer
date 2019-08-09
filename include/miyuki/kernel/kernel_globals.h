#ifndef MIYUKI_KERNEL_KERNEL_GLOBALS_H
#define MIYUKI_KERNEL_KERNEL_GLOBALS_H

#include "kerneldef.h"


MYK_KERNEL_NS_BEGIN

struct Primitive;
struct Scene;

typedef struct KernelGlobals {
	ShaderProgram program;
	struct Scene* scene;
}KernelGlobals;




MYK_KERNEL_NS_END

#endif