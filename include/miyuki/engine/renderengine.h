#ifndef MIYUKI_RENDERENGINE_H
#define MIYUKI_RENDERENGINE_H
#include <miyuki.h>

namespace Miyuki {
	static const char* TempDirectory = ".tmp";
	class RenderEngine {
		std::string _filename;
	public:
		RenderEngine();
		const std::string filename()const { return _filename; }

		// Precondition: graph is not null
		void importObj(const std::string& filename);
	};
}
#endif