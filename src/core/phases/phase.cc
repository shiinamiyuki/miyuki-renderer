#include <core/phases/phase.h>

namespace Miyuki {
	namespace Core {
		Float HenyeyGreenstein::evaluate(const Vec3f& wo, const Vec3f& wi)const {
			return PhaseHG(Vec3f::dot(wo, wi), g);
		}
	}
}