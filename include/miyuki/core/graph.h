#pragma once

#include <reflection.h>
#include <core/materials/material.h>
#include <core/medium/medium.h>
#include <utils/file.hpp>
#include <core/cameras/camera.h>
#include <core/integrators/integrator.h>
#include <core/samplers/sampler.h>
#include <core/lights/light.h>

namespace Miyuki {
	namespace Core {
		struct MaterialSlot final : Reflective {
			MYK_META;
			std::string name;
			Box<Material> material;
			Box<Medium> medium;
		};
		MYK_IMPL(MaterialSlot, "Core.MaterialSlot");
		MYK_REFL(MaterialSlot, (Reflective), (name)(material)(medium));

		struct Object final : Reflective {
			MYK_META;
			std::string name;

			// the name that is shown, and is modifiable
			std::string alias;
			MaterialSlot* material = nullptr;
		};

		MYK_IMPL(Object, "Core.Object");
		MYK_REFL(Object, (Reflective), (name)(alias)(material));

		struct MeshFile final : Reflective {
			MYK_META;
			Transform transform;
			std::string name;
			File file;
			std::vector<Box<Object>> objects;
		};
		MYK_IMPL(MeshFile, "Core.MeshFile");
		MYK_REFL(MeshFile, (Reflective), (file)(name)(transform)(objects));

		struct FilmConfig final :Reflective {
			MYK_META;
			Point2i dimension;
			Float scale;
			File outputImage;
		};
		MYK_IMPL(FilmConfig, "Core.FilmConfig");
		MYK_REFL(FilmConfig, (Reflective), (scale)(dimension)(outputImage));

		struct WorldConfig final :Reflective {
			MYK_CLASS(WorldConfig);
			Box<Light> environmentMap;
			Float rayBias = 0.01f;
		};
		MYK_IMPL(WorldConfig, "Core.WorldConfig");
		MYK_REFL(WorldConfig, (Reflective), (environmentMap)(rayBias));

		class LightCollection : public CachedPreprocessable,public Reflective {
		public:
			void doPreprocess() {}
			MYK_CLASS(LightCollection);
		};
		MYK_IMPL(LightCollection, "Core.LightCollection");
		MYK_REFL(LightCollection, (Reflective), MYK_REFL_NIL);

		struct Graph final : Reflective {
			MYK_META;
			std::vector<Box<MaterialSlot>> materials;
			std::vector<Box<MeshFile>> meshes;
			std::vector<Box<Camera>> cameras;
			Box<Integrator> integrator;
			Box<Sampler> sampler;
			FilmConfig filmConfig;
			Camera* activeCamera = nullptr;
			WorldConfig worldConfig;
			Box<LightCollection> lights;
		};
		MYK_IMPL(Graph, "Core.Graph");
		MYK_REFL(Graph, (Reflective), (materials)(meshes)(cameras)(activeCamera)
			(integrator)(sampler)(filmConfig)(worldConfig)(lights));
	}
}

