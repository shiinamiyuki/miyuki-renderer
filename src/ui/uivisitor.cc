#include <ui/uivisitor.h>
#include <ui/mykui.h>

namespace Miyuki {
	namespace GUI {
		std::string getMaterialName(Core::Material* material) {
			std::string name = "unknown";
			Reflection::match(material)
				.with<Core::MixedMaterial>([&](Core::MixedMaterial* mat) {
				name = mat->name;
			}).with<Core::DiffuseMaterial>([&](Core::DiffuseMaterial* mat) {
				name = mat->name;
			}).with<Core::GlossyMaterial>([&](Core::GlossyMaterial* mat) {
				name = mat->name;
			});
			return name;
		}

		void setMaterialName(Core::Material* material, const std::string& name) {
			Reflection::match(material)
				.with<Core::MixedMaterial>([&](Core::MixedMaterial* mat) {
				mat->name = name;
			}).with<Core::DiffuseMaterial>([&](Core::DiffuseMaterial* mat) {
				mat->name = name;
			}).with<Core::GlossyMaterial>([&](Core::GlossyMaterial* mat) {
				mat->name = name;
			});
		}

		struct TypeSelector {
			std::unordered_map<std::string, const Reflection::TypeInfo*>_map;
			std::unordered_map<const Reflection::TypeInfo*, std::string>_invmap;
			std::unordered_map<const Reflection::TypeInfo*, std::function<Box<Trait>(void)>> _ctors;
			std::vector<std::string> _list;
			template<class T>
			TypeSelector& option(const std::string& s) {
				auto c = T::type();

				_map[s] = c;
				_invmap[c] = s;
				_list.push_back(s);
				_ctors[c] = [](void) -> Box<Trait> {
					return Reflection::make_box<T>();
				};
				return *this;
			}
			template<class T>
			std::optional<Box<T>> select(const std::string& label, const T* current) {
				std::optional<Box<T>> opt = {};
				auto itemName = !current ? "Empty" : _invmap.at(current->typeInfo());
				Combo().name(label).item(itemName).with(true, [=, &opt]()
				{
					SingleSelectableText().name("Empty").selected(current == nullptr).with(true, [=, &opt]() {
						if (current) {
							opt = { nullptr };
						}
						if (!current) {
							ImGui::SetItemDefaultFocus();
						}
					}).show();
				
					for (auto ty : _list) {
						auto currentTy = current ? current->typeInfo() : nullptr;
						bool is_selected = _map.at(ty) == currentTy;
						SingleSelectableText().name(ty).selected(is_selected).with(true, [=, &opt]() {
							if (currentTy != _map.at(ty)) {
								opt = std::move(
									Reflection::static_unique_ptr_cast<T>(
										_ctors[_map.at(ty)]()));
							}
							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}).show();
					}
				}).show();
				return opt;
			}
		};

		std::optional<Box<Core::Material>> selectMaterial(Core::Material* material) {
			static TypeSelector selector;
			static std::once_flag flag;
			std::call_once(flag, [&]() {
				selector.option<Core::DiffuseMaterial>("diffuse shader")
					.option<Core::GlossyMaterial>("glossy shader")
					.option<Core::MixedMaterial>("mixed shader");
			});
			return selector.select<Core::Material>("material type, material", material);
		}

		std::optional<Box<Core::Shader>> selectShader(Core::Shader* shader) {
			static TypeSelector selector;
			static std::once_flag flag;
			std::call_once(flag, [&]() {
				selector.option<Core::FloatShader>("float")
					.option<Core::RGBShader>("RGB")
					.option<Core::ImageTextureShader>("Image Texture");
			});
			return selector.select<Core::Shader>("shader type", shader);
		}

		void UIVisitor::init() {
			visit<Core::FloatShader>([=](Core::FloatShader* shader) {
				auto value = shader->getValue();
				if (auto r = GetInput("value", value)) {
					shader->setValue(r.value());
				}
			});
			visit<Core::RGBShader>([=](Core::RGBShader* shader) {
				Spectrum value = shader->getValue();
				if (auto r = GetInput("color", value)) {
					shader->setValue(r.value());
				}
			});
			visit<Core::ImageTextureShader>([=](Core::ImageTextureShader* shader) {
				Text().name(shader->imageFile.path.string()).show();
			});
			visit<Core::GlossyMaterial>([=](Core::GlossyMaterial* node) {
				visitShaderAndSelect(node->color);
				visitShaderAndSelect(node->roughness);
			});
			visit<Core::DiffuseMaterial>([=](Core::DiffuseMaterial* node) {
				visitShaderAndSelect(node->color);
				visitShaderAndSelect(node->roughness);
			});
			visit<Core::MixedMaterial>([=](Core::MixedMaterial* node) {
				visitShaderAndSelect(node->fraction);
				Text().name("material A").show();
				visitMaterialAndSelect(node->matA);
				Separator().show();
				Text().name("material B").show();
				visitMaterialAndSelect(node->matB);
				Separator().show();
			});
			visit<Core::MeshFile>([=](Core::MeshFile* node) {
				auto name = node->name;
				if (auto r = GetInput("name", name)) {
					node->name = r.value();
				}
				if (auto r = GetInput("transform", node->transform)) {
					node->transform = r.value();
				}
			});
			visit<Core::Object>([=](Core::Object* node) {
				auto graph = engine->getGraph();
				auto objectName = node->name;
				auto matName = getMaterialName(node->material);
				if (auto r = GetInput("name", objectName)) {
					node->name = r.value();
				}
				Combo().name("material").item(matName).with(true, [=]()
				{
					for (auto& m : graph->materials) {
						bool is_selected = (m.get() == node->material);
						SingleSelectableText().name(getMaterialName(m.get())).selected(is_selected).with(true, [=, &m]() {
							node->material = m.get();
							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}).show();
					}
				}).show();
			});
		}

		void UIVisitor::visitMaterialAndSelect(Box<Core::Material>& material) {
			const auto& name = getMaterialName(material.get());
			auto graph = engine->getGraph();
			if (auto r = selectMaterial(material.get())) {
				auto tmp = std::move(r.value());
				for (auto& mesh : graph->meshes) {
					for (auto& object : mesh->objects) {
						if (object->material == material.get()) {
							object->material = tmp.get();
						}
					}
				}
				material = std::move(tmp);
				setMaterialName(material.get(), name);
			}
			visit(material);
		}
		void UIVisitor::visitShaderAndSelect(Box<Core::Shader>& shader) {
			if (auto r = selectShader(shader.get())) {
				shader = std::move(shader);
			}
			visit(shader);
		}

		void UIVisitor::visitSelected() {
			if(selected)
				visit(selected);
		}

		void UIVisitor::visitGraph() {
			auto graph = engine->getGraph();
			if (!graph)return;
			TreeNode().name("Materials").with(true, [=]() {
				auto& materials = graph->materials;
				int index = 0;
				for (auto& material : materials) {
					if (!material) {
						continue;
					}
					const auto& name = getMaterialName(material.get());
					SingleSelectableText().name(name).selected(material.get() == selected)
						.with(true, [=, &material]() {
						selected = material.get();
						selectedNodeType = kMaterial;
					}).show();
					index++;
				}
			}).show();

			TreeNode().name("Meshes").with(true, [=]() {
				auto& meshes = graph->meshes;
				for (auto& mesh : meshes) {
					auto name = mesh->name;
					int flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
					if (mesh.get() == selected)
						flags |= ImGuiTreeNodeFlags_Selected;
					TreeNode().name(name).flag(flags).with(true, [=, &mesh]() {
						if (ImGui::IsItemClicked()) {
							selected = mesh.get();
						}
						for (auto& object : mesh->objects) {
							if (!object)
								continue;
							SingleSelectableText().name(object->name)
								.selected(object.get() == selected)
								.with(true, [=, &object]() {
								selected = object.get();
								selectedNodeType = kObject;
							}).show();
						}
					}).with(false, [=, &mesh]() {
						if (ImGui::IsItemClicked()) {
							selected = mesh.get();
							selectedNodeType = kMesh;
						}
					}).show();
				}
			}).show();
		}
	}
}