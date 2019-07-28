#include <core/shaders/shader.h>

namespace Miyuki {
	namespace Core {
		ShadingResult ImageTextureShader::eval(ShadingPoint& p)const {
			return texture.evalUV(p.uv);
		}
		ShadingResult ImageTextureShader::average()const {
			return _average;
		}
		void ImageTextureShader::preprocess() {
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