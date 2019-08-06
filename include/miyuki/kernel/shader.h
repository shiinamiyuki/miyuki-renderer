
#ifndef MIYUKI_KERNEL_SHADER_H
#define MIYUKI_KERNEL_SHADER_H

#include "kerneldef.h"

MYK_KERNEL_NS_BEGIN

struct ImageTexture;

float3 fetch_image_texture(ImageTexture* texture, float2 uv);

MYK_KERNEL_NS_END
#include "shader.generated.h"

MYK_KERNEL_NS_BEGIN


MYK_KERNEL_NS_END
#endif
