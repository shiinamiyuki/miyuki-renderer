#ifndef MIYUKI_KRENEL_MATERIAL_SAMPLE_H
#define MIYUKI_KRENEL_MATERIAL_SAMPLE_H

#include "kernel_sampler.h"
#include "kernel_func.h"
#include "kernel_svm.h"


MYK_KERNEL_NS_BEGIN

typedef struct BSDFSample {
	float3 wi;
	float3 wo;
	float  pdf;
	float3 bsdf;
	float2 u_pick;
	float2 u_sample;
	BSDFLobe sampled_lobe;
}BSDFSample;


MYK_KERNEL_FUNC_INLINE
void create_bsdf_sample(BSDFSample* sample, float3 wo, SamplerState* sampler) {
	sample->wo = wo;
	sample->u_pick = next1d(sampler);
	sample->u_sample = next2d(sampler);
	sample->sampled_lobe = BSDF_NONE;
	sample->pdf = 0.0f;
}

MYK_KERNEL_FUNC
void diffuse_material_sample(DiffuseMaterial* mat, KernelGlobals* kg, BSDFSample* sample, ShadingPoint sp) {
	float3 color = svm_eval(kg, sp, &mat->color);
	float roughness = svm_eval(kg, sp, &mat->roughness).x;
	sample->bsdf = color * INVPI;
	sample->wi = cosine_hemisphere_sampling(sample->u_sample);
	sample->pdf = cosine_hemiphsere_pdf(sample->wi);
}

MYK_KERNEL_FUNC
float diffuse_material_pdf(DiffuseMaterial* mat, float3 wi, float3 wo) {
	return cosine_hemiphsere_pdf(wo);
}


MYK_KERNEL_NS_END


#endif