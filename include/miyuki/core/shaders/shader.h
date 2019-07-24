#pragma once


#include <miyuki.h>
#include <reflection.h>
#include <utils/file.hpp>
#include <io/image.h>
namespace Miyuki {
	namespace Core {
		struct ShadingPoint {
			Point2f uv;
			Vec3f output;
		};

		class Shader : public Reflective {
		public:
			MYK_INTERFACE(Shader);
			virtual void eval(ShadingPoint&) = 0;
			virtual void preprocess() {}
		};
		MYK_REFL(Shader, (Reflective),MYK_REFL_NIL);

		class FloatShader final : public Shader {
		public:
			MYK_CLASS(FloatShader);
			FloatShader() {}
			FloatShader(Float v) :value(v) {}
			virtual void eval(ShadingPoint& p) override {
				p.output = Vec3f(value);
			}
			void setValue(Float value) { this->value = value; }
			Float getValue()const { return value; }
		private:
			Float value = 0;
		};
		MYK_IMPL(FloatShader, "Shader.Float");
		MYK_REFL(FloatShader, (Shader), (value));

		class RGBShader final: public Shader{
		public:
			MYK_CLASS(RGBShader);
			RGBShader() {}
			RGBShader(Spectrum v) :value(v) {}
			virtual void eval(ShadingPoint& p) override{
				p.output = value;
			}
			void setValue(Spectrum value) { this->value = value; }
			Spectrum getValue()const { return value; }
		private:
			Spectrum value;
		};

		MYK_IMPL(RGBShader,  "Shader.RGB");
		MYK_REFL(RGBShader, (Shader), (value));

		class ImageTextureShader final : public Shader {
		public:
			MYK_CLASS(ImageTextureShader);
			File imageFile;
			IO::Image* texture = nullptr;
			ImageTextureShader() {}
			ImageTextureShader(const File& f) :imageFile(f) {}
			virtual void eval(ShadingPoint&) override {

			}
		};
		MYK_IMPL(ImageTextureShader,  "Shader.ImageTexture");
		MYK_REFL(ImageTextureShader, (Shader), (imageFile));
	}
}
