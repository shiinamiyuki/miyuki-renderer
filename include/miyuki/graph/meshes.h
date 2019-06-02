#ifndef MIYUKI_MESHES_H
#define MIYUKI_MESHES_H

#include <graph/graph.h>

namespace Miyuki {
	namespace Graph {
		// a node holding all meshes
		class Meshes : public Node {
		public:
			Meshes(Graph* G) :Node("meshes", G) {}
			const char* type()const { return "Meshes"; }
		};
	}
}

#endif