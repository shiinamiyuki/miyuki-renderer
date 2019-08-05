#include <core/scene.h>
#include <utils/log.h>

#include <core/materials/mixedmaterial.h>
#include <core/materials/diffusematerial.h>
#include <core/materials/glossymaterial.h>
#include <core/materials/transparentmaterial.h>
#include <core/lights/area.h>
#include <core/lights/infinite.h>


namespace Miyuki {
	namespace Core {
		Scene::Scene() :rayCounter(0) {
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
				visit<Core::AreaLight>([=](Core::AreaLight* light) {

				});
				visit<Core::InfiniteAreaLight>([=](Core::InfiniteAreaLight* light) {
					visit(light->shader);
				});
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
				visit<Core::TransparentMaterial>([=](Core::TransparentMaterial* mat) {
					visit(mat->color);
				});
				visit<Core::ScaledShader> ([=](Core::ScaledShader* shader) {
					visit(shader->shader);
					visit(shader->scale);
				});
				visit<Core::MixedShader>([=](Core::MixedShader* shader) {
					visit(shader->fraction);
					visit(shader->shaderA);
					visit(shader->shaderB);
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
					if (p)
						p->preprocess();
				}
			}
		};

		void Scene::computeLightDistribution() {
			for (auto& instance : instances) {
				instance->clearLightSetup();
			}
			lights.clear();
			meshLights.clear();
			if(getEnvironmentLight())
				lights.emplace_back(getEnvironmentLight());
			for (auto instance : instances) {
				for (auto& primitive : instance->primitives) {
					auto mat = primitive.material();
					if (mat->emission) {
						if (mat->emission->average().toVec3f().max() >= 1e-3f) {
							auto light = makeBox<AreaLight>(&primitive);
							primitive.setLight(light.get());
							lights.emplace_back(light.get());
							meshLights.emplace_back(std::move(light));
						}
					}
				}
			}

			lightPdfMap.clear();
			lightDistribution = nullptr;

			if (lights.empty()) {
				return;
			}
			std::vector<Float> power(lights.size());
			for (int i = 0; i < lights.size(); i++) {
				power[i] = lights[i]->power();
			}
			lightDistribution = std::make_unique<Distribution1D>(&power[0], lights.size());

			for (int i = 0; i < lights.size(); i++) {
				lightPdfMap[lights[i]] = lightDistribution->pdfDiscrete(i);
			}

			Log::log("Important lights: {} Total power: {}\n", lights.size(), lightDistribution->funcInt);
		}

		void Scene::commit(Core::Graph& graph) {
			Visitor visitor(*this);
			visitor.loadMaterials(graph);
			visitor.loadMeshes(graph);
			visitor.loadImages(graph);
			for (auto& i : instances) {
				assignMaterial(i);
			}			
	
			if (graph.worldConfig.environmentMap) {
				visitor.visit(graph.worldConfig.environmentMap);
			}
			setEnvironmentLight(graph.worldConfig.environmentMap.get());
			visitor.preprocessAll();
			embreeScene->commit();
			auto bound = embreeScene->getWorldBound();
			Point3f center;
			Float radius;
			bound.boundingSphere(&center, &radius);
			if (auto light = Reflection::cast<InfiniteAreaLight>(getEnvironmentLight())) {
				light->setWorldRadius(radius);
			}
			computeLightDistribution();
		}

		
	}
}