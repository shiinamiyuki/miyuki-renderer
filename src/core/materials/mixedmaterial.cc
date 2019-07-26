#include <core/materials/mixedmaterial.h>
#include <core/bsdfs/bsdf.h>

namespace Miyuki {
	namespace Core {
		class MixedBSDFImpl : public BSDF {
			BSDF* A;
			BSDF* B;
			Float fraction;

			void combine(Float frac, BSDFSample& sample, BSDF* first, BSDF* other)const {
				if (first->isDelta()) {
					sample.f *= frac;
					sample.pdf *= frac;
				}
				else {
					auto f = other->evaluate(sample.wo, sample.wi, sample.option);
					auto pdf = other->evaluatePdf(sample.wo, sample.wi, sample.option);
					sample.f = frac * sample.f + (1.0f - frac) * f;
					sample.pdf = frac * sample.pdf + (1.0f - frac) * pdf;
				}
			}
		public:
			MixedBSDFImpl(Float fraction, BSDF* A, BSDF* B)
				:BSDF(BSDFLobe(A->getLobe() | B->getLobe())), A(A), B(B), fraction(fraction) {}

			virtual void sample(
				BSDFSample& sample
			)const {
				if (sample.uPick < fraction) {
					sample.uPick /= fraction;
					A->sample(sample);
					combine(fraction, sample, A, B);
				}
				else {
					sample.uPick = (sample.uPick - fraction) / (1.0f - fraction);
					B->sample(sample);
					combine(1.0f - fraction, sample, B, A);
				}
			}

			// evaluate bsdf according to wo, wi
			virtual Spectrum evaluate(
				const Vec3f& wo,
				const Vec3f& wi,
				BSDFSampleOption option,
				BSDFLobe lobe = BSDFLobe::EAll
			)const {
				return fraction * A->evaluate(wo, wi, option, lobe)
					+ (1.0f - fraction) * B->evaluate(wo, wi, option, lobe);
			}

			// evaluate pdf according to wo, wi
			virtual Float evaluatePdf(
				const Vec3f& wo,
				const Vec3f& wi,
				BSDFSampleOption option,
				BSDFLobe lobe = BSDFLobe::EAll
			)const {
				return fraction * A->evaluatePdf(wo, wi, option, lobe)
					+ (1.0f - fraction) * B->evaluatePdf(wo, wi, option, lobe);
			}
		};

		BSDF* MixedMaterial::createBSDF(BSDFCreationContext& ctx)const {
			auto frac = fraction->eval(ctx.shadingPoint).toFloat();
			auto A = matA->createBSDF(ctx); 
			auto B = matB->createBSDF(ctx);
			return ctx.alloc<MixedBSDFImpl>(frac, A, B);
		}
	}
}