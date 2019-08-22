#include <core/materials/glossymaterial.h>
#include <core/bsdfs/bsdf.h>
#include <core/bsdfs/microfacet.hpp>
#include <core/bsdfs/fresnel.hpp>

namespace Miyuki {
	namespace Core {
		

		BSDFComponent* GlossyMaterial::createBSDF(BSDFCreationContext& ctx)const {
			auto _roughness = Shader::evaluate(roughness, ctx.shadingPoint).toFloat();
			auto _color = Shader::evaluate(color, ctx.shadingPoint).toVec3f();
			if (_roughness == 0.0f) {
				return ctx.alloc<SpecularBSDFImpl>(_color);
			}
			else
				return ctx.alloc<GlossyBSDF>(_color, _roughness, (MicrofacetType)model);
		}
		Spectrum GlossyMaterial::evalAlbedo(ShadingPoint& p)const {
			return Shader::evaluate(color, p).toVec3f();
		}
	}
}