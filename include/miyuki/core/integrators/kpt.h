#pragma once

#include <core/integrators/integrator.h>
namespace Miyuki {
	namespace Core {
		class KernelPathTracer : public ProgressiveRenderer {
			bool _aborted = false;
		public:
			MYK_CLASS(KernelPathTracer)
			size_t spp = 16;
			void abort()override {
				_aborted = true;
			}
			void renderProgressive(
				const IntegratorContext& context,
				ProgressiveRenderCallback progressiveCallback)override;
		};
		MYK_IMPL(KernelPathTracer, "Integrator.KernelPT");
		MYK_REFL(KernelPathTracer, (ProgressiveRenderer), (spp));
	}
}