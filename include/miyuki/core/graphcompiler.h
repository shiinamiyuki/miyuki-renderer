#pragma once

#include <reflection.h>
#include <core/allocator.h>
#include <core/kernelrecord.h>
#include <core/texture.h>

namespace Miyuki {
	namespace Kernel {
		struct Shader;
	}
	namespace Core {
		class GraphCompiler {
			std::shared_ptr<KernelRecord> record;

		public:
			void push(Kernel::Shader* shader) { record->shaderProgram.emplace_back(shader); }
			void push(const Vec3f&);
			void newProgram();
			void programEnd();
			Kernel::ImageTexture createTexture(const Texture& texture);
			GraphCompiler(std::shared_ptr<KernelRecord> record) :record(record) {}
			std::shared_ptr<Allocator> getAllocator()const { return record->allocator; }
		};

		class Compilable {
		public:
			virtual void compile(GraphCompiler&)const {
				throw std::runtime_error("Not implemented");
			}
		};
	}
}