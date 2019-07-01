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

		void UIVisitor::init() {
			visit<Core::GlossyMaterial>([=](Core::GlossyMaterial* node) {

			});
			visit<Core::DiffuseMaterial>([=](Core::DiffuseMaterial* node) {

			});
			visit<Core::MixedMaterial>([=](Core::MixedMaterial* node) {
				visit(static_cast<Trait*>(node->matA.get()));
				visit(static_cast<Trait*>(node->matB.get()));
			});
			visit<Core::MeshFile>([=](Core::MeshFile* node) {
				auto name = node->name;
				if (auto r = GetInput("name", name)) {
					node->name = r.value();
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
						SingleSelectableText().name(getMaterialName(m.get())).selected(is_selected).with(true, [=,&m]() {
							node->material = m.get();
							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}).show();
					}
				}).show();
			});
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