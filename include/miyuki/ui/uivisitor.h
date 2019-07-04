#pragma once


#include <engine/renderengine.h>

namespace Miyuki {
	namespace GUI {
		class UIVisitor : public Reflection::TraitVisitor{
			Trait* selected = nullptr;

			void visitMaterialAndSelect(Box<Core::Material>& material);
			void visitShaderAndSelect(Box<Core::Shader>& shader);
			enum SelectedNodeType {
				kMaterial,
				kMesh,
				kObject
			};
			SelectedNodeType selectedNodeType;
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