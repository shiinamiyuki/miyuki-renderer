#ifndef MIYUKI_GRAPHROOT_H
#define MIYUKI_GRAPHROOT_H

#include <graph/leaf.h>
#include <graph/materials.h>
#include <graph/meshes.h>

namespace Miyuki {
	namespace Graph {
		class Root : public Node{
		public:
			MYK_NODE_MEMBER(Materials, materials)
			MYK_NODE_MEMBER(Meshes, meshes)
			Root(Graph* G) :Node("root", G) {
				init_materials();
				init_meshes();
			}
		};
	}
}


#endif