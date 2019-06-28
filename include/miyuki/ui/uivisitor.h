#pragma once
#include <reflection.h>

namespace Miyuki {
	namespace Graph {
		class GraphNode;
	}
	namespace GUI {
		class UIVisitor : public Reflection::Visitor, NonCopyMovable{
			void init();
			Reflection::Runtime* runtime;
			Graph::GraphNode* selected = nullptr;
		public:
			UIVisitor(Reflection::Runtime* rt) :runtime(rt) {}
			UIVisitor() { init(); }
		};
	}
}