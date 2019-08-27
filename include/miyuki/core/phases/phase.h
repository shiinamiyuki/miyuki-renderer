#pragma once

#include <reflection.h>
#include <miyuki.h>

namespace Miyuki {
	namespace Core {
		struct PhaseFunctionSample {
			Vec3f wi;
			Vec3f wo;
			Float phase;
			Point2f u;
		};

		class PhaseFunction {
		public:
			virtual Float evaluate(const Vec3f& wo, const Vec3f& wi)const = 0;
			virtual void sample(PhaseFunctionSample& sample)const = 0;

		};

		inline Float PhaseHG(Float cosTheta, Float g) {
			Float denom = 1 + g * g + 2 * g * cosTheta;
			return INV4PI * (1 - g * g) / (denom * std::sqrt(denom));
		}

		class HenyeyGreenstein : public PhaseFunction {
			const Float g;
		public:
			HenyeyGreenstein(Float g):g(g) {}
			virtual Float evaluate(const Vec3f& wo, const Vec3f& wi)const override;
			virtual void sample(PhaseFunctionSample& sample)const override;
		};

	}
}