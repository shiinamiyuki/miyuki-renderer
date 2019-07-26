#pragma once

#include <core/materials/material.h>

namespace Miyuki {
	namespace Core {
		class GlossyMaterial final : public Material {
		public:
			MYK_CLASS(GlossyMaterial);
			Box<Shader> emissionShader = nullptr;
			Box<Shader> color = nullptr;
			Box<Shader> roughness = nullptr;
			virtual Shader* emission()const {
				return emissionShader.get();
			}
			virtual BSDF* createBSDF(BSDFCreationContext&)const override;
		};
		MYK_IMPL(GlossyMaterial, "Material.Glossy");
		MYK_REFL(GlossyMaterial, (Material), (emissionShader)(color)(roughness));

	}
}