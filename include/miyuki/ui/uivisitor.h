#pragma once


#include <engine/renderengine.h>

namespace Miyuki {
	namespace GUI {
		class UIVisitor : public Reflection::TraitVisitor{
			Trait* selected = nullptr;

		public:
			RenderEngine* engine = nullptr;
			void visitGraph();
			void init();
			UIVisitor() {
				init();
			}
			void visitSelected() {
				if (selected) {
					visit(selected);
				}
			}
		};
	}
}