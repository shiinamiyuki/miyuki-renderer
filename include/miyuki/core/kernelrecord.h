#include <reflection.h>
#include <core/allocator.h>
#include <kernel/kernel_texture.h>
#include <kernel/kerneldef.h>
#include <kernel/kernel_globals.h>

namespace Miyuki {
	namespace Core {
		class GraphCompiler;
		class KernelRecord {
			Arc<Allocator> allocator;
			std::vector<Kernel::ImageTexture> textures;
			std::vector<Kernel::Shader*> shaderProgram;
			friend class GraphCompiler;
		public:
			std::shared_ptr<Allocator> getAllocator()const { return allocator; }
			KernelRecord(Arc<Allocator> alloc) :allocator(alloc) {}
			Kernel::KernelGlobals createKernelGlobals();
		};
	}
}