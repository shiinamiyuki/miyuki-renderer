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
			std::string name;
			File file;
		};
		
	
		struct Graph final  :  Trait {
			MYK_IMPL(Graph);
			std::vector<Box<Material>> materials;
		};
		
	}
}
MYK_REFL(Miyuki::Core::Object, (name)(material))
MYK_REFL(Miyuki::Core::Graph, (materials))
MYK_REFL(Miyuki::Core::MeshFile, (file)(name))