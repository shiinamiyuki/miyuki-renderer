#pragma once

#include <miyuki.h>
#include <math/spectrum.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <utils/noncopyable.hpp>
#include <io/image.h>

namespace Miyuki {
	namespace HW {
		class Texture : NonCopyable{
			GLuint texture;
			size_t width, height;
		public:
			GLuint getTexture()const { return texture; }
			Texture(size_t width, size_t height);
			Texture(size_t width, size_t height, uint8_t * data);
			Texture(const IO::Image&);
			Point2i size() const{ return Point2i(width,height); }

			// clears all contents
			void resize(size_t width, size_t height);

			void setPixel(int x, int y, const Spectrum& color);
			void setPixel(const Point2f& p, const Spectrum& color) {
				int x = clamp<int>(p[0], 0, width - 1);
				int y = clamp<int>(p[1], 0, height - 1);
				setPixel(x, y, color);
			}
			void setData(uint8_t* data);

			void use();

			~Texture();
		};		
	}
}