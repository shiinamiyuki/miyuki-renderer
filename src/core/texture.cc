#include <core/texture.h>

namespace Miyuki {
	namespace Core {
		Spectrum Texture::evalUV(const Point2f& uv)const {
			if (!image)return {};
			int x = static_cast<int>(uv.x * image->width);
			int y = static_cast<int>((1 - uv.y) * image->height);
			return image->operator()(x, y);
		}

		Spectrum Texture::evalPixel(const Point2i& pos)const {
			return image->operator()(pos.x, pos.y);
		}
	}
}