#pragma once

#include <reflection.h>
#include <core/materials/material.h>
#include <utils/file.hpp>
#include <core/cameras/camera.h>
#include <core/integrators/integrator.h>

namespace Miyuki {
	namespace Core {
		struct MaterialSlot final : Reflective {
			MYK_META;
			std::string name;
			Box<Material> material;
		};
		MYK_IMPL(MaterialSlot, (Reflective), "Core.MaterialSlot");
		MYK_REFL(MaterialSlot, (name)(material));
		

		struct Object final : Reflective {
			MYK_META;
			std::string name;
			MaterialSlot* material = nullptr;
		};

		MYK_IMPL(Object, (Reflective), "Core.Object");
		MYK_REFL(Object, (name)(material));

		struct MeshFile final : Reflective {
			MYK_META;
			Transform transform;
			std::string name;
			File file;
			std::vector<Box<Object>> objects;
		};
		MYK_IMPL(MeshFile, (Reflective), "Core.MeshFile");
		MYK_REFL(MeshFile, (file)(name)(transform)(objects));

		struct FilmConfig final :Reflective {
			MYK_META;
			Point2i dimension;
			Float scale;
		};
		MYK_IMPL(FilmConfig, (Reflective), "Core.FilmConfig");
		MYK_REFL(FilmConfig, (scale)(dimension));

		struct Graph final : Reflective {
			MYK_META;
			std::vector<Box<MaterialSlot>> materials;
			std::vector<Box<MeshFile>> meshes;
			std::vector<Box<Camera>> cameras;
			Box<Integrator> integrator;
			FilmConfig filmConfig;
			Camera* activeCamera = nullptr;
		};
		MYK_IMPL(Graph, (Reflective), "Core.Graph");
		MYK_REFL(Graph, (materials)(meshes)(cameras)(activeCamera)(integrator));
	}
}

