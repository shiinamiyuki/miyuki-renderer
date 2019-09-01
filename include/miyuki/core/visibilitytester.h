#pragma once
#include <core/ray.h>

namespace Miyuki {
	namespace Core {
		class Scene;
		class Sampler;
		struct VisibilityTester {
			Ray shadowRay;
			int32_t geomId, primId;

			VisibilityTester() : geomId(-1), primId(-1) {}

			bool visible(Scene& scene);
			Spectrum Tr(Scene& scene, Sampler & sampler);
		};

	}
}