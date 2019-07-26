#include <core/materials/glossymaterial.h>
#include <core/bsdfs/bsdf.h>
#include <core/bsdfs/microfacet.hpp>
#include <core/bsdfs/fresnel.hpp>

namespace Miyuki {
	namespace Core {
		class GlossyBSDFImpl : public BSDFImpl {
			MicrofacetWrapper microfacet;
			FresnelWrapper fresnel;
			Vec3f R; 
			Float alpha;
		public:
			GlossyBSDFImpl(Vec3f R, Float alpha)
				:R(R), alpha(alpha), microfacet(EBeckmann, alpha),fresnel(EPerfectSpecular),
				BSDFImpl(BSDFLobe(EGlossy | EReflection)) {

			}
			virtual void sample(
				BSDFSample& sample
			)const override {

				auto wh = microfacet.sampleWh(sample.wo, sample.u);
				sample.wi = Reflect(sample.wo, wh);
				if (!SameHemisphere(sample.wo, sample.wi)) {
					sample.f = {};
				}
				else {
					sample.pdf = microfacet.evaluatePdf(wh) / (4.0f * Vec3f::dot(sample.wo, wh));
					sample.f = evaluate(sample.wo, sample.wi, sample.option, sample.lobe);
				}

			}

			// evaluate bsdf according to wo, wi
			virtual Spectrum evaluate(
				const Vec3f& wo,
				const Vec3f& wi,
				BSDFSampleOption option,
				BSDFLobe lobe = BSDFLobe::EAll
			)const override {
				Float cosThetaO = AbsCosTheta(wo);
				Float cosThetaI = AbsCosTheta(wi);
				auto wh = (wo + wi);
				if (cosThetaI == 0 || cosThetaO == 0)return {};
				if (wh.x() == 0 && wh.y() == 0 && wh.z() == 0)return {};
				wh.normalize();
				auto F = fresnel.evaluate(Vec3f::dot(wi, wh));
				return F * microfacet.D(wh) * microfacet.G(wo, wi) * F / (4.0f * cosThetaI * cosThetaO);
			}

			// evaluate pdf according to wo, wi
			virtual Float evaluatePdf(
				const Vec3f& wo,
				const Vec3f& wi,
				BSDFSampleOption option,
				BSDFLobe lobe = BSDFLobe::EAll
			) const override{
				if (!SameHemisphere(wo, wi))return 0.0f;
				auto wh = (wo + wi).normalized();
				return microfacet.evaluatePdf(wh) / (4.0f * Vec3f::dot(wo, wh));
			}
		};

		BSDFImpl* GlossyMaterial::createBSDF(BSDFCreationContext& ctx)const {
			auto _roughness = roughness->eval(ctx.shadingPoint).toFloat();
			auto _color = color->eval(ctx.shadingPoint).toVec3f();
			return ctx.alloc<GlossyBSDFImpl>(_color, _roughness * _roughness);
		}
	}
}