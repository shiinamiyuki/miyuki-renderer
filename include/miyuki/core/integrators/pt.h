#ifndef MIYUKI_PT_H
#define MIYUKI_PT_H

#include <core/integrators/samplerintegrator.h>


namespace Miyuki {
	namespace Core {
		class PathTracerIntegrator : public SamplerIntegrator {
		public:
			int maxDepth = 5;
			int minDepth = 3;
			MYK_CLASS(PathTracerIntegrator);
			void Li(const IntegratorContext& context, SamplingContext&)override;
		};
		MYK_IMPL(PathTracerIntegrator, "Integrator.Path");
		MYK_REFL(PathTracerIntegrator, (SamplerIntegrator), MYK_REFL_NIL);
	}
}
#endif