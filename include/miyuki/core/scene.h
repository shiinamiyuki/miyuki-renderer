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
			std::vector<Box<Light>> meshLights;
			std::vector<Light*> lights;
			std::unordered_map<Light*, Float> lightPdfMap;
			void loadObjMesh(const std::string& filename);

			void loadObjMeshAndInstantiate(const std::string& filename, const std::string& meshName,
				const Transform& T = Transform());

			void instantiateMesh(const std::string& filename, const std::string& meshName, const Transform&);
			struct Visitor;

			
			

			void assignMaterial(std::shared_ptr<Mesh> mesh);

			void computeLightDistribution();
			std::atomic<size_t> rayCounter;
			Light* light = nullptr;
		public:
			inline void postIntersect(Intersection*);
			void setEnvironmentLight(Light* light) {
				this->light = light;
			}
			Light* getEnvironmentLight()const { return light; }
			const Distribution1D& getLightDistribution()const {
				return *lightDistribution;
			}
			const std::vector<Light*>& getLights()const {
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
			void intersect4(const Ray4& ray, Intersection4* isct) {
				rayCounter += 4;
				embreeScene->intersect4(ray, isct);
				for (int i = 0; i < 4; i++) {
					if (ray.rays[i].valid() && isct->isct[i].hit()) {
						postIntersect(&isct->isct[i]);
					}
				}
			}
			void intersect8(const Ray8& ray, Intersection8* isct) {
				rayCounter += 8;
				embreeScene->intersect8(ray, isct);
				for (int i = 0; i < 8; i++) {
					if (ray.rays[i].valid() && isct->isct[i].hit()) {
						postIntersect(&isct->isct[i]);
					}
				}
			}
			Bound3f getWorldBound()const {
				return embreeScene->getWorldBound();
			}
		};

		static inline Float mod(Float a, Float b) {
			int k = a / b;
			Float x = a - k * b;
			if (x < 0)
				x += b;
			if (x >= b)
				x -= b;
			return x;
		}

		inline void Scene::postIntersect(Intersection* isct) {
			isct->primitive = &instances[isct->geomId]->getPrimitives()[isct->primId];
			auto p = isct->primitive;
			
			

			auto uv = PointOnTriangle(isct->primitive->textureCoord[0],
				isct->primitive->textureCoord[1],
				isct->primitive->textureCoord[2],
				isct->uv[0],
				isct->uv[1]);
			uv.x = mod(uv.x, 1);
			uv.y = mod(uv.y, 1);

			auto mat = p->material();
			isct->Ns = p->Ns(isct->uv);
			isct->computeLocalFrame(isct->Ns);
			if (mat && mat->normalMap) {
				auto perturb = Shader::evaluate(mat->normalMap, ShadingPoint(uv)).toVec3f();
				perturb = 2 * perturb - Vec3f(1);
				auto Ns = Vec3f(0, 0, 1);
				Ns += perturb;
				Ns.normalize();
				isct->Ns = isct->localToWorld(Ns);
				isct->computeLocalFrame(isct->Ns);
			}

			isct->Ng = p->Ng();
			
			isct->textureUV = uv;

			// TODO: partial derivatives
		}
	}
}


#endif