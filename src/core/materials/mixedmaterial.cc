#include <core/materials/mixedmaterial.h>
#include <core/bsdfs/bsdf.h>

namespace Miyuki {
	namespace Core {
		class MixedBSDFImpl : public BSDFImpl {
			BSDFImpl* A;
			BSDFImpl* B;
			Float fraction;

			void combine(Float frac, BSDFSample& sample, BSDFImpl* first, BSDFImpl* other)const {
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
			MixedBSDFImpl(Float fraction, BSDFImpl* A, BSDFImpl* B)
				:BSDFImpl(BSDFLobe(A->getLobe() | B->getLobe())), A(A), B(B), fraction(fraction) {}

			virtual void sample(
				BSDFSample& sample
			)const {
				BSDFImpl* first, * second;
				Float frac;
				if (sample.uPick < fraction) {
					sample.uPick /= fraction;
					first = A;
					second = B;
					frac = fraction;
				}
				else {
					sample.uPick = (sample.uPick - fraction) / (1.0f - fraction);
					first = B;
					second = A;
					frac = 1.0f - fraction;
				}
				first->sample(sample);
				if (B->match(sample.lobe)) {
					combine(frac, sample, first, second);
				}
			}

			// evaluate bsdf according to wo, wi
			virtual Spectrum evaluate(
				const Vec3f& wo,
				const Vec3f& wi,
				BSDFSampleOption option,
				BSDFLobe lobe = BSDFLobe::EAll
			)const {
				Float sum = 0.0f;
				Vec3f f;
				if (A->match(lobe)) {
					f += A->evaluate(wo, wi, option, lobe);
					sum += fraction;
				}
				if (B->match(lobe)) {
					f += B->evaluate(wo, wi, option, lobe);
					sum += 1.0f - fraction;
				}
				return sum == 0.0f ? f : f / sum;
			}

			// evaluate pdf according to wo, wi
			virtual Float evaluatePdf(
				const Vec3f& wo,
				const Vec3f& wi,
				BSDFSampleOption option,
				BSDFLobe lobe = BSDFLobe::EAll
			)const {
				Float sum = 0.0f;
				Float pdf = 0.0f;
				if (A->match(lobe)) {
					pdf += A->evaluatePdf(wo, wi, option, lobe);
					sum += fraction;
				}
				if (B->match(lobe)) {
					pdf += B->evaluatePdf(wo, wi, option, lobe);
					sum += 1.0f - fraction;
				}
				return sum == 0.0f ? pdf : pdf / sum;
			}
		};

		BSDFImpl* MixedMaterial::createBSDF(BSDFCreationContext& ctx)const {
			auto frac = fraction->eval(ctx.shadingPoint).toFloat();
			auto A = matA->createBSDF(ctx); 
			auto B = matB->createBSDF(ctx);
			return ctx.alloc<MixedBSDFImpl>(frac, A, B);
		}
	}
}