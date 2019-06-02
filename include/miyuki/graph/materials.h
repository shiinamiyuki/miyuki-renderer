#ifndef MIYUKI_MATERIALS_H
#define MIYUKI_MATERIALS_H

#include <graph/graph.h>

namespace Miyuki {
	namespace Graph {
		// a node holding all materials
		class Materials : public Node{
		public:
			Materials(Graph* G) :Node("materials", G) {}
			const char* type()const { return "Materials"; }
		};
	}
}

#endif