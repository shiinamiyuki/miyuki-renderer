#pragma once


#include <engine/renderengine.h>
#include <imgui/imgui.h>

namespace Miyuki {
	namespace GUI {
		class UIVisitor : public Reflection::TraitVisitor{
			Trait* selected = nullptr;

			void visitMaterialAndSelect(Box<Core::Material>& material, const std::string& label);
			void visitShaderAndSelect(Box<Core::Shader>& shader, const std::string& label);
			enum SelectedNodeType {
				kMaterial,
				kMesh,
				kObject
			};
			int selectedMaterialIndex = -1;
			SelectedNodeType selectedNodeType;
			using Base = Reflection::TraitVisitor;
		public:
			RenderEngine* engine = nullptr;
			void visitGraph();
			void init();
			UIVisitor() {
				init();
			}
			void reset() {
				selected = nullptr;
			}
			void visitSelected();
		};
	}
}