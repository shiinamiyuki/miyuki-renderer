#ifndef MIYUKI_MATERIAL_H
#define MIYUKI_MATERIAL_H
#include <miyuki.h>
#include <math/transform.h>
#include <core/intersection.hpp>
#include <core/shaders/shader.h>
namespace Miyuki {
	namespace Core {

		enum BSDFLobe {
			ENone = 0,
			EDiffuse = 1 << 0,
			EGlossy = 1 << 2,
			ESpecular = 1 << 3,
			EReflection = 1 << 4,
			ETransmission = 1 << 5,
			EAll = EDiffuse | ESpecular | EGlossy | EReflection | ETransmission,
			EAllButSpecular = EAll & ~ESpecular
		};

		class Material : public Reflective {
		public:
			MYK_INTERFACE(Material);
			virtual BSDFLobe lobe()const = 0;
			virtual Shader* emission()const = 0;
		};
		MYK_REFL(Material, (Reflective), MYK_REFL_NIL);

		class MixedMaterial final : public Material {
		public:
			MYK_CLASS(MixedMaterial);
			Box<Material> matA = nullptr, matB = nullptr;
			Box<Shader> emissionShader = nullptr;
			Box<Shader> fraction = nullptr;
			virtual BSDFLobe lobe()const override{
				return static_cast<BSDFLobe>(matA->lobe() | matB->lobe());
			}
			virtual Shader* emission()const {
				return emissionShader.get();
			}
		};
		MYK_IMPL(MixedMaterial,  "Material.Mixed");
		MYK_REFL(MixedMaterial, (Material), (emissionShader)(fraction)(matA)(matB));

		class DiffuseMaterial final  : public Material {
		public:
			MYK_CLASS(DiffuseMaterial);
			Box<Shader> emissionShader;
			Box<Shader> color;
			Box<Shader> roughness;
			virtual BSDFLobe lobe()const override {
				return BSDFLobe(EDiffuse | EReflection);
			}
			virtual Shader* emission()const {
				return emissionShader.get();
			}
		};
		MYK_IMPL(DiffuseMaterial, "Material.Diffuse");
		MYK_REFL(DiffuseMaterial, (Material), (emissionShader)(color)(roughness));

		class GlossyMaterial final : public Material {
		public:
			MYK_CLASS(GlossyMaterial);
			Box<Shader> emissionShader = nullptr;
			Box<Shader> color = nullptr;
			Box<Shader> roughness = nullptr;
			virtual BSDFLobe lobe()const override {
				return BSDFLobe(EGlossy | EReflection);
			}
			virtual Shader* emission()const {
				return emissionShader.get();
			}
		};
		MYK_IMPL(GlossyMaterial,  "Material.Glossy");
		MYK_REFL(GlossyMaterial, (Material), (emissionShader)(color)(roughness));
	}
}

#endif