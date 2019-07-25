#include <core/integrators/ao.h>


namespace Miyuki {
	namespace Core {
		void AOIntegrator::Li(const IntegratorContext& integratorContext, SamplingContext& ctx) {
			auto& film = *integratorContext.film;
			film.addSample(ctx.cameraSample.pFilm, Spectrum(1));
		}
	}
}