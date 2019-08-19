#ifndef MIYUKI_BSDF_DIFFUSE_HPP
#define MIYUKI_BSDF_DIFFUSE_HPP
#include <core/bsdfs/scatteringfunction.hpp>
#include <core/bsdfs/trignometry.hpp>
#include <core/bsdfs/bsdf.h>
namespace Miyuki {
	namespace Core {
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
				return CosineHemispherePDF().evaluate(wi);
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
				return CosineHemispherePDF().evaluate(wi);
			}
		};

		class DiffuseBSDF : public BSDFComponent {
			const Float roughness;
			const Spectrum R;
		public:
			DiffuseBSDF(Float roughness, Spectrum R)
				:BSDFComponent(BSDFLobe(EReflection | EDiffuse)), R(R), roughness(roughness) {}

			virtual void sample(
				BSDFEvaluationContext& ctx,
				BSDFSample& sample
			)const override {
				if (roughness >= 1e-6f) {
					OrenNayarReflection(R, roughness).sample(sample);
				}
				else {
					LambertianReflection(R).sample(sample);
				}
				if (!SameHemisphere(sample.wi, sample.wo)) {
					sample.wi *= -1;
				}
				sample.lobe = getLobe();
				ctx.assignWi(sample.wi);
			}

			// evaluate bsdf according to wo, wi
			virtual Spectrum evaluate(
				const BSDFEvaluationContext& ctx
			)const override {
				auto& wo = ctx.wo();
				auto& wi = ctx.wi();
				if (!SameHemisphere(wo, wi))return {};
				if (roughness < 0.0f) {
					return LambertianReflection(R).evaluate(wo, wi);
				}
				else {
					return OrenNayarReflection(R, roughness).evaluate(wo, wi);
				}
			}

			// evaluate pdf according to wo, wi
			virtual Float evaluatePdf(
				const BSDFEvaluationContext& ctx
			)const override {
				auto& wo = ctx.wo();
				auto& wi = ctx.wi();
				if (!SameHemisphere(wo, wi))return {};
				if (roughness < 0.0f) {
					return LambertianReflection(R).evaluatePdf(wo, wi);
				}
				else {
					return OrenNayarReflection(R, roughness).evaluatePdf(wo, wi);
				}
			}
		};
	}
}



#endif