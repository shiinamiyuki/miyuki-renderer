#pragma once
#include <graph/visitor.h>

namespace Miyuki {
	namespace GUI {
		class UIVisitor : public Graph::Visitor{
			void init();
			Reflection::Runtime* runtime;
		public:
			UIVisitor(Reflection::Runtime* rt) :runtime(rt) {}
			UIVisitor() { init(); }
		};
	}
}