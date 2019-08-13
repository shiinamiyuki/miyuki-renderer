#pragma once

namespace Miyuki {
	namespace Core {
		class GraphCompiler;
		class Compilable {
		public:
			virtual void compile(GraphCompiler&)const {
				throw std::runtime_error("Not implemented");
			}
		};
	}
}