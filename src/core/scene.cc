#include <core/scene.h>
#include <utils/log.h>

#include <core/materials/mixedmaterial.h>
#include <core/materials/diffusematerial.h>
#include <core/materials/glossymaterial.h>


namespace Miyuki {
	namespace Core {
		Scene::Scene() {
			embreeScene = std::make_unique<EmbreeScene>();
		}
		void Scene::loadObjMesh(const std::string& filename) {
			if (meshes.find(filename) != meshes.end())
				return;
			auto mesh = std::make_shared<Mesh>(filename);
			meshes[filename] = mesh;
			Log::log("Loaded {}\n", filename);
		}

		void Scene::loadObjMeshAndInstantiate(
			const std::string& filename, const std::string& meshName, const Transform& T) {
			loadObjMesh(filename);
			instantiateMesh(filename, meshName, T);
		}

		void Scene::instantiateMesh(const std::string& filename, const std::string& meshName, const Transform& T) {
			if (meshToId.find(meshName) != meshToId.end()) {
				Log::log("Reading from mesh data {}\n", meshName);
				return;
			}
			CHECK(meshes.find(filename) != meshes.end());
			auto mesh = meshes[filename]->instantiate(meshName, T);
			auto id = (uint32_t)instances.size();
			embreeScene->addMesh(mesh, id);
			meshToId[meshName] = id;

			instances.emplace_back(mesh);
		}

		void Scene::assignMaterial(std::shared_ptr<Mesh> mesh) {
			mesh->materials.clear();
			for (const auto& name : mesh->names) {
				mesh->materials.push_back(materialAssignment.at(name));
			}
		}
		struct Scene::Visitor : Reflection::ComponentVisitor {
			std::vector<Preprocessable*> preprocessables;
			using Base = Reflection::ComponentVisitor;
			Scene& scene;
			Visitor(Scene& scene) :scene(scene) {
			}
			void loadMaterials(Core::Graph& graph) {
				for (auto& slot : graph.materials) {
					auto name = slot->name;
					scene.materials[name] = slot->material.get();
				}
			}
			template<class T>
			std::enable_if_t<std::is_base_of_v<Reflective, T>, void>
				visit(const std::function<void(T*)>& f) {
				ComponentVisitor::visit<T>(f);
			}

			template<class T>
			std::enable_if_t<std::is_base_of_v<Preprocessable, T>, void> visit(T* p) {
				preprocessables.emplace_back(p);
				ComponentVisitor::visit(p);
			}
			template<class T>
			std::enable_if_t<!std::is_base_of_v<Preprocessable, T>, void> visit(T* p) {
				ComponentVisitor::visit(p);
			}

			template<class T>
			std::enable_if_t<std::is_base_of_v<Reflective, T>, void> visit(Box<T>& p) {
				visit<T>(p.get());
			}

			void loadImages(Core::Graph& graph) {
				if (!scene.imageLoader)
					scene.imageLoader = std::make_unique<IO::ImageLoader>();
				Base::_map.clear();
				visit<Core::GlossyMaterial>([=](Core::GlossyMaterial* mat) {
					visit(mat->color);
					visit(mat->roughness);
				});
				visit<Core::DiffuseMaterial>([=](Core::DiffuseMaterial* mat) {
					visit(mat->color);
					visit(mat->roughness);
				});
				visit<Core::MixedMaterial>([=](Core::MixedMaterial* mat) {
					visit(mat->matA);
					visit(mat->matB);
				});
				visit<Core::FloatShader>([=](Core::FloatShader* shader) {

				});
				visit<Core::RGBShader>([=](Core::RGBShader* shader) {

				});
				auto& loader = scene.imageLoader;
				visit<Core::ImageTextureShader>([=, &loader](Core::ImageTextureShader* shader) {
					shader->texture = Texture(loader->load(shader->imageFile));
				});
				for (auto& material : graph.materials) {
					visit(material->material);
				}
			}
			void loadMeshes(Core::Graph& graph) {
				for (auto& mesh : graph.meshes) {
					for (auto& object : mesh->objects) {
						scene.materialAssignment[object->name] = object->material->material.get();
					}
					scene.loadObjMeshAndInstantiate(mesh->file.fullpath().string(), mesh->name, mesh->transform);
				}
			}

			void preprocessAll() {
				for (auto p : preprocessables) {
					if(p)
						p->preprocess();
				}
			}
		};

		void Scene::commit(Core::Graph& graph) {
			Visitor visitor(*this);
			visitor.loadMaterials(graph);
			visitor.loadMeshes(graph);
			visitor.loadImages(graph);
			for (auto& i : instances) {
				assignMaterial(i);
			}
			visitor.preprocessAll();
			embreeScene->commit();
		}

		static inline Float mod(Float a, Float b) {
			int k = a / b;
			Float x = a - k * b;
			if (x < 0)
				x += b;
			if (x >= b)
				x -= b;
			return x;
		}

		void Scene::postIntersect(Intersection* isct) {
			isct->primitive = &instances[isct->geomId]->primitives[isct->primId];
			auto p = isct->primitive;
			isct->Ns = p->Ns(isct->uv);
			isct->Ng = p->Ng();
			isct->computeLocalFrame(isct->Ns);

			auto uv = PointOnTriangle(isct->primitive->textureCoord[0],
				isct->primitive->textureCoord[1],
				isct->primitive->textureCoord[2],
				isct->uv[0],
				isct->uv[1]);
			uv.x() = mod(uv.x(), 1);
			uv.y() = mod(uv.y(), 1);
			isct->textureUV = uv;

			// TODO: partial derivatives
		}
	}
}