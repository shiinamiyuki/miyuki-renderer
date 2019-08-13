#ifndef MIYUKI_KERNEL_CAMERA_SAMPLE_H
#define MIYUKI_KERNEL_CAMERA_SAMPLE_H

#include "kernel_globals.h"
#include "kernel_intersection.h"
#include "kernel_sampler.h"

MYK_KERNEL_NS_BEGIN

typedef struct CameraSample {
	float2 pLens, pFilm;
	float weight;
	Ray primary;
}CameraSample;

MYK_KERNEL_FUNC_INLINE
float3 camera_to_world(const Camera* camera, float3 v) {
	float4 result = mat4x4_mult(camera->transform, make_float4(v.x, v.y, v.z, 1.0f));
	return make_float3(result.x, result.y, result.z);
}

MYK_KERNEL_FUNC_INLINE
void perspective_camera_sample(const Camera* camera, CameraSample* sample, int2 raster, SamplerState* state) {
	Float x = raster.x;
	Float y = raster.y;
	Float rx = 0.5f * (2 * next1d(state) - 1);
	Float ry = 0.5f * (2 * next1d(state) - 1);
	x += rx;
	y += ry;
	sample->pFilm = Point2f(x, y);
	x = -(2 * x / camera->dimension.x - 1) * (float)(camera->dimension.x) /
		camera->dimension.y;
	y = 2 * (1 - y / camera->dimension.y) - 1;

	float3 ro(0, 0, 0);
	auto z = (Float)(2.0 / tan(camera->fov / 2));
	sample->weight = 1;
	float3 rd = float3(x, y, 0) - float3(0, 0, -z);
	rd = normalize(rd);
	if (camera->lens_radius > 0) {
		float2 pLens = make_float2(camera->lens_radius, camera->lens_radius) * concetric_disk_sampling(next2d(state));
		Float ft = camera->focal_distance / rd.z;
		auto pFocus = ro + ft * rd;
		ro = make_float3(pLens.x, pLens.y, 0);
		rd = normalize(pFocus - ro);
		sample->pLens = pLens;
	}
	else {
		sample->pLens = make_float2(0, 0);
	}
	ro = camera_to_world(camera, ro);
	ro += camera->position;

	rd = normalize(camera_to_world(camera, rd));
	sample->primary.origin = ro;
	sample->primary.direction = rd;
}
MYK_KERNEL_NS_END

#endif