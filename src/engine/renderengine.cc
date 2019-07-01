#include <engine/renderengine.h>
#include <io/importobj.h>
#include <utils/log.h>

namespace Miyuki {
	static const char* MeshDirectory = "mesh";
	RenderEngine::RenderEngine() {
		_filename = "";
	}
	void RenderEngine::importObj(const std::string& filename) {
		
	}

	void RenderEngine::newGraph() {
		
	}


	void RenderEngine::saveTo(const std::string& filename) {
		
	}
	void RenderEngine::open(const std::string& filename) {
		_filename = filename;
		std::ifstream in(filename);
		std::string content((std::istreambuf_iterator<char>(in)),
			std::istreambuf_iterator<char>());
		json j = json::parse(content);
		
	}
}