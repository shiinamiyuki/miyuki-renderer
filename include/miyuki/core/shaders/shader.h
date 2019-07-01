#pragma once


#include <miyuki.h>
#include <reflection.h>
#include <utils/file.hpp>
namespace Miyuki {
	namespace Core {
		struct ShadingPoint {
			Point2f uv;
			Vec3f output;
		};

		struct Shader : Trait{
		public:
			virtual void eval(ShadingPoint&) = 0;
			virtual void preprocess() {}
		};

		struct FloatShader final : public Shader {
		//	MYK_IMPL(FloatShader);
			
			FloatShader(Float v) :value(v) {}
			virtual void eval(ShadingPoint& p) override {
				p.output = Vec3f(value);
			}
		private:
			Float value = 0;
		};



		struct RGBShader final: public Shader{
		//	MYK_IMPL(RGBShader);
			RGBShader(Spectrum v) :value(v) {}
			virtual void eval(ShadingPoint& p) override{
				p.output = value;
			}
		private:
			Spectrum value;
		};

		struct ImageTextureShader final : public Shader {
			Box<File> imageFile;
		//	MYK_IMPL(ImageTextureShader);
			virtual void eval(ShadingPoint&) override {

			}
		};
	}
}