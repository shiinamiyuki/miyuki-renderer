#ifndef MIYUKI_RENDERENGINE_H
#define MIYUKI_RENDERENGINE_H
#include <miyuki.h>
#include <core/graph.h>
#include <core/integrators/integrator.h>
#include <core/scene.h>

namespace Miyuki {
	class RenderEngine {
	private:
		std::string _filename;
		Box<Core::Graph> graph;
		std::unique_ptr<Core::Scene> scene;
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

		// overwrites current graph if any
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
		void commit() {
			scene->commit(*graph);
		}
		Core::Scene* getScene() {
			return scene.get();
		}
	};
}
#endif