#pragma once
#include <core/ray.h>

namespace Miyuki {
	namespace Core {
		class Scene;
		struct VisibilityTester {
			Ray shadowRay;
			int32_t geomId, primId;

			VisibilityTester() : geomId(-1), primId(-1) {}

			bool visible(Scene& scene);
		};

	}
}