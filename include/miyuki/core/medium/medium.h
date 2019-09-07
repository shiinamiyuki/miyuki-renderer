#pragma once

#include <reflection.h>
#include <core/ray.h>
#include <utils/preprocessable.hpp>

namespace Miyuki {
	class MemoryArena;
	namespace Core {
		class Sampler;
		class PhaseFunction;
		struct MediumSample {
			const Ray ray;
			Sampler* sampler;
			MemoryArena* arena;
			Vec3f origin;// new ray origin
			PhaseFunction* phase = nullptr;
			MediumSample(const Ray& ray,
				Sampler* sampler,
				MemoryArena* arena) :ray(ray), sampler(sampler), arena(arena) {}
			bool isValid()const {
				return phase != nullptr;
			}
		};
		class Medium : public Reflective, public CachedPreprocessable {
		public:
			MYK_INTERFACE(Medium);
			virtual Spectrum Tr(const Ray& ray, Sampler& sampler)const = 0;
			virtual Spectrum sample(MediumSample& sample)const = 0;
			virtual void doPreprocess() override {}
		};
		MYK_REFL(Medium, (Reflective), MYK_REFL_NIL);
	}
}