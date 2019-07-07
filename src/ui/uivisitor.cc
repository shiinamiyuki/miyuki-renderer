#include <ui/uivisitor.h>
#include <ui/mykui.h>

namespace Miyuki {
	namespace GUI {

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

		std::optional<Box<Core::Material>> selectMaterial(Core::Material* material, const std::string& label) {
			static TypeSelector selector;
			static std::once_flag flag;
			std::call_once(flag, [&]() {
				selector.option<Core::DiffuseMaterial>("diffuse material")
					.option<Core::GlossyMaterial>("glossy material")
					.option<Core::MixedMaterial>("mixed material");
			});
			return selector.select<Core::Material>(label, material);
		}

		std::optional<Box<Core::Shader>> selectShader(Core::Shader* shader, const std::string& label) {
			static TypeSelector selector;
			static std::once_flag flag;
			std::call_once(flag, [&]() {
				selector.option<Core::FloatShader>("float")
					.option<Core::RGBShader>("RGB")
					.option<Core::ImageTextureShader>("Image Texture");
			});
			return selector.select<Core::Shader>(label, shader);
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
				if (auto r = GetInput("value", value)) {
					shader->setValue(r.value());
				}
			});
			visit<Core::ImageTextureShader>([=](Core::ImageTextureShader* shader) {
				Text().name(shader->imageFile.path.string()).show();
			});
			visit<Core::GlossyMaterial>([=](Core::GlossyMaterial* node) {
				visitShaderAndSelect(node->color, "color");
				visitShaderAndSelect(node->roughness, "roughness");
			});
			visit<Core::DiffuseMaterial>([=](Core::DiffuseMaterial* node) {
				visitShaderAndSelect(node->color, "color");
				visitShaderAndSelect(node->roughness, "roughness");
			});
			visit<Core::MixedMaterial>([=](Core::MixedMaterial* node) {
				visitShaderAndSelect(node->fraction,"fraction");
				visitMaterialAndSelect(node->matA,"material A");
				Separator().show();
				visitMaterialAndSelect(node->matB,"material B");
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
				auto matName = node->material->name;
				if (auto r = GetInput("name", objectName)) {
					node->name = r.value();
				}
				
				LineText("material");

				Combo().name("use material").item(matName).with(true, [=]()
				{
					for (auto& slot : graph->materials) {
						auto& m = slot->material;
						bool is_selected = (node->material && m.get() == node->material->material.get());
						SingleSelectableText().name(slot->name).selected(is_selected).with(true, [=, &m,&slot]() {
							node->material = slot.get();
							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}).show();
					}
				}).show();

				for (auto& slot : engine->getGraph()->materials) {
					auto& material = slot->material;
					if (material == node->material->material) {
						visitMaterialAndSelect(slot->material, "type");
						break;
					}
				}
			});
			visit<Core::MaterialSlot>([=](Core::MaterialSlot* slot) {
				if (auto r = GetInput("name", slot->name)) {
					slot->name = r.value();
				}
				visit(slot->material);
			});
		}

		void UIVisitor::visitMaterialAndSelect(Box<Core::Material>& material, const std::string& label) {
			ImGui::PushID(&material);
			if (auto r = selectMaterial(material.get(), label)) {
				material = std::move(r.value());
			}
			visit(material);
			ImGui::PopID();
		}

		void UIVisitor::visitShaderAndSelect(Box<Core::Shader>& shader, const std::string& label) {
			ImGui::PushID(&shader);
			if (auto r = selectShader(shader.get(),label)) {
				shader = std::move(r.value());
			}
			visit(shader);
			ImGui::PopID();
		}

		void UIVisitor::visitSelected() {
			if (selected) {
				visit(selected);				
			}
		}

		void UIVisitor::visitGraph() {
			auto graph = engine->getGraph();
			if (!graph)return;
			TreeNode().name("Materials").with(true, [=]() {
				auto& materials = graph->materials;
				int index = 0;
				for (auto& slot : materials) {
					auto& material = slot->material;
					const auto& name = slot->name;
					SingleSelectableText().name(name).selected(material.get() == selected)
						.with(true, [=, &material,&slot]() {
						selected = slot.get();
						selectedNodeType = kMaterial;
						selectedMaterialIndex = index;
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
							selectedNodeType = kMesh;
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