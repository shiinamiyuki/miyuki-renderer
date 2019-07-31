#pragma once
#include <miyuki.h>
#include <reflection.h>
#include <utils/file.hpp>
#include <utils/preprocessable.hpp>
#include <core/texture.h>


namespace Miyuki {
	namespace Core {
		struct ShadingPoint {
			Point2f uv;
			ShadingPoint(const Point2f& uv) :uv(uv) {}
		};

		struct ShadingResult {
			ShadingResult() {}
			ShadingResult(const Vec3f& v) :value(v) {}
			const Vec3f& toVec3f()const {
				return value;
			}
			Float toFloat()const { return value[0]; }
			ShadingResult operator + (const Vec3f k)const {
				return value + k;
			}
			ShadingResult operator - (const Vec3f k)const {
				return value - k;
			}
			ShadingResult operator * (Float k)const {
				return value * k;
			}
			ShadingResult operator / (Float k)const {
				return value / k;
			}
			ShadingResult operator + (const ShadingResult k)const {
				return value + k.value;
			}
			ShadingResult operator - (const ShadingResult k)const {
				return value - k.value;
			}
			ShadingResult operator * (const ShadingResult& k)const {
				return value * k.value;
			}
			ShadingResult operator / (const ShadingResult& k)const {
				return value / k.value;
			}
		private:
			friend ShadingResult operator + (Float k, const ShadingResult& r);
			friend ShadingResult operator - (Float k, const ShadingResult& r);
			friend ShadingResult operator * (Float k, const ShadingResult& r);
			friend ShadingResult operator / (Float k, const ShadingResult& r);
			Vec3f value;
		};
		inline ShadingResult operator + (Float k, const ShadingResult& r) {
			return ShadingResult(Vec3f(k) + r.value);
		}
		inline ShadingResult operator - (Float k, const ShadingResult& r) {
			return ShadingResult(Vec3f(k) - r.value);
		}
		inline ShadingResult operator * (Float k, const ShadingResult& r) {
			return ShadingResult(Vec3f(k) *  r.value);
		}
		inline ShadingResult operator / (Float k, const ShadingResult& r) {
			return ShadingResult(Vec3f(k) / r.value);
		}


		class Shader : public Reflective, public Preprocessable {
		public:
			MYK_INTERFACE(Shader);
			virtual ShadingResult eval(ShadingPoint&) const = 0;
			virtual ShadingResult average()const = 0;
			virtual void preprocess()override {}
			static ShadingResult evaluate(const Shader* shader, ShadingPoint& p) {
				if (shader) {
					return shader->eval(p);
				}
				else {
					return ShadingResult{};
				}
			}
			static ShadingResult evaluate(const Box<Shader>& shader, ShadingPoint& p) {
				return evaluate(shader.get(), p);
			}
		};
		MYK_REFL(Shader, (Reflective), MYK_REFL_NIL);

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

		class RGBShader final : public Shader {
		public:
			MYK_CLASS(RGBShader);
			RGBShader() {}
			RGBShader(Spectrum v) { setValue(v); }
			virtual ShadingResult eval(ShadingPoint& p) const override {
				return value * multiplier;
			}
			virtual ShadingResult average()const override {
				return Vec3f(value * multiplier);
			}
			void setValue(Spectrum value) {
				if (value.max() > 1.0f) {
					multiplier = value.max();
					value /= multiplier;
				}
				this->value = value;
			}
			Spectrum getValue()const { return value; }
			Float getMultiplier()const {
				return multiplier;
			}
			void setMultiplier(Float val) {
				multiplier = val;
			}
		private:
			Float multiplier = 1.0f;
			Spectrum value;
		};

		MYK_IMPL(RGBShader, "Shader.RGB");
		MYK_REFL(RGBShader, (Shader), (value)(multiplier));

		class ImageTextureShader final : public Shader {
			Spectrum _average;
		public:
			MYK_CLASS(ImageTextureShader);
			File imageFile;
			Texture texture;
			ImageTextureShader() {}
			ImageTextureShader(const File& f) :imageFile(f) {}
			virtual ShadingResult eval(ShadingPoint&) const override;
			virtual ShadingResult average()const override;
			virtual void preprocess()override;
		};
		MYK_IMPL(ImageTextureShader, "Shader.ImageTexture");
		MYK_REFL(ImageTextureShader, (Shader), (imageFile));

		class MixedShader final :public Shader {
		public:
			Box<Shader> fraction;
			Box<Shader> shaderA, shaderB;
			MYK_CLASS(MixedShader);
			virtual ShadingResult eval(ShadingPoint& p) const override {
				auto frac = Shader::evaluate(fraction, p);
				return Shader::evaluate(shaderA, p) * frac + Shader::evaluate(shaderB, p) * (1.0f - frac);
			}
			virtual ShadingResult average()const override {
				ShadingResult r;
				auto frac = fraction ? fraction->average() : ShadingResult();
				if (shaderA) {
					r = r + shaderA->average() * frac;
				}
				if (shaderB) {
					r = r + shaderB->average() * (1.0f - frac);
				}
				return r;
			}
		};
		MYK_IMPL(MixedShader, "Shader.Mixed");
		MYK_REFL(MixedShader, (Shader), (fraction)(shaderA)(shaderB));

	}
}
