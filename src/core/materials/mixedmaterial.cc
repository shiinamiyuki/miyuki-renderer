#include <core/materials/mixedmaterial.h>
#include <core/bsdfs/bsdf.h>
#include <core/bsdfs/mixedbsdf.h>

namespace Miyuki {
	namespace Core {
		

		BSDFComponent* MixedMaterial::createBSDF(BSDFCreationContext& ctx)const {
			auto frac = Shader::evaluate(fraction, ctx.shadingPoint).toFloat();
			auto A = matA->createBSDF(ctx);
			auto B = matB->createBSDF(ctx);
			return ctx.alloc<MixedBSDF>(frac, A, B);
		}

		Spectrum MixedMaterial::evalAlbedo(ShadingPoint& p)const {
			auto frac = Shader::evaluate(fraction, p).toFloat();
			return frac * matA->evalAlbedo(p) + (1 - frac)* matB->evalAlbedo(p);
		}
	}
}