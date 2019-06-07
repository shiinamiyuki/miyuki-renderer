#ifndef MIYUKI_GRAPHROOT_H
#define MIYUKI_GRAPHROOT_H

#include <graph/leaf.h>
#include <graph/materials.h>
#include <graph/meshes.h>

namespace Miyuki {
	namespace Graph {
		class Root : public Node{
			MYK_DECL_MEMBER(Materials, materials);
			MYK_DECL_MEMBER(Meshes, meshes);
		public:
			Root(Graph* G) 
				:Node("root", G),
				materials(this, "materials"),
				meshes(this, "meshes") {
			}
		};
	}
}


#endif