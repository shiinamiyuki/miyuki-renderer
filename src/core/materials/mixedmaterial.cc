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
	}
}