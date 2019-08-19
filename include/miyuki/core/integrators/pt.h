#ifndef MIYUKI_PT_H
#define MIYUKI_PT_H

#include <core/integrators/samplerintegrator.h>
#include <core/denoiser/denoiser.h>

namespace Miyuki {
	namespace Core {
		class PathTracerIntegrator : public SamplerIntegrator {
			Box<Denoiser> denoiser;
		public:
			int maxDepth = 5;
			int minDepth = 3;
			Float maxRayIntensity = 10000.0f;
			bool useNEE = true;
			bool denoised = false;
			MYK_CLASS(PathTracerIntegrator);
			void renderStart(const IntegratorContext& context)override;
			void renderEnd(const IntegratorContext& context)override;
			void Li(Intersection* isct, const IntegratorContext& context, SamplingContext&)noexcept override;
			PathTracerIntegrator() = default;
			PathTracerIntegrator(const PathTracerIntegrator& pt) {
				minDepth = pt.minDepth;
				maxDepth = pt.maxDepth;
				maxRayIntensity = pt.maxRayIntensity;
				useNEE = pt.useNEE;
				denoised = pt.denoised;
			}
		};
		MYK_IMPL(PathTracerIntegrator, "Integrator.Path");
		MYK_REFL(PathTracerIntegrator, (SamplerIntegrator), (useNEE)(maxDepth)(minDepth)(maxRayIntensity)(denoised));
	}
}
#endif