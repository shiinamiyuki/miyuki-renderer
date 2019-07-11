#ifndef MIYUKI_MATERIAL_H
#define MIYUKI_MATERIAL_H
#include <miyuki.h>
#include <math/transform.h>
#include <core/intersection.hpp>
#include <core/shaders/shader.h>
namespace Miyuki {
	namespace Core {

		enum BSDFLobe {
			kNone = 0,
			kDiffuse = 1 << 0,
			kGlossy = 1 << 2,
			kSpecular = 1 << 3,
			kReflection = 1 << 4,
			kTransmission = 1 << 5,
			kAll = kDiffuse | kSpecular | kGlossy | kReflection | kTransmission,
			kAllButSpecular = kAll & ~kSpecular
		};

		struct Material : public Component {
			virtual BSDFLobe lobe()const = 0;
			virtual Shader* emission()const = 0;
		};

		struct MixedMaterial final : public Material {
			MYK_IMPL(MixedMaterial)
			Box<Material> matA = nullptr, matB = nullptr;
			Box<Shader> emissionShader = nullptr;
			Box<Shader> fraction = nullptr;
		//	MYK_IMPL(MixedMaterial);
			virtual BSDFLobe lobe()const override{
				return static_cast<BSDFLobe>(matA->lobe() | matB->lobe());
			}
			virtual Shader* emission()const {
				return emissionShader.get();
			}
		};

		struct DiffuseMaterial final  : public Material {
			MYK_IMPL(DiffuseMaterial)
			Box<Shader> emissionShader;
			Box<Shader> color;
			Box<Shader> roughness;
			virtual BSDFLobe lobe()const override {
				return BSDFLobe(kDiffuse | kReflection);
			}
			virtual Shader* emission()const {
				return emissionShader.get();
			}
		};

		struct GlossyMaterial final : public Material {
			MYK_IMPL(GlossyMaterial)
			Box<Shader> emissionShader = nullptr;
			Box<Shader> color = nullptr;
			Box<Shader> roughness = nullptr;
			virtual BSDFLobe lobe()const override {
				return BSDFLobe(kGlossy | kReflection);
			}
			virtual Shader* emission()const {
				return emissionShader.get();
			}
		};
	}
}
MYK_REFL(Miyuki::Core::MixedMaterial, (emissionShader)(fraction)(matA)(matB))
MYK_REFL(Miyuki::Core::DiffuseMaterial, (emissionShader)(color)(roughness))
MYK_REFL(Miyuki::Core::GlossyMaterial,(emissionShader)(color)(roughness))
#endif