#ifndef MIYUKI_RENDERENGINE_H
#define MIYUKI_RENDERENGINE_H
#include <graph/graph.h>


namespace Miyuki {
	static const char* TempDirectory = ".tmp";
	class RenderEngine {
		std::unique_ptr<Graph::Graph> _graph;
		std::string _filename;
	public:
		RenderEngine();
		Graph:: Graph* graph() { return _graph.get(); }
		const std::string filename()const { return _filename; }
		void newGraph() {
			_graph = Graph::Graph::NewGraph();
		}

		// Precondition: graph is not null
		void importObj(const std::string& filename);
	};
}
#endif