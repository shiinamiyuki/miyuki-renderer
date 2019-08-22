#include <core/materials/transparentmaterial.h>
#include <core/bsdfs/bsdf.h>
#include <core/bsdfs/trignometry.hpp>
namespace Miyuki {
	namespace Core {
		class TransparentBSDF : public BSDFComponent {
			const Vec3f R;
		public:
			TransparentBSDF(const Vec3f& R) :R(R), BSDFComponent(BSDFLobe(ESpecular | ETransmission)) {}
			virtual void sample(
				BSDFEvaluationContext& ctx,
				BSDFSample& sample
			)const override {
				sample.wi = -sample.wo;
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

		BSDFComponent* TransparentMaterial::createBSDF(BSDFCreationContext& ctx)const {
			auto _color = Shader::evaluate(color, ctx.shadingPoint).toVec3f();
			return ctx.alloc<TransparentBSDF>(_color);
		}
		Spectrum TransparentMaterial::evalAlbedo(ShadingPoint& p)const {
			return Shader::evaluate(color, p).toVec3f();
		}
	}
}