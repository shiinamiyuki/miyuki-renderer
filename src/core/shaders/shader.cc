#include <core/shaders/shader.h>
#include <core/graphcompiler.h>
#include <kernel/kernel_shader.h>
#include <kernel/kernel_texture.h>

namespace Miyuki {
	namespace Core {
		Kernel::ShaderData Shader::compileToKernelShader(const Shader* shader, GraphCompiler& compiler) {
			auto data = compiler.newProgram();
			if (shader) {
				shader->compile(compiler);
			}
			compiler.programEnd();
			return data;
		}
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
		void ImageTextureShader::preprocess() {
			Float cnt = 0;
			_average = {};
			for (auto i : texture.image->pixelData) {
				_average += i;
				cnt += 1;
			}
			_average /= cnt;
		}
		void FloatShader::compile(GraphCompiler& compiler)const {
			compiler.push(Vec3f(value));
		}

		void RGBShader::compile(GraphCompiler& compiler)const {
			compiler.push(value);
		}
		void ImageTextureShader::compile(GraphCompiler& compiler)const {
			Kernel::Shader shader;
			Kernel::create_image_texture_shader(&shader);
			shader.image_texture_shader.texture = compiler.createTexture(texture);
			compiler.push(shader);
			CHECK(shader.image_texture_shader.texture.data);
			CHECK(shader.image_texture_shader.texture.width >= 0);
			CHECK(shader.image_texture_shader.texture.height >= 0);
		}

		void MixedShader::compile(GraphCompiler& compiler)const {
			Kernel::Shader shader;
			Kernel::create_mixed_shader(&shader);
			if (shaderB)
				shaderB->compile(compiler);
			else {
				compiler.push(Vec3f(0.0f));
			}
			if (shaderA)
				shaderA->compile(compiler);
			else {
				compiler.push(Vec3f(0.0f));
			}
			if (fraction) {
				fraction->compile(compiler);
			}
			else {
				compiler.push(Vec3f(0.5f));
			}
			compiler.push(shader);
		}

		void ScaledShader::compile(GraphCompiler& compiler)const {
			Kernel::Shader shader;
			Kernel::create_scaled_shader(&shader);
			if (this->shader) {
				this->shader->compile(compiler);
			}
			else {
				compiler.push(Vec3f(0.0f));
			}
			if (scale) {
				scale->compile(compiler);
			}
			else {
				compiler.push(Vec3f(1.0f));
			}

			compiler.push(shader);
		}
	}
}