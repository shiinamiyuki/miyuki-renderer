#include <core/integrators/ao.h>
#include <core/scene.h>
#include <core/probablity/commonpdf.hpp>

namespace Miyuki {
	namespace Core {
		void AOIntegrator::Li(Intersection* firstIsct, const IntegratorContext& integratorContext, SamplingContext& ctx) {
			auto& film = *integratorContext.film;
			auto& scene = *integratorContext.scene;
			auto& sampler = *ctx.sampler;
			Intersection isct;
			Spectrum AO;
			Intersection* pIsct;
			if (firstIsct) {
				pIsct = firstIsct;
			}
			else {
				pIsct = &isct;
				scene.intersect(ctx.primary, &isct);
			}
			if (pIsct->hit()) {
				CosineHemispherePDF pdf;
				Float p;
				auto w = pIsct->localToWorld(pdf.sample(sampler.get2D(), &p));
				Ray ray = pIsct->spawnRay(w);
				if (!scene.intersect(ray, &isct) || isct.distance >= occlusionDistance) {
					AO = Spectrum(1);
				}
			}
			film.addSample(ctx.cameraSample.pFilm,  AO);
		}
	}
}