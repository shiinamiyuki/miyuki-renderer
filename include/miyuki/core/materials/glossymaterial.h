#pragma once

#include <core/materials/material.h>
namespace Miyuki {
	namespace Core {
		class GlossyMaterial final : public Material {
		public:
			MYK_CLASS(GlossyMaterial);
			Box<Shader> color = nullptr;
			Box<Shader> roughness = nullptr;
			int model = 0;
			virtual BSDFComponent* createBSDF(BSDFCreationContext&)const override;
			virtual Spectrum evalAlbedo(ShadingPoint& p)const override;
		};
		MYK_IMPL(GlossyMaterial, "Material.Glossy");
		MYK_REFL(GlossyMaterial, (Material), (color)(roughness)(model));

	}
}