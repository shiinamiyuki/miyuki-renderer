#ifndef MIYUKI_RENDERENGINE_H
#define MIYUKI_RENDERENGINE_H
#include <graph/graph.h>


namespace Miyuki {
	class RenderEngine {
		std::unique_ptr<Graph::Graph> _graph;
	public:
		Graph:: Graph* sceneGraph() { return _graph.get(); }
	};
}
#endif