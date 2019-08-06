#include <core/materials/glossymaterial.h>
#include <core/bsdfs/bsdf.h>
#include <core/bsdfs/microfacet.hpp>
#include <core/bsdfs/fresnel.hpp>

namespace Miyuki {
	namespace Core {
		class GlossyBSDFImpl : public BSDFImpl {
			const MicrofacetWrapper microfacet;
			const FresnelWrapper fresnel;
			const Vec3f R;
			const Float alpha;
		public:
			GlossyBSDFImpl(Vec3f R, Float alpha)
				:R(R), alpha(alpha), microfacet(EBeckmann, alpha), fresnel(EPerfectSpecular),
				BSDFImpl(BSDFLobe(EGlossy | EReflection)) {

			}
			virtual void sample(
				BSDFEvaluationContext& ctx,
				BSDFSample& sample
			)const override {
				auto wh = microfacet.sampleWh(sample.wo, sample.u);
				sample.wi = Reflect(sample.wo, wh);
				ctx.assignWi(sample.wi);
				if (!SameHemisphere(sample.wo, sample.wi)) {
					sample.pdf = 0.0f;
					sample.f = {};
				}
				else {
					sample.pdf = microfacet.evaluatePdf(wh) / (4.0f * Vec3f::dot(sample.wo, wh));
					sample.f = evaluate(ctx);
				
				}
				CHECK(!sample.f.hasNaNs());
				sample.lobe = getLobe();
				
			}

			// evaluate bsdf according to wo, wi
			virtual Spectrum evaluate(
				const BSDFEvaluationContext& ctx
			)const override {
				auto& wo = ctx.wo();
				auto& wi = ctx.wi();
				Float cosThetaO = AbsCosTheta(wo);
				Float cosThetaI = AbsCosTheta(wi);
				auto wh = (wo + wi);
				if (cosThetaI == 0 || cosThetaO == 0)return {};
				if (wh.x == 0 && wh.y == 0 && wh.z == 0)return {};
				wh.normalize();
				auto F = fresnel.evaluate(Vec3f::dot(wi, wh));
				return R * F * microfacet.D(wh) * microfacet.G(wo, wi) * F / (4.0f * cosThetaI * cosThetaO);
			}

			// evaluate pdf according to wo, wi
			virtual Float evaluatePdf(
				const BSDFEvaluationContext& ctx
			) const override {
				auto& wo = ctx.wo();
				auto& wi = ctx.wi();
				if (!SameHemisphere(wo, wi))return 0.0f;
				auto wh = (wo + wi).normalized();
				return microfacet.evaluatePdf(wh) / (4.0f * Vec3f::dot(wo, wh));
			}
		};

		class SpecularBSDFImpl : public BSDFImpl {
			const Vec3f R;
		public:
			SpecularBSDFImpl(const Vec3f& R) :R(R), BSDFImpl(BSDFLobe(ESpecular | EReflection)) {}
			virtual void sample(
				BSDFEvaluationContext& ctx,
				BSDFSample& sample
			)const override {
				sample.wi = Reflect(sample.wo, Vec3f(0, 0, 1));
				sample.f = R / AbsCosTheta(sample.wi);
				sample.pdf = 1.0f;
				sample.lobe = getLobe();
				ctx.assignWi(sample.wi);
			}

			// evaluate bsdf according to wo, wi
			virtual Spectrum evaluate(
				const BSDFEvaluationContext& ctx
			)const override {
				return {};
			}

			// evaluate pdf according to wo, wi
			virtual Float evaluatePdf(
				const BSDFEvaluationContext& ctx
			) const override {
				return 0.0f;
			}

		};
		BSDFImpl* GlossyMaterial::createBSDF(BSDFCreationContext& ctx)const {
			auto _roughness = Shader::evaluate(roughness, ctx.shadingPoint).toFloat();
			auto _color = Shader::evaluate(color, ctx.shadingPoint).toVec3f();
			if (_roughness == 0.0f) {
				return ctx.alloc<SpecularBSDFImpl>(_color);
			}
			else
				return ctx.alloc<GlossyBSDFImpl>(_color, _roughness * _roughness);
		}
	}
}