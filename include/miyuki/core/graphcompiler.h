#pragma once

#include <reflection.h>
#include <core/allocator.h>
#include <core/kernelrecord.h>
#include <core/texture.h>
#include <kernel/kernel_shader.h>
#include <kernel/kernel_shaderdata.h>
#include <kernel/kernel_material.h>
namespace Miyuki {
	namespace Kernel {
		struct Shader;
	}
	namespace Core {
		class GraphCompiler {
			std::shared_ptr<KernelRecord> record;

		public:
			int getNullMaterialId()const {
				return 0;
			}
			int getLastAddedMaterialId()const;
			void addMaterial(const Kernel::Material&);
			void push(const Kernel::Shader& shader) { record->shaderProgram.emplace_back(shader); }
			void push(const Vec3f&);
			Kernel::ShaderData newProgram();
			void programEnd();
			Kernel::ImageTexture createTexture(const Texture& texture);
			GraphCompiler(std::shared_ptr<KernelRecord> record);
			std::shared_ptr<Allocator> getAllocator()const { return record->allocator; }
		};
	}
}