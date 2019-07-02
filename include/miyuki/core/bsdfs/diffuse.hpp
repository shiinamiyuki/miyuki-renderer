#ifndef MIYUKI_BSDF_DIFFUSE_HPP
#define MIYUKI_BSDF_DIFFUSE_HPP
#include <core/bsdfs/scatteringfunction.hpp>

namespace Miyuki {
	namespace Core {
		template<class Derived>
		struct MicrofacetModel {

		};

		struct OrenNayarReflection : ScatteringFunction<OrenNayarReflection> {
			Spectrum R;
			OrenNayarReflection(const Spectrum& R) :R(R) {}
		};

		struct LambertianReflection : ScatteringFunction<LambertianReflection> {
			Spectrum R;
			LambertianReflection(const Spectrum& R) :R(R) {}
			void sampleImpl(
				ScatteringFunctionSample& sample)const {
				sample.wi = CosineHemispherePDF().sample(sample.u, &sample.pdf);
				sample.f = evaluate(sample.wo, sample.wi);
			}

			Spectrum evaluateImpl(const Vec3f& wo, const Vec3f& wi)const {
				return R * INVPI;
			}

			// evaluate pdf according to wo, wi
			Float evaluatePdfImpl(const Vec3f& wo, const Vec3f& wi)const {
				return CosineHemispherePDF().evaluate(AbsCosTheta(wi));
			}
		};
	}
}



#endif