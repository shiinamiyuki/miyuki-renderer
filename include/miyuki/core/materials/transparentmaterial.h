#pragma once
#include <core/materials/material.h>

namespace Miyuki {
	namespace Core {
		class TransparentMaterial final : public Material {
		public:
			Box<Shader> color;
			MYK_CLASS(TransparentMaterial);
			virtual BSDFComponent* createBSDF(BSDFCreationContext&)const override;
			virtual Spectrum evalAlbedo(ShadingPoint& p)const override;
		};
		MYK_IMPL(TransparentMaterial, "Material.Transparent");
		MYK_REFL(TransparentMaterial, (Material), (color));
	}
}