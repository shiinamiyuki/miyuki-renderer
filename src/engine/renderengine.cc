#include <engine/renderengine.h>
#include <io/importobj.h>
namespace Miyuki {
	
	RenderEngine::RenderEngine():graph(runtime) {
		if (!cxx::filesystem::exists(TempDirectory)) {
			cxx::filesystem::create_directory(TempDirectory);
		}
		_filename = std::string(TempDirectory) + "scene.json";
	}
	void RenderEngine::importObj(const std::string& filename) {
		std::unique_ptr<Reflection::Runtime> runtime(new Reflection::Runtime());
		IO::ObjLoadInfo info(runtime.get());
		IO::LoadObjFile(filename, info);
	}
}