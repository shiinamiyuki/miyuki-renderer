#ifndef MIYUKI_AO_H
#define MIYUKI_AO_H

#include <core/integrators/samplerintegrator.h>

namespace Miyuki {
	namespace Core {
		class AOIntegrator final : public SamplerIntegrator {
		public:
			
			Float occlusionDistance = 1e5;
			MYK_CLASS(AOIntegrator);
			MKY_BASE(SamplerIntegrator);
			void Li(const IntegratorContext& context, SamplingContext&)override;
			
			AOIntegrator() :SamplerIntegrator() {}
		private:
			
		};
		MYK_REFL(AOIntegrator, (occlusionDistance));
		MYK_IMPL(AOIntegrator, (SamplerIntegrator), "Integrator.AO");
	}
}

#endif