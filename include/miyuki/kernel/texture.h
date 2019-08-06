#ifndef MIYUKI_KERNEL_TEXTURE_H
#define MIYUKI_KERNEL_TEXTURE_H

#include "kerneldef.h"

MYK_KERNEL_NS_BEGIN

typedef struct ImageTexture {
	float4* data;
	unsigned int width, height;
}ImageTexture ;

MYK_KERNEL_FUNC float3 fetch_image_texture(ImageTexture* texture, float2 uv) {
}

MYK_KERNEL_NS_END


#endif