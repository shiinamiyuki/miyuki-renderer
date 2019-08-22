#include <core/integrators/pt.h>
#include <core/integrators/pathtracer.hpp>
#include <core/materials/mixedmaterial.h> 
#include <core/materials/diffusematerial.h>
#include <core/denoiser/bcddenoiser.h>
#include <core/denoiser/oidndenoiser.h>

namespace Miyuki {
	namespace Core {
		void imageToFilm(const IO::Image& image, Arc<Film>film) {
			for (int j = 0; j < image.height; j++) {
				for (int i = 0; i < image.width; i++) {
					film->addSample(Point2f(i, j), image(i, j));
				}
			}
		}
		void PathTracerIntegrator::renderStart(const IntegratorContext& context) {
			if (denoised) {
				denoiser = makeBox<OIDNDenoiser>();
				denoiser->setup(context.film->imageDimension());
			}
		}
		void PathTracerIntegrator::renderEnd(const IntegratorContext& context) {
			if (denoised) {
				try {
					context.film->writePNG("temp/pre_denoiser_out.png");
					IO::Image image;
					denoiser->denoise(image);
					image.save("temp/post_denoiser_out.png");
					context.film->clear();
					imageToFilm(image, context.film);
				}
				catch (std::exception& e) {
					std::cerr << e.what() << std::endl;
				}
				denoiser = nullptr;
			}
		}
		void PathTracerIntegrator::Li(Intersection* firstIsct, const IntegratorContext& integratorContext, SamplingContext& ctx)noexcept {
			auto& film = *integratorContext.film;
			auto& scene = *integratorContext.scene;
			auto& sampler = *integratorContext.sampler;
			PathTracer pathtracer(scene, ctx, minDepth, maxDepth, useNEE);
			auto record = pathtracer.trace(firstIsct);
			for (int i = 0; i < AOVCount; i++) {
				record.aovs[i] = clampRadiance(record.aovs[i], maxRayIntensity);
			}
			film.addSample(ctx.cameraSample.pFilm, removeNaNs(record.L()));
			if (denoised) {
				denoiser->addSample(ctx.cameraSample.pFilm, record);
			}
		}
	}
}