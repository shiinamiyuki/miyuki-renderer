#pragma once


#include <miyuki.h>
#include <reflection.h>
#include <utils/file.hpp>
#include <io/image.h>
namespace Miyuki {
	namespace Core {
		struct ShadingPoint {
			Point2f uv;
			ShadingPoint(const Point2f& uv) :uv(uv){}
		};

		struct ShadingResult {
			ShadingResult() {}
			ShadingResult(const Vec3f& v) :value(v) {}
			const Vec3f& toVec3f()const {
				return value;
			}
			Float toFloat()const { return value[0]; }
		private:
			Vec3f value;
		};

		class Shader : public Reflective {
		public:
			MYK_INTERFACE(Shader);
			virtual ShadingResult eval(ShadingPoint&) const = 0;
			virtual ShadingResult average()const = 0;
			virtual void preprocess() {}
		};
		MYK_REFL(Shader, (Reflective),MYK_REFL_NIL);

		class FloatShader final : public Shader {
		public:
			MYK_CLASS(FloatShader);
			FloatShader() {}
			FloatShader(Float v) :value(v) {}
			virtual ShadingResult eval(ShadingPoint& p) const override {
				return Vec3f(value);
			}
			virtual ShadingResult average()const override {
				return Vec3f(value);
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
			virtual ShadingResult eval(ShadingPoint& p) const override{
				return value;
			}
			virtual ShadingResult average()const override {
				return Vec3f(value);
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
			virtual ShadingResult eval(ShadingPoint&) const override {
				return {};
			}
			virtual ShadingResult average()const override {
				return {};
			}
		};
		MYK_IMPL(ImageTextureShader,  "Shader.ImageTexture");
		MYK_REFL(ImageTextureShader, (Shader), (imageFile));
	}
}
