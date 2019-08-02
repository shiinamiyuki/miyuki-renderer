#include <core/integrators/pt.h>
#include <core/integrators/pathtracer.hpp>
#include <core/materials/mixedmaterial.h> 
#include <core/materials/diffusematerial.h>

namespace Miyuki {
	namespace Core {
		void PathTracerIntegrator::renderStart(const IntegratorContext& context) {
			if(denoised)
				denoiser = std::make_unique<DenoiserDriver>(
					context.film->imageDimension(), DenoiserHistogramParameters());
		}
		void PathTracerIntegrator::renderEnd(const IntegratorContext& context) {
			if (denoised) {
				try {
					context.film->writePNG("pre_denoiser_out.png");
					IO::Image image;
					denoiser->denoise(image);
					image.save("denoised.png");
				}
				catch (std::exception& e) {
					std::cerr << e.what() << std::endl;
				}
			}
		}
		void PathTracerIntegrator::Li(const IntegratorContext& integratorContext, SamplingContext& ctx) {
			auto& film = *integratorContext.film;
			auto& scene = *integratorContext.scene;
			auto& sampler = *integratorContext.sampler;
			PathTracer pathtracer(scene, ctx, minDepth, maxDepth, useNEE);
			auto record = pathtracer.trace();
			for (int i = 0; i < AOVCount; i++) {
				record.aovs[i] = clampRadiance(record.aovs[i], maxRayIntensity);
			}
			film.addSample(ctx.cameraSample.pFilm, removeNaNs(record.L()));
			if (denoised) {
				for (int i = 0; i < AOVCount; i++) {
					denoiser->addSample((AOVType)i, ctx.cameraSample.pFilm, record.aovs[i]);
				}
			}
		}
	}
}