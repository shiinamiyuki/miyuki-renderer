#include <core/materials/diffusematerial.h>
#include <core/bsdfs/bsdf.h>
#include <core/bsdfs/diffuse.hpp>

namespace Miyuki {
	namespace Core {

		BSDFComponent* DiffuseMaterial::createBSDF(BSDFCreationContext& ctx)const {
			Float _roughness = Shader::evaluate(roughness, ctx.shadingPoint).toFloat();
			Spectrum R = Shader::evaluate(color, ctx.shadingPoint).toVec3f();
			auto bsdf = ctx.alloc<DiffuseBSDF>(_roughness, R);
			return bsdf;
		}
		Spectrum DiffuseMaterial::evalAlbedo(ShadingPoint& p)const {
			return Shader::evaluate(color, p).toVec3f();
		}
	}
}