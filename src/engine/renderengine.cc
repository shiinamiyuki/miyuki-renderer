#include <engine/renderengine.h>
#include <io/importobj.h>
#include <utils/log.h>

namespace Miyuki {
	static const char* MeshDirectory = "mesh";
	RenderEngine::RenderEngine() {
		_filename = "";
	}
	void RenderEngine::importObj(const std::string& filename) {
		IO::ObjLoadInfo info;
		info.basePath = cxx::filesystem::path(filename).parent_path();
		IO::LoadObjFile(filename, info);
		std::unordered_map<std::string, Core::Material*> map;
		for (auto& _m : info.materials) {
			auto m = static_cast<Core::MixedMaterial*>(_m.get());		
			map[m->name] = m;
		}
		std::move(info.materials.begin(), info.materials.end(), std::back_inserter(graph->materials));
		cxx::filesystem::path meshDir = cxx::filesystem::path(_filename).parent_path().append(MeshDirectory);
		if (!cxx::filesystem::exists(meshDir)) {
			cxx::filesystem::create_directory(meshDir);
			Log::log("Created {}\n", meshDir.string());
		}
		int meshId = graph->meshFiles.size();
		auto meshname = fmt::format("mesh{}.obj", meshId);
		auto meshPath = meshDir.append(meshname);
		Log::log("Created meshfile {}\n", meshPath.string());
		std::ofstream out(meshPath);
		out << info.outputContent << std::endl;
		auto mesh = Reflection::make_box<Core::MeshFile>();
		mesh->file = File(cxx::filesystem::relative(meshPath, cxx::filesystem::path(_filename).parent_path()));
	
	
		for (auto& s : info.shapeMat) {
			auto object = Reflection::make_box<Core::Object>();
			object->name = s.first;
			object->material = map.at(s.second);
			mesh->objects.emplace_back(std::move(object));
		}
		mesh->name = meshname;
		graph->meshFiles.emplace_back(std::move(mesh));
	}

	void RenderEngine::newGraph() {
		graph = Reflection::make_box<Core::Graph>();
	}


	void RenderEngine::saveTo(const std::string& filename) {
		if (!graph) {
			Log::log("Scene graph is empty!\n");
			return;
		}
		try {
			Reflection::OutStream stream;
			graph->serialize(stream);
			std::ofstream out(filename);
			out << stream.dump(1) << std::endl;
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
		
	}
}