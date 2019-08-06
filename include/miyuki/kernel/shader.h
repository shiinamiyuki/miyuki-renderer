
#ifndef MIYUKI_KERNEL_SHADER_H
#define MIYUKI_KERNEL_SHADER_H

#include "kerneldef.h"

MYK_KERNEL_NS_BEGIN

struct ImageTexture;

MYK_KERNEL_FUNC float3 fetch_image_texture(struct ImageTexture* texture, float2 uv);

MYK_KERNEL_NS_END


#include "shader.generated.h"

MYK_KERNEL_NS_BEGIN

typedef struct ShadingPoint {
	float2 uv;
}ShadingPoint;

MYK_KERNEL_FUNC float3 shader_eval(const Shader* shader, const ShadingPoint* sp);

MYK_KERNEL_FUNC float3 float_shader_eval(const FloatShader* shader, const ShadingPoint* sp) {
	float v = shader->value;
	return make_float3(v, v, v);
}

MYK_KERNEL_FUNC float3 float3_shader_eval(const Float3Shader* shader, const ShadingPoint* sp) {
	return shader->value;
}

MYK_KERNEL_FUNC float3 image_texture_shader_eval(const ImageTextureShader* shader, const ShadingPoint* sp) {
	return fetch_image_texture(shader->texture, sp->uv);
}

MYK_KERNEL_FUNC float3 scaled_shader_eval(const ScaledShader* shader, const ShadingPoint* sp) {
	if (!shader->shader) {
		return make_float3(0, 0, 0);
	}
	float3 r = shader_eval(shader->shader, sp);
	if (!shader->scale) {
		return r;
	}
	return r * shader_eval(shader->scale, sp);
}

MYK_KERNEL_FUNC float3 mixed_shader_eval(const MixedShader* shader, const ShadingPoint* sp) {
	if (!shader->fraction) {
		return make_float3(0, 0, 0);
	}
	float3 fraction = shader_eval(shader->fraction, sp);
	float3 a = shader_eval(shader->shaderA, sp);
	float3 b = shader_eval(shader->shaderB, sp);
	return fraction * a + (make_float3(1, 1, 1) - fraction) * b;
}

MYK_KERNEL_FUNC float3 shader_eval(const Shader* shader, const ShadingPoint* sp) {
	DISPATCH_SHADER(eval, shader, sp)
}

MYK_KERNEL_NS_END
#endif
