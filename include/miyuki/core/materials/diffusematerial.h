#pragma once

#include <core/materials/material.h>

namespace Miyuki {
	namespace Core {
		class DiffuseMaterial final : public Material {
		public:
			MYK_CLASS(DiffuseMaterial);
			Box<Shader> emissionShader;
			Box<Shader> color;
			Box<Shader> roughness;
			virtual Shader* emission()const {
				return emissionShader.get();
			}
			virtual BSDF* createBSDF(BSDFCreationContext&)const override;
		};
		MYK_IMPL(DiffuseMaterial, "Material.Diffuse");
		MYK_REFL(DiffuseMaterial, (Material), (emissionShader)(color)(roughness));
	}
}