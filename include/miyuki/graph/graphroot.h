#ifndef MIYUKI_GRAPHROOT_H
#define MIYUKI_GRAPHROOT_H

#include <graph/leaf.h>
#include <graph/materials.h>
#include <graph/meshes.h>

namespace Miyuki {
	namespace Graph {
		class Root : public Node{
			Materials* materials = nullptr;
			Meshes* meshes = nullptr;
		public:
			Root(Graph* G) :Node("root", G) {
			}
		};
	}
}


#endif