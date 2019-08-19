#ifndef MIYUKI_AO_H
#define MIYUKI_AO_H

#include <core/integrators/samplerintegrator.h>

namespace Miyuki {
	namespace Core {
		class AOIntegrator final : public SamplerIntegrator {
		public:
			
			Float occlusionDistance = 1e5;
			MYK_CLASS(AOIntegrator);
			void Li(Intersection* isct, const IntegratorContext& context, SamplingContext&)noexcept override;
			
			AOIntegrator() :SamplerIntegrator() {}
		private:
			
		};
		MYK_REFL(AOIntegrator, (SamplerIntegrator), (occlusionDistance));
		MYK_IMPL(AOIntegrator,  "Integrator.AO");
	}
}

#endif