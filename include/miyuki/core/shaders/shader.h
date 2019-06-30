#pragma once


#include <miyuki.h>

namespace Miyuki {
	namespace Core {
		struct ShadingPoint {
			Point2f uv;
			Vec3f output;
		};

		class Shader {
		public:
			virtual void eval(ShadingPoint&) = 0;
		};

		class RGBShader : public Shader{

		};

		class TextureShader : public Shader {

		};

		class ImageTextureShader : public Shader {

		};
	}
}