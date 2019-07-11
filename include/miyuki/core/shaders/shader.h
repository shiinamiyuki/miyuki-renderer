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

		struct Shader : Component {
		public:
			virtual void eval(ShadingPoint&) = 0;
			virtual void preprocess() {}
		};

		struct FloatShader final : public Shader {
			MYK_IMPL(FloatShader);
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

		struct RGBShader final: public Shader{
			MYK_IMPL(RGBShader);
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

		struct ImageTextureShader final : public Shader {
			File imageFile;
			IO::Image* texture = nullptr;
			ImageTextureShader() {}
			ImageTextureShader(const File& f) :imageFile(f) {}
			MYK_IMPL(ImageTextureShader);
			virtual void eval(ShadingPoint&) override {

			}
		};
	}
}
MYK_REFL(Miyuki::Core::FloatShader,(value))
MYK_REFL(Miyuki::Core::RGBShader, (value))
MYK_REFL(Miyuki::Core::ImageTextureShader, (imageFile))