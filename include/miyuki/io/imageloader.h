#ifndef MIYUKI_IMAGELOADER_H
#define MIYUKI_IMAGELOADER_H

#include <io/image.h>
#include <utils/file.hpp>
namespace Miyuki {
	namespace IO {
		class ImageLoader {
			std::unordered_map<std::string, std::unique_ptr<Image>> images;
		public:
			Image* load(const File& path);
			void reloadAll();
		};
	}
}
#endif