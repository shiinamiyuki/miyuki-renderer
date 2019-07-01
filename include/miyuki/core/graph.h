#pragma once

#include <reflection.h>
#include <core/materials/material.h>
#include <utils/file.hpp>

namespace Miyuki {	
	namespace Core {
		struct Object final : Trait {
			MYK_IMPL(Object);
			std::string name;
			Material* material;
		};	

		struct MeshFile final : Trait {
			MYK_IMPL(MeshFile);
			Transform transform;
			std::string name;
			File file;
			std::vector<Box<Object>> objects;
		};		
	
		struct Graph final  :  Trait {
			MYK_IMPL(Graph);
			std::vector<Box<Material>> materials;
			std::vector<Box<MeshFile>> meshFiles;
			void addMaterial(Box<Material> mat) {
				materials.emplace_back(std::move(mat));
			}
		};		
	}
}
MYK_REFL(Miyuki::Core::Object, (name)(material))
MYK_REFL(Miyuki::Core::Graph, (materials)(meshFiles))
MYK_REFL(Miyuki::Core::MeshFile, (file)(name)(transform)(objects))