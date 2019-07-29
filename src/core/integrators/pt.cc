#include <core/integrators/pt.h>
#include <core/integrators/pathtracer.hpp>

namespace Miyuki {
	namespace Core {

		void PathTracerIntegrator::Li(const IntegratorContext& integratorContext, SamplingContext&ctx) {
			auto& film = *integratorContext.film;
			auto& scene = *integratorContext.scene;
			auto& sampler = *integratorContext.sampler;
			Intersection isct;
			Spectrum L;
			if (!scene.intersect(ctx.primary, &isct)) {
				film.addSample(ctx.cameraSample.pFilm, L);
				return;
			}
			int depth = 0;
			while (true) {
				 
			}
			film.addSample(ctx.cameraSample.pFilm, L);
		}
	}
}