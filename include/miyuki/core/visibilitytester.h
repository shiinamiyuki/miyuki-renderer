#pragma once
#include <core/ray.h>

namespace Miyuki {
	namespace Core {
		class Scene;
		class Sampler;
		class Medium;
		struct Intersection;
		struct VisibilityTester {
			Ray shadowRay;
			int32_t geomId, primId;

			VisibilityTester() : geomId(-1), primId(-1) {}

			bool visible(Scene& scene);
			Spectrum Tr(Scene& scene, Sampler & sampler);
		};

		// Compute transmission between ray.o and isct
		// Updates ray.mediumStack if needed
		// ray.mediumStack mustn't be null
		Spectrum Tr(const Ray& ray, const Intersection& isct, Sampler& sampler);

		// Updates ray.mediumStack if needed
		// Returns the medium needed to be sampled/evaluated
		Medium* UpdateMediumStack(const Ray& ray, const Intersection& isct);
	}
}