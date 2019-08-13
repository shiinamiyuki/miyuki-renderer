#include <core/materials/mixedmaterial.h>
#include <core/bsdfs/bsdf.h>

namespace Miyuki {
	namespace Core {
		class MixedBSDFImpl : public BSDFImpl {
			BSDFImpl* A;
			BSDFImpl* B;
			Float fraction;

			void combine(BSDFEvaluationContext& ctx, Float frac, BSDFSample& sample, BSDFImpl* first, BSDFImpl* other)const {
				if (first->isDelta() || other->isDelta()) {
					sample.f *= frac;
					sample.pdf *= frac;
				}
				else {
					auto f = BSDFImpl::evaluate(other, ctx);
					auto pdf = BSDFImpl::evaluatePdf(other, ctx);
					sample.f = frac * sample.f + (1.0f - frac) * f;
					sample.pdf = frac * sample.pdf + (1.0f - frac) * pdf;
				}
			}
		public:
			MixedBSDFImpl(Float fraction, BSDFImpl* A, BSDFImpl* B)
				:BSDFImpl(BSDFLobe(A->getLobe() | B->getLobe())), A(A), B(B), fraction(fraction) {}

			virtual void sample(
				BSDFEvaluationContext& ctx,
				BSDFSample& sample
			)const {
				//A->sample(sample);
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

				first->sample(ctx, sample);
				CHECK(ctx.isAssigned());
				if (second->match(sample.lobe)) {
					combine(ctx, frac, sample, first, second);
				}
			}

			// evaluate bsdf according to wo, wi
			virtual Spectrum evaluate(
				const BSDFEvaluationContext& ctx
			)const {
				return fraction * BSDFImpl::evaluate(A, ctx) + (1.0f - fraction) * BSDFImpl::evaluate(B, ctx);
			}

			// evaluate pdf according to wo, wi
			virtual Float evaluatePdf(
				const BSDFEvaluationContext& ctx
			)const {
				return fraction * BSDFImpl::evaluatePdf(A, ctx) + (1.0f - fraction) * BSDFImpl::evaluatePdf(B, ctx);
			}
		};

		BSDFImpl* MixedMaterial::createBSDF(BSDFCreationContext& ctx)const {
			auto frac = Shader::evaluate(fraction, ctx.shadingPoint).toFloat();
			auto A = matA->createBSDF(ctx);
			auto B = matB->createBSDF(ctx);
			return ctx.alloc<MixedBSDFImpl>(frac, A, B);
		}
	}
}