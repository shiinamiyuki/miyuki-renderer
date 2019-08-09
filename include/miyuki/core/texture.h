#pragma once
#include <miyuki.h>

#include <io/image.h>

namespace Miyuki {
	namespace Core {
		struct Texture {
			IO::Image* image=nullptr;
			Texture() {}

			Texture(IO::Image* image):image(image) {}

			Spectrum evalUV(const Point2f& uv) const;

			Spectrum evalPixel(const Point2i& pos) const;

		};
	}
}
