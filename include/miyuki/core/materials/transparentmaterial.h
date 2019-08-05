#pragma once
#include <core/materials/material.h>

namespace Miyuki {
	namespace Core {
		class TransparentMaterial final : public Material {
		public:
			Box<Shader> color;
			MYK_CLASS(TransparentMaterial);
			virtual BSDFImpl* createBSDF(BSDFCreationContext&)const override;
		};
		MYK_IMPL(TransparentMaterial, "Material.Transparent");
		MYK_REFL(TransparentMaterial, (Material), (color));
	}
}