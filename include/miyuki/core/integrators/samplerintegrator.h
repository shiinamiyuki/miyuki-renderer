#ifndef MIYUKI_SAMPLER_INTEGRATOR_H
#define MIYUKI_SAMPLER_INTEGRATOR_H

#include <core/integrators/integrator.h>

namespace Miyuki {
	namespace Core {
		class SamplerIntegrator : public ProgressiveRenderer {
		public:
			MYK_ABSTRACT(SamplerIntegrator); 
			SamplerIntegrator():_aborted(false){}
			size_t spp = 16;
			void abort()override {
				_aborted = true;
			}
			virtual void Li(const IntegratorContext& context, SamplingContext&) = 0;
			void renderProgressive(
				const IntegratorContext& context,
				ProgressiveRenderCallback progressiveCallback)override;
		protected:
			std::atomic<bool> _aborted;
		};
		MYK_EXTENDS(SamplerIntegrator, (ProgressiveRenderer));
		MYK_REFL(SamplerIntegrator, (spp));
	}
}

#endif