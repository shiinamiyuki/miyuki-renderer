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
			Float A, B;

			OrenNayarReflection(const Spectrum& R, Float sigma) :R(R) {
				Float sigma2 = sigma * sigma;
				A = 1.f - (sigma2 / (2.f * (sigma2 + 0.33f)));
				B = 0.45f * sigma2 / (sigma2 + 0.09f);
			}
			void sampleImpl(ScatteringFunctionSample& sample)const {
				sample.wi = CosineHemispherePDF().sample(sample.u, &sample.pdf);
				sample.f = evaluate(sample.wo, sample.wi);
			}

			Spectrum evaluateImpl(const Vec3f& wo, const Vec3f& wi)const {
				void sampleImpl(
					Float sinThetaI = SinTheta(wi);
				Float sinThetaO = SinTheta(wo);
				Float maxCos = 0;
				if (sinThetaI > 1e-4 && sinThetaO > 1e-4) {
					Float sinPhiI = SinPhi(wi), cosPhiI = CosPhi(wi);
					Float sinPhiO = SinPhi(wo), cosPhiO = CosPhi(wo);
					Float dCos = cosPhiI * cosPhiO + sinPhiI * sinPhiO;
					maxCos = std::max((Float)0, dCos);
				}
				Float sinAlpha, tanBeta;
				if (AbsCosTheta(wi) > AbsCosTheta(wo)) {
					sinAlpha = sinThetaO;
					tanBeta = sinThetaI / AbsCosTheta(wi);
				}
				else {
					sinAlpha = sinThetaI;
					tanBeta = sinThetaO / AbsCosTheta(wo);
				}

				return R * INVPI * (A + B * maxCos * sinAlpha * tanBeta);
			}

			Float evaluatePdfImpl(const Vec3f& wo, const Vec3f& wi)const {
				return CosineHemispherePDF().evaluate(AbsCosTheta(wi));
			}
		};

		struct LambertianReflection : ScatteringFunction<LambertianReflection> {
			Spectrum R;
			LambertianReflection(const Spectrum& R) :R(R) {}
			void sampleImpl(ScatteringFunctionSample& sample)const {
				sample.wi = CosineHemispherePDF().sample(sample.u, &sample.pdf);
				sample.f = evaluate(sample.wo, sample.wi);
			}

			Spectrum evaluateImpl(const Vec3f& wo, const Vec3f& wi)const {
				return R * INVPI;
			}

			Float evaluatePdfImpl(const Vec3f& wo, const Vec3f& wi)const {
				return CosineHemispherePDF().evaluate(AbsCosTheta(wi));
			}
		};
	}
}



#endif