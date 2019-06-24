#pragma once
#include <reflection.h>

namespace Miyuki {
	namespace GUI {
		class UIVisitor : public Reflection::Visitor{
			void init();
			Reflection::Runtime* runtime;
		public:
			UIVisitor(Reflection::Runtime* rt) :runtime(rt) {}
			UIVisitor() { init(); }
		};
	}
}