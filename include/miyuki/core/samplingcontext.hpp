#ifndef MIYUKI_SAMPLING_CONTEXT_HPP
#define MIYUKI_SAMPLING_CONTEXT_HPP

#include <core/cameras/camera.h>
#include <core/samplers/sampler.h>
#include <core/memory.h>

namespace Miyuki {
	namespace Core {
		struct SamplingContext {
			Camera* camera;
			Sampler* sampler;
			MemoryArena* arena;
			CameraSample cameraSample;
			RayDifferential primary;
			SamplingContext(Camera* camera,
				Sampler* sampler,
				MemoryArena* arena,
				const CameraSample& cameraSample,
				const RayDifferential& primary) :camera(camera),
				sampler(sampler),
				arena(arena),
				cameraSample(cameraSample),
				primary(primary) {}
		};

		inline SamplingContext CreateSamplingContext(
			Camera* camera,
			Sampler* sampler,
			MemoryArena* arena,
			const Point2i& dimension,
			const Point2i& raster) {
			sampler->startNextSample();
			Ray primary;
			CameraSample sample;
			camera->generateRay(*sampler, dimension, raster, &primary, &sample);
			return SamplingContext(camera, sampler, arena, sample, primary);
		}
	}
}
#endif