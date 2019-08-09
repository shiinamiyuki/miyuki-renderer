#include <core/kernelrecord.h>

namespace Miyuki {
	namespace Core {
		Kernel::KernelGlobals KernelRecord::createKernelGlobals() {
			Kernel::KernelGlobals globals;
			globals.program.program = &shaderProgram[0];
			globals.program.length = shaderProgram.size();
			return globals;
		}
	}
}