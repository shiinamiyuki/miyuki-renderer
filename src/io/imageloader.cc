#include <io/imageloader.h>
#include <utils/log.h>

namespace Miyuki {
	namespace IO {
		Image* ImageLoader::load(const File& path) {
			auto filename = path.fullpath().string();
			auto iter = images.find(filename);
			if (iter != images.end()) {
				return iter->second.get();
			}
			auto image = std::make_unique<Image>(filename);
			Log::log("Loaded {}\n", filename);
			images.insert(std::make_pair(filename, std::move(image)));
			return images.at(filename).get();
		}
		void ImageLoader::reloadAll() {
			for (auto& pair : images) {
				auto filename = pair.first;
				pair.second = std::move(std::make_unique<Image>(filename));
			}
		}
	}
}