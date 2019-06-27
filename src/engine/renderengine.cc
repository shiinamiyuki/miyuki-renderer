#include <engine/renderengine.h>
#include <io/importobj.h>
#include <utils/log.h>

namespace Miyuki {

	RenderEngine::RenderEngine() :graph(runtime) {
		_filename = "";
	}

	void RenderEngine::importObj(const std::string& filename) {
		IO::ObjLoadInfo info(&runtime);
		info.basePath = cxx::filesystem::path(filename).parent_path();
		IO::LoadObjFile(filename, info);
		for (auto& m : info.materials) {
			graph->addMaterial(m);
		}
		int meshId = graph->meshes->size();
		std::ofstream out(info.meshFile.path);
		out << info.outputContent << std::endl;
		auto mesh = runtime.New<Graph::MeshNode>();
		mesh->meshFile = runtime.New<Reflection::FileNode>();
		mesh->meshFile->setValue(info.meshFile);
		mesh->transform = runtime.New<Graph::TransformNode>();
	}

	void RenderEngine::newGraph() {
		graph = runtime.New<Graph::Graph>(runtime.New<Reflection::Array<Graph::MaterialNode>>(),
			runtime.New<Reflection::Array<Graph::MeshNode>>());
	}

	void RenderEngine::visit(Reflection::Visitor& visitor) {
		if (graph)
			graph->accept(visitor);
	}

	void RenderEngine::saveTo(const std::string& filename) {
		if (!graph) {
			Log::log("Scene graph is empty!\n");
			return;
		}
		try {
			json j;
			auto err = graph->serialize(j);
			if (err.has_value()) {
				Log::log("Failed to save to {}; Error: {}\n", filename, err.value().what());
				return;
			}
			std::ofstream out(filename);
			out << j.dump(1) << std::endl;
			Log::log("Saved to {}\n", filename);
		}
		catch (std::runtime_error& e) {
			std::cout << e.what() << std::endl;
		}
	}
	void RenderEngine::open(const std::string& filename) {
		_filename = filename;
		std::ifstream in(filename);
		std::string content((std::istreambuf_iterator<char>(in)),
			std::istreambuf_iterator<char>());
		json j = json::parse(content);
		try {
			graph = runtime.deserialize<Graph::Graph>(j);
			Log::log("Opened {}\n", filename);
		}
		catch (std::runtime_error& e) {
			Log::log("Failed to open {}; Error: {}\n", filename, e.what());
		}
	}
}