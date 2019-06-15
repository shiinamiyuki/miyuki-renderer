#include <miyuki.h>
#include <reflection.h>
namespace Miyuki {
	namespace IO {
		struct ObjLoadInfo {
			std::unordered_map<std::string, std::string> shapeMat;
			std::string outputContent;
			json mtlDescription;
			Reflection::Runtime* runtime = nullptr;
			ObjLoadInfo(Reflection::Runtime* runtime) :runtime(runtime) {}
		};
		void LoadMTLFile(const std::string& filename, ObjLoadInfo& info);
		void LoadObjFile(const std::string& filename, ObjLoadInfo& info);
	}
}