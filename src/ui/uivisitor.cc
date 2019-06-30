#include <ui/uivisitor.h>
#include <ui/mykui.h>
#include <graph/graph.h>
#include <graph/materialnode.h>
#include <utils/log.h>
#include <graph/meshnode.h>

namespace Miyuki {
	namespace GUI {
		class TypeSelector {
			std::unordered_map<std::string, const Reflection::Class*>_map;
			std::unordered_map<const Reflection::Class*, std::string>_invmap;
			std::unordered_map<const Reflection::Class*, std::function<Reflection::Object*(Reflection::Runtime*)>> _ctors;
			std::vector<std::string> _list;
		public:
			template<class T>
			TypeSelector& option(const std::string& s) {
				auto c = T::__classinfo__();
				
				_map[s] = c;
				_invmap[c] = s;
				_list.push_back(s);
				_ctors[c] = [](Reflection::Runtime* runtime)->Reflection::Object * {
					return runtime->New<T>();
				};
				return *this;
			}

			template<class T>
			Optional<T*> select(Reflection::Runtime* runtime,const std::string&label, const Reflection::Object* current) {
				Optional<T*> opt = {};
				auto currentTy = &current->getClass();
				Combo().name(label).item(_invmap.at(currentTy)).with(true, [=, &opt]()
				{					
					for (auto ty : _list) {
						bool is_selected = _map.at(ty) == currentTy;;
						SingleSelectableText().name(ty).selected(is_selected).with(true, [=, &opt]() {
							if (currentTy != _map.at(ty)) {
								opt = static_cast<T*>(_ctors[_map.at(ty)](runtime));
							}
							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}).show();
					}
				}).show();
				return opt;
			}

		};
		TypeSelector materialSelector;
		void initSelector() {
			materialSelector
				.option<Graph::DiffuseMaterialNode>("Diffuse Material")
				.option<Graph::GlossyMaterialNode>("Glossy Material")
				.option<Graph::MixedMaterialNode>("Mixed Material");
		}
		void UIVisitor::visitSelected() {
			if (!selected)return;
			if (selected->isDerivedOf(Graph::MaterialNode::__classinfo__())) {
				auto mat = static_cast<Graph::MaterialNode*>(selected);
				auto name = mat->getName();
				if (auto r = GetInput("name", name)) {
					mat->name->setValue(r.value());
				}
				if (auto _new = materialSelector.select<Graph::MaterialNode>(runtime, "Type", selected)) {
					for (auto mesh : *graph->meshes) {
						for (auto object : *mesh->objects) {
							if (object->material == selected) {
								object->material = _new.value();
							}
						}
					}
					selected = _new.value();
					auto matNew = static_cast<Graph::MaterialNode*>(selected);
					matNew->name = mat->name;
				}
				visit(selected);
			}
			
		}

		void UIVisitor::init() {
			initSelector();
			visit<Graph::Graph>([=](Graph::Graph* node)->void {
				graph = node;
				TreeNode().name("Materials").with(true, [=]() {
					auto materials = node->materials;
					selectedMaterialIndex = -1;
					int index = 0;
					for (auto material : *materials) {
						if (!material) {
							continue;
						}
						const auto& name = material->getName();
						SingleSelectableText().name(name).selected(material == selected)
							.with(true, [=]() {
							selected = material;
							selectedMaterialIndex = index;
						}).show();
						index++;
					}
				}).show();

				TreeNode().name("Meshes").with(true, [=]() {
					auto meshes = node->meshes;
					for (auto mesh : *meshes) {
						auto name = mesh->name;
						int flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
						if (mesh == selected)
							flags |= ImGuiTreeNodeFlags_Selected;
						TreeNode().name(name->getValue()).flag(flags).with(true, [=]() {
							if (ImGui::IsItemClicked()) {
								selected = mesh;
							}
							for (auto object : *mesh->objects) {
								if (!object)
									continue;
								SingleSelectableText().name(object->getName())
									.selected(object == selected)
									.with(true, [=]() {
									selected = object;
								}).show();
							}
						}).with(false, [=]() {
							if (ImGui::IsItemClicked()) {
								selected = mesh;
							}
						}).show();
					}
				}).show();
			});
			visit<Graph::GlossyMaterialNode>([=](Graph::GlossyMaterialNode* node) {
				
			});
			visit<Graph::DiffuseMaterialNode>([=](Graph::DiffuseMaterialNode* node) {
			
			});
			visit<Graph::MixedMaterialNode>([=](Graph::MixedMaterialNode* node) {

				visit(static_cast<Graph::MaterialNode*>(node->matA.object));
				visit(static_cast<Graph::MaterialNode*>(node->matB.object));
			});
			visit<Graph::MeshNode>([=](Graph::MeshNode* node) {
				auto name = node->name->getValue();
				if (auto r = GetInput("name", name)) {
					node->name->setValue(r.value());
				}
			});
			visit<Graph::ObjectNode>([=](Graph::ObjectNode* node) {
				auto objectName = node->objectName->getValue();
				auto matName = node->material->getName();
				if (auto r = GetInput("name", objectName)) {
					node->objectName->setValue(r.value());
				}
				/*if (auto r = GetInput("material", matName)) {
					bool success;
					for (auto m : *graph->materials) {
						if (m->getName() == r.value()) {
							success = true;
						}
					}
				}*/
				Combo().name("material").item(matName).with(true, [=]()
				{
					for (auto m : *graph->materials) {
						bool is_selected = (m == node->material.object);
						SingleSelectableText().name(m->getName()).selected(is_selected).with(true, [=]() {
							node->material = m;
							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}).show();
					}
				}).show();
			});
		}
	}
}