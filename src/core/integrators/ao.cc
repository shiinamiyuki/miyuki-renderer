#include <core/integrators/ao.h>
#include <core/scene.h>
#include <core/probablity/commonpdf.hpp>

namespace Miyuki {
	namespace Core {
		void AOIntegrator::Li(const IntegratorContext& integratorContext, SamplingContext& ctx) {
			auto& film = *integratorContext.film;
			auto& scene = *integratorContext.scene;
			auto& sampler = *integratorContext.sampler;
			Intersection isct;
			bool flag = false;
			if (scene.intersect(ctx.primary, &isct)) {
				CosineHemispherePDF pdf;
				auto w = isct.localToWord(pdf.sample(sampler.get2D()));
				Ray ray = isct.spawnRay(w);
				if (!scene.intersect(ray, &isct) || isct.distance >= occlusionDistance) {
					flag = true;
				}
			}
			film.addSample(ctx.cameraSample.pFilm,  flag * Spectrum(1));
		}
	}
}