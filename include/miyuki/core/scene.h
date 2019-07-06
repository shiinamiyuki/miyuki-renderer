#ifndef MIYUKI_SCENE_H
#define MIYUKI_SCENE_H
#include <reflection.h>
#include <core/materials/material.h>
#include <core/embreescene.h>
#include <core/mesh.h>
#include <core/graph.h>
#include <io/imageloader.h>

namespace Miyuki {
	namespace Core {
		class Scene {
			std::unique_ptr<EmbreeScene> embreeScene;
			std::unordered_map<std::string, Core::Material*> materials;
			std::unordered_map<std::string, Core::Material*> materialAssignment;
			std::vector<std::shared_ptr<Mesh>> instances;
			std::map<std::string, std::shared_ptr<Mesh>> meshes;
			std::unique_ptr<IO::ImageLoader> imageLoader;
			void loadObjMesh(const std::string& filename);

			void loadObjMeshAndInstantiate(const std::string& filename, const std::string& meshName,
				const Transform& T = Transform());

			void instantiateMesh(const std::string& filename, const std::string& meshName, const Transform&);
			class Visitor;
		public:
			Scene();
			void commit(Core::Graph&);
		};
	}
}


#endif