#pragma once

#include <core/bsdfs/bsdf.h>

namespace Miyuki {
	namespace Core {
		class MixedBSDF : public BSDFComponent {
			BSDFComponent* A;
			BSDFComponent* B;
			Float fraction;

			void combine(BSDFEvaluationContext& ctx, Float frac, BSDFSample& sample, BSDFComponent* first, BSDFComponent* other)const {
				if (first->isDelta() || other->isDelta()) {
					sample.f *= frac;
					sample.pdf *= frac;
				}
				else {
					auto f = BSDFComponent::evaluate(other, ctx);
					auto pdf = BSDFComponent::evaluatePdf(other, ctx);
					sample.f = frac * sample.f + (1.0f - frac) * f;
					sample.pdf = frac * sample.pdf + (1.0f - frac) * pdf;
				}
			}
		public:
			MixedBSDF(Float fraction, BSDFComponent* A, BSDFComponent* B)
				:BSDFComponent(BSDFLobe(A->getLobe() | B->getLobe())), A(A), B(B), fraction(fraction) {}

			virtual void sample(
				BSDFEvaluationContext& ctx,
				BSDFSample& sample
			)const {
				//A->sample(sample);
				BSDFComponent* first, * second;
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
				return fraction * BSDFComponent::evaluate(A, ctx) + (1.0f - fraction) * BSDFComponent::evaluate(B, ctx);
			}

			// evaluate pdf according to wo, wi
			virtual Float evaluatePdf(
				const BSDFEvaluationContext& ctx
			)const {
				return fraction * BSDFComponent::evaluatePdf(A, ctx) + (1.0f - fraction) * BSDFComponent::evaluatePdf(B, ctx);
			}
		};
	}
}