#include <miyuki.h>

namespace Miyuki {
	namespace IO {
		struct ObjLoadInfo {
			std::unordered_map<std::string, std::string> shapeMat;
			std::string outputContent;
			json mtlDescription;
		};
		void LoadMTLFile(const std::string& filename, ObjLoadInfo& info);
		void LoadObjFile(const std::string& filename, ObjLoadInfo& info);
	}
}