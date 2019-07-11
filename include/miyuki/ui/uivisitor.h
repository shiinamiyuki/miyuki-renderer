#pragma once


#include <engine/renderengine.h>
#include <imgui/imgui.h>
#include <boost/signals2.hpp>

namespace Miyuki {
	namespace GUI {
		class UIVisitor : public Reflection::ComponentVisitor{
			Component* selected = nullptr;

			void visitMaterialAndSelect(Box<Core::Material>& material, const std::string& label);
			void visitShaderAndSelect(Box<Core::Shader>& shader, const std::string& label);
			enum SelectedNodeType {
				kMaterial,
				kMesh,
				kObject
			};
			int selectedMaterialIndex = -1;
			SelectedNodeType selectedNodeType;
			using Base = Reflection::ComponentVisitor;
			boost::signals2::connection connection;
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
			~UIVisitor();
		};
	}
}