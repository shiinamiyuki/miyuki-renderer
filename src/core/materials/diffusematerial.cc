#include <core/materials/diffusematerial.h>
#include <core/bsdfs/bsdf.h>
#include <core/bsdfs/diffuse.hpp>
namespace Miyuki {
	namespace Core {
		class DiffuseBSDFImpl : public BSDFImpl {
			Float roughness;
			Spectrum R;
		public:
			DiffuseBSDFImpl(Float roughness, Spectrum R) :BSDFImpl(BSDFLobe(EReflection | EDiffuse)) {}

			virtual void sample(
				BSDFSample& sample
			)const override {
				if (roughness >= 0.0f) {
					LambertianReflection(R).sample(sample);
				}
				else {
					OrenNayarReflection(R, roughness).sample(sample);
				}
			}

			// evaluate bsdf according to wo, wi
			virtual Spectrum evaluate(
				const Vec3f& wo,
				const Vec3f& wi,
				BSDFSampleOption option,
				BSDFLobe lobe = BSDFLobe::EAll
			)const override {
				if (roughness >= 0.0f) {
					return LambertianReflection(R).evaluate(wo,wi);
				}
				else {
					return OrenNayarReflection(R, roughness).evaluate(wo, wi);
				}
			}

			// evaluate pdf according to wo, wi
			virtual Float evaluatePdf(
				const Vec3f& wo,
				const Vec3f& wi,
				BSDFSampleOption option,
				BSDFLobe lobe = BSDFLobe::EAll
			)const override {
				if (roughness >= 0.0f) {
					return LambertianReflection(R).evaluatePdf(wo, wi);
				}
				else {
					return OrenNayarReflection(R, roughness).evaluatePdf(wo, wi);
				}
			}
		};

		BSDFImpl* DiffuseMaterial::createBSDF(BSDFCreationContext& ctx)const {
			Float _roughness = roughness->eval(ctx.shadingPoint).toFloat();
			Spectrum R = color->eval(ctx.shadingPoint).toVec3f();
			auto bsdf = ctx.alloc<DiffuseBSDFImpl>(_roughness, R);
			return bsdf;
		}

	}
}