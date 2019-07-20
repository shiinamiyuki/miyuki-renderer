#pragma once

#include <reflection.h>
#include <core/materials/material.h>
#include <utils/file.hpp>
#include <core/cameras/camera.h>
#include <core/integrators/integrator.h>

namespace Miyuki {
	namespace Core {
		struct MaterialSlot final : Component {
			MYK_META(MaterialSlot);
			std::string name;
			Box<Material> material;
		};
		MYK_IMPL(MaterialSlot, Component, "Core.MaterialSlot");
		MYK_REFL(MaterialSlot, (name)(material));
		

		struct Object final : Component {
			MYK_META(Object);
			std::string name;
			MaterialSlot* material = nullptr;
		};

		MYK_IMPL(Object, Component, "Core.Object");
		MYK_REFL(Object, (name)(material));

		struct MeshFile final : Component {
			MYK_META(MeshFile);
			Transform transform;
			std::string name;
			File file;
			std::vector<Box<Object>> objects;
		};
		MYK_IMPL(MeshFile, Component, "Core.MeshFile");
		MYK_REFL(MeshFile, (file)(name)(transform)(objects));

		struct Graph final : Component {
			MYK_META(Graph);
			std::vector<Box<MaterialSlot>> materials;
			std::vector<Box<MeshFile>> meshes;
			std::vector<Box<Camera>> cameras;
			Box<Integrator> integrator;
			Camera* activeCamera = nullptr;
		};
		MYK_IMPL(Graph, Component, "Core.Graph");
		MYK_REFL(Graph, (materials)(meshes)(cameras)(activeCamera)(integrator));
	}
}

