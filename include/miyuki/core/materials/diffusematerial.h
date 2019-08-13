#pragma once

#include <core/materials/material.h>

namespace Miyuki {
	namespace Core {
		class DiffuseMaterial final : public Material {
		public:
			MYK_CLASS(DiffuseMaterial);
			Box<Shader> color = nullptr;
			Box<Shader> roughness = nullptr;
			virtual BSDFImpl* createBSDF(BSDFCreationContext&)const override;
		};
		MYK_IMPL(DiffuseMaterial, "Material.Diffuse");
		MYK_REFL(DiffuseMaterial, (Material), (color)(roughness));
	}
}