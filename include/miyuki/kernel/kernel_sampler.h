#ifndef MIYUKI_KERNEL_SAMPLER_H
#define MIYUKI_KERNEL_SAMPLER_H

#include "kerneldef.h"
MYK_KERNEL_NS_BEGIN

#define MYK_SAMPLER_SOBOL 1
#define MYK_SAMPLER_RANDOM 2
#define MYK_USE_SAMPLER MYK_SAMPLER_SOBOL

typedef unsigned int RNG;

typedef struct SamplerState {
	int2 pixel;
	int sample;
	int dimension;
	float rotation;
	RNG rng;
}SampleState;



MYK_KERNEL_FUNC_INLINE float lcg_rng(RNG* rng)
{
	*rng = (1103515245 * (*rng) + 12345);
	return (float)* rng / (float)0xFFFFFFFF;
}

MYK_KERNEL_FUNC void start_sampler(SamplerState* state) {
	state->dimension = 0;
	state->rotation = lcg_rng(&state->rng);
}

MYK_KERNEL_FUNC void start_next_sample(SamplerState* state) {
	state->dimension = 0;
}

MYK_KERNEL_FUNC_INLINE float next1d(SamplerState* state);

float2 next2d(SamplerState* state) {
	return make_float2(next1d(state), next1d(state));
}
MYK_KERNEL_NS_END

#if MYK_USE_SAMPLER == MYK_SAMPLER_SOBOL
#include <core/samplers/sobolmat.hpp>

MYK_KERNEL_NS_BEGIN
static inline uint32_t cmj_hash_simple(uint32_t i, uint32_t p) {
	i = (i ^ 61) ^ p;
	i += i << 3;
	i ^= i >> 4;
	i *= 0x27d4eb2d;
	return i;
}

#define SOBOL_SKIP 64

static inline float sobol(const unsigned int vectors[][32], unsigned int dimension, unsigned int i, unsigned int rng) {
	unsigned int result = 0;
	i += SOBOL_SKIP;
	for (unsigned int j = 0; i; i >>= 1, j++)
		if (i & 1)
			result ^= vectors[dimension][j];

	float r = result * (1.0f / (float)0xFFFFFFFF);
	uint32_t tmp_rng = cmj_hash_simple(dimension, rng);
	float shift = tmp_rng * (1.0f / (float)0xFFFFFFFF);
	return r + shift - floorf(r + shift);
}

MYK_KERNEL_FUNC_INLINE float next1d(SamplerState* state) {
	return sobol(SobolMatrix, state->dimension++, state->sample, state->rotation);
}

MYK_KERNEL_NS_END

#elif MYK_USE_SAMPLER == MYK_SAMPLER_RANDOM
MYK_KERNEL_NS_BEGIN

MYK_KERNEL_FUNC_INLINE float next1d(SamplerState* state) {
	return lcg_rng(&state->rng);
}

MYK_KERNEL_NS_END
#else
static_assert(false, "What the hell");
#endif 



#endif  