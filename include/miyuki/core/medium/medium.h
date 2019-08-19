#pragma once

#include <reflection.h>
#include <core/ray.h>
namespace Miyuki {
	namespace Core {
		class Sampler;
		struct MediumSample {
			const Ray ray;
			Sampler* sampler;
			MediumSample(const Ray& ray, Sampler* sampler) :ray(ray),sampler(sampler) {}
		};
		class Medium : public Reflective {
		public:
			MYK_INTERFACE(Medium);
			virtual Spectrum Tr(const Ray& ray, Sampler& sampler)const = 0;
			virtual void sample(MediumSample& sample)const = 0;
		};
		MYK_REFL(Medium, (Reflective), MYK_REFL_NIL);
	}
}