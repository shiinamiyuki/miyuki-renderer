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
			Spectrum AO;
			if (scene.intersect(ctx.primary, &isct)) {
				CosineHemispherePDF pdf;
				Float p;
				auto w = isct.localToWord(pdf.sample(sampler.get2D(), &p));
				Ray ray = isct.spawnRay(w);
				if (!scene.intersect(ray, &isct) || isct.distance >= occlusionDistance) {
					AO = Spectrum(1);
				}
			}
			film.addSample(ctx.cameraSample.pFilm,  AO);
		}
	}
}