#ifndef MIYUKI_KERNEL_SCENE_H
#define MIYUKI_KERNEL_SCENE_H


#include "kerneldef.h"

MYK_KERNEL_NS_BEGIN
struct KernelGlobals;
struct Material;
struct Scene;

MYK_KERNEL_FUNC struct Material* get_material_by_id(KernelGlobals * globals, int material_id);

MYK_KERNEL_NS_END

#endif