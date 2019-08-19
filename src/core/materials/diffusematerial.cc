#include <core/materials/diffusematerial.h>
#include <core/bsdfs/bsdf.h>
#include <core/bsdfs/diffuse.hpp>

namespace Miyuki {
	namespace Core {
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

		BSDFComponent* DiffuseMaterial::createBSDF(BSDFCreationContext& ctx)const {
			Float _roughness = Shader::evaluate(roughness, ctx.shadingPoint).toFloat();
			Spectrum R = Shader::evaluate(color, ctx.shadingPoint).toVec3f();
			auto bsdf = ctx.alloc<DiffuseBSDF>(_roughness, R);
			return bsdf;
		}

	}
}