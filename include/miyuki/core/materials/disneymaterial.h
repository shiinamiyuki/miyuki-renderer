#pragma once

#include <core/materials/material.h>

namespace Miyuki {
	namespace Core {
		class DisneyMaterial : public Material {
		public:
			MYK_CLASS(DisneyMaterial);
			Box<Shader> baseColor;
			Box<Shader> subsurface;
			Box<Shader> subsurfaceColor;
			Box<Shader> metallic;
			Box<Shader> specular;
			Box<Shader> specularTint;
			Box<Shader> roughness;
			Box<Shader> anisotropic;
			Box<Shader> anisotropicRotation;
			Box<Shader> sheen;
			Box<Shader> sheenTint;
			Box<Shader> clearCoat;
			Box<Shader> clearCoatRoughness;
			Box<Shader> IOR;
			Box<Shader> transmission;
			Box<Shader> transmissionRoughness;
			virtual BSDFComponent* createBSDF(BSDFCreationContext&)const;
		};
		MYK_IMPL(DisneyMaterial, "Material.Disney");
		MYK_REFL(DisneyMaterial, (Material),
			(baseColor)
			(subsurface)
			(subsurfaceColor)
			(metallic)
			(specular)
			(specularTint)
			(roughness)
			(anisotropic)
			(anisotropicRotation)
			(sheen)
			(sheenTint)
			(clearCoat)
			(clearCoatRoughness)
			(IOR)
			(transmission)
			(transmissionRoughness)
		);
	}
}