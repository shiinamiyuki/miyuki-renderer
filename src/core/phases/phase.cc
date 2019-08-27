#include <core/phases/phase.h>
#include <math/transform.h>
#include <math/func.h>

namespace Miyuki {
	namespace Core {
		Float HenyeyGreenstein::evaluate(const Vec3f& wo, const Vec3f& wi)const {
			return PhaseHG(Vec3f::dot(wo, wi), g);
		}
		void HenyeyGreenstein::sample(PhaseFunctionSample& sample)const {
			Float cosTheta;
			if (std::abs(g) < 1e-3f) {
				cosTheta = 1 - 2 * sample.u[0];
			}
			else {
				Float g2 = g * g;
				Float sq = (1 - g2) / (1 - g + 2 * g * sample.u[0]);
				cosTheta = (1 + g2 - sq * sq) / (2 * g);
			}
			Float sinTheta = std::sqrt(std::max(0.0f, 1 - cosTheta * cosTheta));
			Float phi = 2 * sample.u[1] * PI;
			Vec3f v1, v2;
			CoordinateSystem coord(sample.wo);

			auto w = SphericalToXYZ(sinTheta, cosTheta, phi);
			sample.wi = coord.localToWorld(w);
		}
	}
}