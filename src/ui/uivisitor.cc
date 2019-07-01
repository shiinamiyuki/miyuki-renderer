#include <ui/uivisitor.h>
#include <ui/mykui.h>

namespace Miyuki {
	namespace GUI {
		void UIVisitor::init() {

		}
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
						.with(true, [=,&material]() {
						selected = material.get();
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
							}).show();
						}
					}).with(false, [=, &mesh]() {
						if (ImGui::IsItemClicked()) {
							selected = mesh.get();
						}
					}).show();
				}
			}).show();
		}
	}
}