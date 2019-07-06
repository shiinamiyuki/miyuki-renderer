#include <miyuki.h>
#include <core/materials/material.h>

#include <core/graph.h>

namespace Miyuki {
	namespace IO {
		static const char* TextureDir = "texture";
		struct ObjLoadInfo {
			std::unordered_map<std::string, std::string> shapeMat;
			std::string outputContent;
			std::vector<Box<Core::MaterialSlot>> materials;
			std::unordered_map<std::string, std::string> imageFileMoves;
			cxx::filesystem::path basePath;
			File meshFile;
		};
		void LoadMTLFile(const std::string& filename, ObjLoadInfo& info);
		void LoadObjFile(const std::string& filename, ObjLoadInfo& info);
	}
}