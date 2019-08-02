#ifndef MIYUKI_PT_H
#define MIYUKI_PT_H

#include <core/integrators/samplerintegrator.h>
#include <core/denoiser/denoiserdriver.h>

namespace Miyuki {
	namespace Core {
		class PathTracerIntegrator : public SamplerIntegrator {
			std::unique_ptr<DenoiserDriver> denoiser;
		public:
			int maxDepth = 5;
			int minDepth = 3;
			Float maxRayIntensity = 10000.0f;
			bool useNEE = true;
			bool denoised = false;
			MYK_CLASS(PathTracerIntegrator);
			void renderStart(const IntegratorContext& context)override;
			void renderEnd(const IntegratorContext& context)override;
			void Li(const IntegratorContext& context, SamplingContext&)override;
		};
		MYK_IMPL(PathTracerIntegrator, "Integrator.Path");
		MYK_REFL(PathTracerIntegrator, (SamplerIntegrator), (useNEE)(maxDepth)(minDepth)(maxRayIntensity)(denoised));
	}
}
#endif