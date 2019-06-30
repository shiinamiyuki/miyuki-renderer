#pragma once
#include <reflection.h>
#include <graph/graph.h>
namespace Miyuki {

	namespace GUI {
		class MainWindow;
		class UIVisitor : public Reflection::Visitor, NonCopyMovable {
			void init();
			Reflection::Runtime* runtime;
			Graph::GraphNode* selected = nullptr;
			int selectedMaterialIndex = -1;
			MainWindow * mainWindow = nullptr;
			Graph::Graph* graph = nullptr;
		public:
			void visitSelected();
			UIVisitor(Reflection::Runtime* rt) :runtime(rt) { init(); }
		};
	}
}