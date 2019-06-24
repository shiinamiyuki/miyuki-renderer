#include <engine/renderengine.h>
#include <io/importobj.h>
#include <utils/log.h>

namespace Miyuki {

	RenderEngine::RenderEngine() :graph(runtime) {
		if (!cxx::filesystem::exists(TempDirectory)) {
			cxx::filesystem::create_directory(TempDirectory);
		}
		_filename = std::string(TempDirectory) + "scene.json";
	}

	void RenderEngine::importObj(const std::string& filename) {
		IO::ObjLoadInfo info(&runtime);
		IO::LoadObjFile(filename, info);
		for (auto& m : info.materials) {
			graph->addMaterial(m);
		}
	}

	void RenderEngine::newGraph() {
		graph = runtime.New<Graph::Graph>(runtime.New<Reflection::Array<Graph::MaterialNode>>(),
			runtime.New<Reflection::Array<Graph::MeshNode>>());
	}

	void RenderEngine::visit(Reflection::Visitor& visitor) {
		graph->accept(visitor);
	}
}