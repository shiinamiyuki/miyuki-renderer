#pragma once

#include <reflection.h>
#include <core/materials/material.h>
#include <utils/file.hpp>
#include <core/cameras/camera.h>
#include <core/integrators/integrator.h>

namespace Miyuki {
	namespace Core {
		struct MaterialSlot final : Component {
			MYK_IMPL(MaterialSlot);
			std::string name;
			Box<Material> material;
		};


		struct Object final : Component {
			MYK_IMPL(Object);
			std::string name;
			MaterialSlot* material = nullptr;
		};

		struct MeshFile final : Component {
			MYK_IMPL(MeshFile);
			Transform transform;
			std::string name;
			File file;
			std::vector<Box<Object>> objects;
		};

		
		struct Graph final : Component {
			MYK_IMPL(Graph);
			std::vector<Box<MaterialSlot>> materials;
			std::vector<Box<MeshFile>> meshes;
			std::vector<Box<Camera>> cameras;
			Box<Integrator> integrator;
			Camera* activeCamera = nullptr;
		};
	}
}

MYK_REFL(Miyuki::Core::MaterialSlot, (name)(material))
MYK_REFL(Miyuki::Core::Object, (name)(material))
MYK_REFL(Miyuki::Core::Graph, (materials)(meshes)(cameras)(activeCamera)(integrator))
MYK_REFL(Miyuki::Core::MeshFile, (file)(name)(transform)(objects))