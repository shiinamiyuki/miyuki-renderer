#include <engine/renderengine.h>
#include <io/importobj.h>
#include <utils/log.h>
#include <core/cameras/camera.h>

namespace Miyuki {
	static const char* MeshDirectory = "mesh";
	RenderEngine::RenderEngine() {
		_filename = "";
	}
	void RenderEngine::importObj(const std::string& filename) {
		IO::ObjLoadInfo info;
		info.basePath = cxx::filesystem::path(filename).parent_path();
		IO::LoadObjFile(filename, info);

		std::move(info.materials.begin(), info.materials.end(), std::back_inserter(graph->materials));
		cxx::filesystem::path meshDir = cxx::filesystem::path(_filename).parent_path().append(MeshDirectory);
		if (!cxx::filesystem::exists(meshDir)) {
			cxx::filesystem::create_directory(meshDir);
			Log::log("Created {}\n", meshDir.string());
		}
		int meshId = graph->meshes.size();
		auto meshname = fmt::format("mesh{}.obj", meshId);
		auto meshPath = meshDir.append(meshname);
		Log::log("Created meshfile {}\n", meshPath.string());
		std::ofstream out(meshPath);
		out << info.outputContent << std::endl;
		auto mesh = Reflection::makeBox<Core::MeshFile>();
		mesh->file = File(cxx::filesystem::relative(meshPath, cxx::filesystem::path(_filename).parent_path()));

		std::unordered_map<std::string, Core::MaterialSlot*> map;
		for (auto& m : graph->materials) {
			map[m->name] = m.get();
		}

		for (auto& s : info.shapeMat) {
			auto object = Reflection::makeBox<Core::Object>();
			object->name = s.first;
			object->material = map.at(s.second);
			mesh->objects.emplace_back(std::move(object));
		}
		mesh->name = meshname;
		graph->meshes.emplace_back(std::move(mesh));
	}

	void RenderEngine::newGraph() {
		graph = Reflection::makeBox<Core::Graph>();
		scene = std::make_unique<Core::Scene>();
	}


	void RenderEngine::saveTo(const std::string& filename) {
		if (!graph) {
			Log::log("Scene graph is empty!\n");
			return;
		}
		try {
			Reflection::OutObjectStream stream;
			graph->serialize(stream);
			std::ofstream out(filename);
			out << stream.toJson().dump(1) << std::endl;
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
			Reflection::InObjectStream in(j);
			newGraph();
			graph->deserialize(in);
			Log::log("Opened {}\n", filename);
			Log::log("Loaded {} materials\n", graph->materials.size());

			if (graph->cameras.empty()) {
				auto camera = Reflection::makeBox<Core::PerspectiveCamera>();
				graph->activeCamera = camera.get();
				graph->cameras.emplace_back(std::move(camera));
			}
			else if (!graph->activeCamera) {
				graph->activeCamera = graph->cameras.front().get();
			}
		}
		catch (std::runtime_error& e) {
			Log::log("Failed to open {}; Error: {}\n", filename, e.what());
		}
	}
	bool RenderEngine::startProgressiveRender(Core::ProgressiveRenderCallback callback) {
		if (isRenderThreadStarted())
			throw std::runtime_error("render thread is still alive!");
		if (!graph)
			return false;
		auto integrator = Reflection::cast<Core::ProgressiveRenderer>(graph->integrator.get());
		if (!integrator)return false;
		commit();
		renderThread = std::make_unique<std::thread>([=]() {
			Core::IntegratorContext ctx;
			ctx.camera = graph->activeCamera->clone();
			ctx.camera->preprocess();
			ctx.film = makeArc<Core::Film>(graph->filmConfig.dimension);
			ctx.sampler = graph->sampler->clone();
			ctx.scene = scene.get();
			ctx.resultCallback = [=](Arc<Core::Film>film) {
				film->writePNG("out.png");
				renderThread->detach();
				renderThread = nullptr;
			};
			Log::log("render thread started\n");
			integrator->renderProgressive(ctx, std::move(callback));
		});
		
		return true;
	}
}