#ifndef MIYUKI_SCENE_H
#define MIYUKI_SCENE_H
#include <reflection.h>
#include <core/materials/material.h>
#include <core/embreescene.h>
#include <core/mesh.h>
#include <core/graph.h>
#include <io/imageloader.h>
#include <math/distribution.h>

namespace Miyuki {
	namespace Core {
		class Light;
		class Scene {
			std::unique_ptr<EmbreeScene> embreeScene;
			std::unordered_map<std::string, Core::Material*> materials;
			std::unordered_map<std::string, Core::Material*> materialAssignment;
			std::vector<std::shared_ptr<Mesh>> instances;
			std::map<std::string, std::shared_ptr<Mesh>> meshes;
			std::map<std::string, uint32_t> meshToId;
			std::unique_ptr<IO::ImageLoader> imageLoader;
			std::unique_ptr<Distribution1D> lightDistribution;
			std::vector<Box<Light>> lights;
			std::unordered_map<Light*, Float> lightPdfMap;
			void loadObjMesh(const std::string& filename);

			void loadObjMeshAndInstantiate(const std::string& filename, const std::string& meshName,
				const Transform& T = Transform());

			void instantiateMesh(const std::string& filename, const std::string& meshName, const Transform&);
			struct Visitor;

			
			void postIntersect(Intersection*);

			void assignMaterial(std::shared_ptr<Mesh> mesh);

			void computeLightDistribution();
			std::atomic<size_t> rayCounter;
			Light* light = nullptr;
		public:
			void setEnvironmentLight(Light* light) {
				this->light = light;
			}
			Light* getEnvironmentLight()const { return light; }
			const Distribution1D& getLightDistribution()const {
				return *lightDistribution;
			}
			const std::vector<Box<Light>>& getLights()const {
				return lights;
			}
			const std::unordered_map<Light*, Float>& getLightPdfMap()const {
				return lightPdfMap;
			}
			Mesh* getMeshById(uint32_t id) {
				return instances[id].get();
			}
			Scene();
			void commit(Core::Graph&);
			void resetRayCount() { rayCounter = 0; }
			size_t getRayCount()const { return rayCounter; }
			bool intersect(const Ray& ray, Intersection* isct) {
				rayCounter++;
				if (embreeScene->intersect(ray, isct)) {
					postIntersect(isct);
					return true;
				}
				return false;
			}
		};
	}
}


#endif