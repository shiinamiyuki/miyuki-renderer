#pragma once

#include <core/materials/material.h>

namespace Miyuki {
	namespace Core {
		class MixedMaterial final : public Material {
		public:
			MYK_CLASS(MixedMaterial);
			Box<Material> matA = nullptr, matB = nullptr;
			Box<Shader> emissionShader = nullptr;
			Box<Shader> fraction = nullptr;
			virtual Shader* emission()const {
				return emissionShader.get();
			}
			virtual BSDF* createBSDF(BSDFCreationContext&)const override;
		};
		MYK_IMPL(MixedMaterial, "Material.Mixed");
		MYK_REFL(MixedMaterial, (Material), (emissionShader)(fraction)(matA)(matB));
	}
}