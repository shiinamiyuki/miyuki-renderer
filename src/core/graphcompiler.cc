#include <core/graphcompiler.h>
#include <kernel/kernel_shader.h>
namespace Miyuki {
	namespace Core {
		int GraphCompiler::getLastAddedMaterialId()const {
			return record->materials.size() - 1;
		}
		void GraphCompiler::addMaterial(const Kernel::Material& mat) {
			record->materials.push_back(mat);
		}
		void GraphCompiler::push(const Vec3f& v) {
			Kernel::Shader shader;
			Kernel::create_float3_shader(&shader);
			shader.float3_shader.value = v;
			push(shader);
		}
		Kernel::ShaderData GraphCompiler::newProgram() {
			Kernel::ShaderData data;
			data.offset = record->shaderProgram.size();
			return data;
		}
		void GraphCompiler::programEnd() {
			Kernel::Shader shader;
			Kernel::create_end_shader(&shader);
			push(shader);
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

		GraphCompiler::GraphCompiler(std::shared_ptr<KernelRecord> record) {
			 
		}
	}
}