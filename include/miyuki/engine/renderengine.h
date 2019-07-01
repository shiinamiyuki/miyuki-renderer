#ifndef MIYUKI_RENDERENGINE_H
#define MIYUKI_RENDERENGINE_H
#include <miyuki.h>
#include <core/graph.h>

namespace Miyuki {
	class RenderEngine {
		std::string _filename;
		Box<Core::Graph> graph;
	public:
		RenderEngine();
		const std::string filename()const { return _filename; }
		// Precondition: graph is not null
		void importObj(const std::string& filename);
		void newGraph();
		void saveTo(const std::string& filename);
		void save() {
			saveTo(filename());
		}
		bool hasGraph() {
			return graph != nullptr;
		}
		// overrides current graph if any
		void open(const std::string& filename);
		bool isFilenameEmpty() {
			return filename().empty();
		}
		void firstSave(const std::string& filename) {
			_filename = filename;
			save();
		}
		Core::Graph* getGraph() {
			return graph.get();
		}
	};
}
#endif