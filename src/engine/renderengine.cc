#include <engine/renderengine.h>
#include <io/importobj.h>
namespace Miyuki {
	
	RenderEngine::RenderEngine() {
		if (!cxx::filesystem::exists(TempDirectory)) {
			cxx::filesystem::create_directory(TempDirectory);
		}
		_filename = std::string(TempDirectory) + "scene.json";
	}
	void RenderEngine::importObj(const std::string& filename) {
		IO::ObjLoadInfo info;
		IO::LoadObjFile(filename, info);
	}
}