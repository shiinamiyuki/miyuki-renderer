#ifndef MIYUKI_RENDERENGINE_H
#define MIYUKI_RENDERENGINE_H
#include <miyuki.h>
#include <reflection.h>
#include <graph/graph.h>

namespace Miyuki {
	class RenderEngine {
		std::string _filename;
		Reflection::Runtime runtime;
		Reflection::LocalObject<Graph::Graph> graph;
		void registerClasses();
	public:
		RenderEngine();
		const std::string filename()const { return _filename; }
		Reflection::Runtime* getRuntime() { return &runtime; }
		// Precondition: graph is not null
		void importObj(const std::string& filename);
		void newGraph();
		void visit(Reflection::Visitor& visitor);
		void saveTo(const std::string& filename);
		void save() {
			saveTo(filename());
		}

		// overrides current graph if any
		void open(const std::string& filename);
		bool isFilenameEmpty() {
			return filename().empty();
		}
		bool hasGraph() {
			return graph;
		}
		void firstSave(const std::string& filename) {
			_filename = filename;
			save();
		}
	};
}
#endif