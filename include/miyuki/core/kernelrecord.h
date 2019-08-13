#include <reflection.h>
#include <core/allocator.h>
#include <kernel/kernel_texture.h>
#include <kernel/kerneldef.h>
#include <kernel/kernel_globals.h>
#include <kernel/kernel_shader.h>
#include <kernel/kernel_material.h>

namespace Miyuki {
	namespace Core {
		class GraphCompiler;
		class KernelRecord {
			Arc<Allocator> allocator;
			std::vector<Kernel::ImageTexture> textures;
			std::vector<Kernel::Shader> shaderProgram;
			std::vector<Kernel::Material> materials;
			friend class GraphCompiler;
		public:
			std::shared_ptr<Allocator> getAllocator()const { return allocator; }
			KernelRecord(Arc<Allocator> alloc);
			Kernel::KernelGlobals createKernelGlobals();
		};
	}
}