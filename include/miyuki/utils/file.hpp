#ifndef MIYUKI_FILE_HPP
#define MIYUKI_FILE_HPP

#include <miyuki.h>

namespace Miyuki {
	struct File {
		cxx::filesystem::path path;
		File() {}
		File(const std::string& s) :path(s) {}
		File(cxx::filesystem::path path) :path(std::move(path)) {}
		bool operator == (const File& rhs) const{
			return path == rhs.path;
		}
	};

	inline void from_json(const json& j, File& file) {
		file.path = j.get<std::string>();
	}
	inline void to_json(json& j, const File& file) {
		j = file.path.string();
	}
}


#endif