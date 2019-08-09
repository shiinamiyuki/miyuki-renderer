#include <core/graphcompiler.h>
#include <kernel/kernel_shader.h>
namespace Miyuki {
	namespace Core {
		void GraphCompiler::push(const Vec3f& v) {
			auto shader = getAllocator()->allocT<Kernel::Float3Shader>();
			Kernel::create_float3_shader(shader);
			shader->value = v;
			push((Kernel::Shader*)shader);
		}
		void GraphCompiler::newProgram() {

		}
		void GraphCompiler::programEnd() {
			auto shader = getAllocator()->allocT<Kernel::EndShader>();
			Kernel::create_end_shader(shader);
			push((Kernel::Shader*)shader);
		}
		Kernel::ImageTexture GraphCompiler::createTexture(const Texture& texture) {
			auto& textures = record->textures;
			Kernel::ImageTexture image;
			Kernel::float4* data = (Kernel::float4*)getAllocator()->alloc(
				sizeof(Kernel::float4) * texture.image->pixelData.size());
			for (auto i = 0; i < texture.image->pixelData.size(); i++) {
				const auto& v = texture.image->pixelData[i];
				data[i].r = v.r;
				data[i].g = v.g;
				data[i].b = v.b;
				data[i].a = 1.0f;
			}
			image.data = data;
			image.width = texture.image->width;
			image.width = texture.image->height;
			textures.push_back(image);
			return textures.back();
		}
	}
}