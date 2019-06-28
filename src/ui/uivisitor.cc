#include <ui/uivisitor.h>
#include <ui/mykui.h>
#include <graph/graph.h>
#include <graph/materialnode.h>
#include <graph/meshnode.h>

namespace Miyuki {
	namespace GUI {
		class TypeSelector {
			std::unordered_map<std::string, const Reflection::Class*>_map;
		public:
			TypeSelector& select(const std::string& s, const Reflection::Class* c) {
				_map[s] = c;
				return *this;
			}
		};
		void UIVisitor::init() {
			visit<Graph::Graph>([=](Graph::Graph* node)->void {
				TreeNode().name("Materials").with(true, [=]() {
					auto materials = node->materials;
					for (auto material :*materials) {
						if (!material) {
							continue;
						}
						const auto& name = material->getName();
						SingleSelectableText().name(name).selected(material == selected)
							.with(true, [=]() {
							selected = material;
						}).show();
					}
				}).show();
			
				TreeNode().name("Meshes").with(true, [=]() {
					auto meshes = node->meshes;
					for (auto mesh : *meshes) {
						auto name = mesh->name;
						int flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
						if(mesh == selected)
							flags |= ImGuiTreeNodeFlags_Selected;
						TreeNode().name(name->getValue()).flag(flags).with(true, [=]() {
							if (ImGui::IsItemClicked()) {
								selected = mesh;
							}
							for (auto object : *mesh->objects) {
								if (!object)
									continue;
								SingleSelectableText().name(object->materialName->getValue())
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
			visit<Graph::MaterialNode>([=](Graph::MaterialNode* node) {

			});
			visit<Graph::MeshNode>([=](Graph::MeshNode* node) {

			});
			visit<Graph::ObjectNode>([=](Graph::ObjectNode * node) {

			});
		}
	}
}