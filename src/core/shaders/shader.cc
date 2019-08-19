#include <core/shaders/shader.h>

namespace Miyuki {
	namespace Core {
		ShadingResult ImageTextureShader::eval(ShadingPoint& p)const {
			return texture.evalUV(p.uv);
		}
		ShadingResult ImageTextureShader::average()const {
			return _average;
		}
		Point2i ImageTextureShader::resolution()const {
			if (texture.image)
				return Point2i(texture.image->width, texture.image->height);
			return Point2i{};
		}
		void ImageTextureShader::doPreprocess() {
			Float cnt = 0;
			_average = {};
			for (auto i : texture.image->pixelData) {
				_average += i;
				cnt += 1;
			}
			_average /= cnt;
		}
	}
}