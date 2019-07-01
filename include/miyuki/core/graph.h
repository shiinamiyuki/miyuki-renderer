#pragma once

#include <reflection.h>
#include <core/materials/material.h>
#include <utils/file.hpp>
namespace Miyuki {
	
	namespace Core {
		struct Object final : Trait {
		//	MYK_IMPL(Object);
			std::string name;
			Material* material;
		};

	

		struct MeshFile final : Trait {
		//	MYK_IMPL(MeshFile);
			std::string name;
			std::string relativePath;
		};
		
	
		struct Graph final  :  Trait {
		//	MYK_IMPL(Graph);
			std::vector<Box<Material>> materials;
		};
		
	}
}
//MYK_REFL(Miyuki::Core::MeshFile, (relativePath)(name))