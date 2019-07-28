#include <core/integrators/pt.h>
#include <core/scene.h>

namespace Miyuki {
	namespace Core {
		void PathTracerIntegrator::Li(const IntegratorContext& integratorContext, SamplingContext&ctx) {
			auto& film = *integratorContext.film;
			auto& scene = *integratorContext.scene;
			auto& sampler = *integratorContext.sampler;
			Intersection isct;
			Spectrum L;
			if (scene.intersect(ctx.primary, &isct)) {
				auto& emission = isct.primitive->material()->emission;
				if (emission)
					L = Vec3f(1, 1, 1);
			}
			film.addSample(ctx.cameraSample.pFilm, L);
		}
	}
}