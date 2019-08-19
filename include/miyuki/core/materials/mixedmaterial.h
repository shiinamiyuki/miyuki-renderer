#pragma once

#include <core/materials/material.h>

namespace Miyuki {
	namespace Core {
		class MixedMaterial final : public Material {
		public:
			MYK_CLASS(MixedMaterial);
			Box<Material> matA = nullptr, matB = nullptr;
			Box<Shader> fraction = nullptr;
			virtual BSDFComponent* createBSDF(BSDFCreationContext&)const override;
		};
		MYK_IMPL(MixedMaterial, "Material.Mixed");
		MYK_REFL(MixedMaterial, (Material), (fraction)(matA)(matB));
	}
}