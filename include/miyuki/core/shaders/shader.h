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


		class Shader : public Reflective, public CachedPreprocessable {
		public:
			MYK_INTERFACE(Shader);
			virtual ShadingResult eval(ShadingPoint&) const = 0;
			virtual ShadingResult average()const = 0;
			virtual void doPreprocess()override {}
			virtual Point2i resolution()const = 0;
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
			virtual Point2i resolution()const override {
				return Point2i(1, 1);
			}
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
			virtual Point2i resolution()const override {
				return Point2i(1, 1);
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
			virtual void doPreprocess()override;
			virtual Point2i resolution()const override;
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
			virtual Point2i resolution()const override {
				Point2i r1(1,1), r2;
				if (shaderA) {
					r1 = shaderA->resolution();
				}
				if (shaderB) {
					r2 = shaderB->resolution();
				}
				return Point2i(std::max(r1[0], r2[0]), std::max(r1[1], r2[1]));
			}
		};
		MYK_IMPL(MixedShader, "Shader.Mixed");
		MYK_REFL(MixedShader, (Shader), (fraction)(shaderA)(shaderB));

		class ScaledShader final :public Shader {
		public:
			Box<Shader> scale;
			Box<Shader> shader;
			MYK_CLASS(MixedShader);
			virtual ShadingResult eval(ShadingPoint& p) const override {
				auto k = Shader::evaluate(scale, p);
				return Shader::evaluate(shader, p) * k;
			}
			virtual ShadingResult average()const override {
				ShadingResult r;
				auto k = scale ? scale->average() : ShadingResult();
				if (shader) {
					r = r + shader->average() * k;
				}
				return r;
			}
			virtual Point2i resolution()const override {
				Point2i r1(1, 1), r2;
				if (scale)
					r1 = scale->resolution();
				if (shader)
					r2 = shader->resolution();
				return Point2i(std::max(r1[0], r2[0]), std::max(r1[1], r2[1]));
			}
		};
		MYK_IMPL(ScaledShader, "Shader.Scaled");
		MYK_REFL(ScaledShader, (Shader), (shader)(scale));

	}
}
