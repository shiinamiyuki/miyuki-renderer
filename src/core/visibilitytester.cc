#include <core/visibilitytester.h>
#include <core/intersection.hpp>
#include <core/scene.h>

namespace Miyuki {
	namespace Core {
		bool VisibilityTester::visible(Scene& scene) {
			Intersection intersection;
			if (!scene.intersect(shadowRay, &intersection)) {
				return true;
			}
			return intersection.geomId == geomId && intersection.primId == primId;
		}
	}
}