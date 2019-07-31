#include <core/bsdfs/bsdf.h> 
#include <core/bsdfs/trignometry.hpp>
namespace Miyuki {
	namespace Core {
		
		//shows an annoying purple color
		class DefaultBSDFImpl : public BSDFImpl {
		public:
			DefaultBSDFImpl() :BSDFImpl(EAll) {}
			// Inherited via BSDFImpl
			virtual void sample(BSDFEvaluationContext& ctx,BSDFSample& sample) const override {
				sample.wi = sample.wo;
				sample.f = Spectrum(1, 0, 1) / AbsCosTheta(sample.wi);
			}

			virtual Spectrum evaluate(
				const BSDFEvaluationContext&) const override {
				return Spectrum();
			}
			virtual Float evaluatePdf(const BSDFEvaluationContext&) const override {
				return 0;
			}
		};

		BSDFImpl* getDefaultBSDFImpl() {
			std::once_flag flag;
			DefaultBSDFImpl* bsdf = nullptr;
			std::call_once(flag, [&]() {
				bsdf = new DefaultBSDFImpl();
			});
			return bsdf;
		}
		void BSDF::sample(
			BSDFSample& sample
		)const {
			BSDFEvaluationContext ctx(*this, sample.wo, EAll, sample.option);
			impl->sample(ctx, sample);
			
		}

		// evaluate bsdf according to wo, wi
		Spectrum BSDF::evaluate(
			const Vec3f& wo,
			const Vec3f& wi,
			BSDFSampleOption option,
			BSDFLobe lobe
		)const {			
			BSDFEvaluationContext ctx(*this, wo, lobe, option);
			ctx.assignWi(wi);
			return BSDFImpl::evaluate(impl, ctx);
		}

		// evaluate pdf according to wo, wi
		Float BSDF::evaluatePdf(
			const Vec3f& wo,
			const Vec3f& wi,
			BSDFSampleOption option,
			BSDFLobe lobe
		)const {
			BSDFEvaluationContext ctx(*this, wo, lobe, option);
			ctx.assignWi(wi);
			return BSDFImpl::evaluatePdf(impl, ctx);
		}
		Spectrum BSDFImpl::evaluate(BSDFImpl* bsdf, const BSDFEvaluationContext& ctx) {
			bool reflectStrict = Vec3f::dot(ctx.woW(), ctx.Ng()) * Vec3f::dot(ctx.wiW(), ctx.Ng()) > 0;
			bool reflect = SameHemisphere(ctx.wi(), ctx.wo() );
			if (bsdf->match(ctx.lobe)) {

				return bsdf->evaluate(ctx);
			}
			return {};
		}

		Float BSDFImpl::evaluatePdf(BSDFImpl* bsdf, const BSDFEvaluationContext& ctx) {
			if (bsdf->match(ctx.lobe))
				return bsdf->evaluatePdf(ctx);
			else
				return {};
		}
	}
}