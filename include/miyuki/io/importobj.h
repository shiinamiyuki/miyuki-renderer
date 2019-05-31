#include <miyuki.h>

namespace Miyuki {
	namespace Graph { class Graph;  }
	namespace IO {
		struct ObjLoadInfo {
			std::unordered_map<std::string, std::string> shapeMat;
			std::string outputContent;
			json mtlDescription;
			Graph::Graph* graph = nullptr;
			ObjLoadInfo(Graph::Graph* graph) :graph(graph) {}
		};
		void LoadMTLFile(const std::string& filename, ObjLoadInfo& info);
		void LoadObjFile(const std::string& filename, ObjLoadInfo& info);
	}
}