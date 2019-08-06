#pragma once

#include <reflection.h>


namespace Miyuki {
	namespace Core {
		class GraphCompiler {
		public:
		};

		class Compilable {
		public:
			virtual void compile(GraphCompiler&)const {
				throw std::runtime_error("Not implemented");
			}
		};
	}
}