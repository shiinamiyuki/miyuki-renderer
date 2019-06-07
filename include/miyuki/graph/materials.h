#ifndef MIYUKI_MATERIALS_H
#define MIYUKI_MATERIALS_H

#include <graph/graph.h>

namespace Miyuki {
	namespace Graph {
		// a node holding all materials
		class Materials : public Node{
			MYK_NODE_CLASS(Materials)
		public:
			Materials(Graph* G) :Node("materials", G) {}
			const std::vector<Edge*> subnodes()const override { return {}; }
		};
	}
}

#endif