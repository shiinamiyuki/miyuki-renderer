#ifndef MIYUKI_KERNEL_SHADER_H
#define MIYUKI_KERNEL_SHADER_H

#include "kerneldef.h"

MYK_KERNEL_NS_BEGIN

struct ImageTexture;
MYK_KERNEL_FUNC float3 fetch_image_texture(struct ImageTexture* texture, float2 uv);

MYK_KERNEL_NS_END

#include "kernel_texture.h"
#include "generated.shader.h"
#include "kernel_svm.h"

MYK_KERNEL_NS_BEGIN

MYK_KERNEL_FUNC
void float_shader_eval(FloatShader* shader, KernelGlobals*, SVM* svm) {
	float v = shader->value;
	svm_push(svm, make_float3(v, v, v));
}

MYK_KERNEL_FUNC
void float3_shader_eval(Float3Shader* shader, KernelGlobals*, SVM* svm) {
	svm_push(svm, shader->value);
}

MYK_KERNEL_FUNC
void image_texture_shader_eval(ImageTextureShader* shader, KernelGlobals*, SVM* svm) {
	float3 value = fetch_image_texture(&shader->texture, svm->shading_point.uv);
	svm_push(svm, value);
}

MYK_KERNEL_FUNC
void mixed_shader_eval(MixedShader* shader, KernelGlobals*, SVM* svm) {
	ShadingResult fraction = svm_pop(svm);
	ShadingResult A = svm_pop(svm);
	ShadingResult B = svm_pop(svm);
	svm_push(svm, fraction * A + (make_float3(1, 1, 1) - fraction) * B);
}

MYK_KERNEL_FUNC
void scaled_shader_eval(ScaledShader* shader, KernelGlobals*, SVM* svm) {
	ShadingResult k = svm_pop(svm);
	ShadingResult v = svm_pop(svm);
	svm_push(svm, k * v);
}

MYK_KERNEL_FUNC
void end_shader_eval(EndShader* shader, KernelGlobals*, SVM* svm) {
	svm->halt = true;
}


MYK_KERNEL_FUNC
void shader_eval(KernelGlobals* globals, SVM* svm) {
	Shader* s = read_next_shader(globals, svm);
	svm_pc_advance(svm);
	DISPATCH_SHADER(eval, s, globals, svm)
}

MYK_KERNEL_FUNC
ShadingResult svm_eval(KernelGlobals* globals, ShadingPoint sp, ShaderData * data) {
	SVM svm;
	svm_init(&svm, data->offset, sp);
	while (!svm.halt) {
		shader_eval(globals, &svm);
	}
	return svm_pop(&svm);
}

MYK_KERNEL_NS_END

#endif
