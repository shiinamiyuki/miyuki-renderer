#include <core/scene.h>

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
		}

		void Scene::loadObjMeshAndInstantiate(
			const std::string& filename, const std::string& meshName, const Transform& T) {
			loadObjMesh(filename);
			instantiateMesh(filename, meshName, T);
		}

		void Scene::instantiateMesh(const std::string& filename, const std::string& meshName, const Transform& T) {
			CHECK(meshes.find(filename) != meshes.end());
			auto mesh = meshes[filename]->instantiate(meshName, T);
			embreeScene->addMesh(mesh, (int)instances.size());
			for (const auto& name : mesh->names) {
				mesh->materials.push_back(materialAssignment.at(name));
			}
			instances.emplace_back(mesh);
		}

		struct Scene::Visitor : Reflection::TraitVisitor {
			using Base = Reflection::TraitVisitor;
			Scene& scene;
			Visitor(Scene& scene) :scene(scene) {
			}
			void loadMaterials(Core::Graph& graph) {
				for (auto& slot : graph.materials) {
					auto name = slot->name;
					scene.materials[name] = slot->material.get();
				}
			}
			void loadImages(Core::Graph& graph) {
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
				visit<Core::FloatShader>([=](Core::FloatShader *) {});
				visit<Core::RGBShader>([=](Core::RGBShader*) {});
				auto& loader = scene.imageLoader;
				visit<Core::ImageTextureShader>([=, &loader](Core::ImageTextureShader* shader) {
					shader->texture = loader->load(shader->imageFile);
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
		};

		void Scene::commit(Core::Graph& graph) {
			Visitor visitor(*this);
			visitor.loadMaterials(graph);
			visitor.loadMeshes(graph);
			visitor.loadImages(graph);
			embreeScene->commit();
		}
	}
}