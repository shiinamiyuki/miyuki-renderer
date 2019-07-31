#ifndef MIYUKI_FILE_HPP
#define MIYUKI_FILE_HPP

#include <miyuki.h>
#include <reflection.h>

namespace Miyuki {
	struct File {
		cxx::filesystem::path path;
		File() {}
		File(const std::string& s) :path(s) {}
		File(cxx::filesystem::path path) :path(std::move(path)) {}
		bool operator == (const File& rhs) const{
			return path == rhs.path;
		}
		std::string str()const {
			return path.string();
		}
		cxx::filesystem::path fullpath()const {
			return cxx::filesystem::absolute(path);
		}
		cxx::filesystem::path relativePath()const {
			return cxx::filesystem::relative(path);
		}
	};

	inline void from_json(const json& j, File& file) {
		file.path = j.get<std::string>();
	}
	inline void to_json(json& j, const File& file) {
		j = file.relativePath().string();
	}
	namespace Reflection {
		MYK_SAVE_LOAD_TRIVIAL(File)
	}
}


#endif