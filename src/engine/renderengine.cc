#include <engine/renderengine.h>
#include <io/importobj.h>
#include <utils/log.h>

namespace Miyuki {
	static const char* MeshDirectory = "mesh";
	RenderEngine::RenderEngine() :graph(runtime) {
		_filename = "";
		registerClasses();
	}
	void RenderEngine::registerClasses() {
		runtime.registerClass<Graph::DiffuseMaterialNode>()
			.registerClass<Reflection::IntNode>()
			.registerClass<Reflection::FloatNode>()
			.registerClass<Reflection::Float3Node>()
			.registerClass<Reflection::StringNode>()
			.registerClass<Reflection::FileNode>()
			.registerClass<Graph::FloatNode>()
			.registerClass<Graph::GlossyMaterialNode>()
			.registerClass<Graph::Graph>()
			.registerClass<Graph::GraphNode>()
			.registerClass<Graph::ImageTextureNode>()
			.registerClass<Graph::MaterialNode>()
			.registerClass<Reflection::Array<Graph::MaterialNode>>()
			.registerClass<Reflection::Array<Graph::MeshNode>>()
			.registerClass<Graph::MeshNode>()
			.registerClass<Graph::MixedMaterialNode>()
			.registerClass<Graph::ShaderNode>()
			.registerClass<Graph::RGBNode>()
			.registerClass<Graph::TextureNode>()
			.registerClass<Graph::TransformNode>();
			
	}
	void RenderEngine::importObj(const std::string& filename) {
		IO::ObjLoadInfo info(&runtime);
		info.basePath = cxx::filesystem::path(filename).parent_path();
		IO::LoadObjFile(filename, info);
		for (auto& m : info.materials) {
			graph->addMaterial(m);
		}
		cxx::filesystem::path meshDir = cxx::filesystem::path(_filename).parent_path().append(MeshDirectory);
		if (!cxx::filesystem::exists(meshDir)) {
			cxx::filesystem::create_directory(meshDir);
			Log::log("Created {}\n", meshDir.string());
		}
		int meshId = graph->meshes->size();
		auto meshPath = meshDir.append(fmt::format("mesh{}.obj",meshId));
		Log::log("Created meshfile {}\n", meshPath.string());
		std::ofstream out(meshPath);
		out << info.outputContent << std::endl;
		auto mesh = runtime.New<Graph::MeshNode>();
		mesh->meshFile = runtime.New<Reflection::FileNode>();
		mesh->meshFile->setValue(File(cxx::filesystem::relative(meshPath, cxx::filesystem::path(_filename).parent_path())));
		mesh->transform = runtime.New<Graph::TransformNode>();
		graph->meshes->push_back(mesh);
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