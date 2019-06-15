#ifndef MIYUKI_RENDERENGINE_H
#define MIYUKI_RENDERENGINE_H
#include <miyuki.h>
#include <reflection.h>
#include <graph/graph.h>

namespace Miyuki {
	static const char* TempDirectory = ".tmp";
	class RenderEngine {
		std::string _filename;
		Reflection::Runtime runtime;
		Reflection::LocalObject<Graph::Graph> graph;
	public:
		RenderEngine();
		const std::string filename()const { return _filename; }

		// Precondition: graph is not null
		void importObj(const std::string& filename);
	};
}
#endif