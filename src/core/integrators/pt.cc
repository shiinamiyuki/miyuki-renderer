#include <core/integrators/pt.h>
#include <core/integrators/pathtracer.hpp>
#include <core/materials/mixedmaterial.h> 
#include <core/materials/diffusematerial.h>
namespace Miyuki {
	namespace Core {

		void PathTracerIntegrator::Li(const IntegratorContext& integratorContext, SamplingContext& ctx) {
			auto& film = *integratorContext.film;
			auto& scene = *integratorContext.scene;
			auto& sampler = *integratorContext.sampler;
			PathTracer pathtracer(scene, ctx, minDepth, maxDepth, useNEE);
			auto record = pathtracer.trace();
			
			film.addSample(ctx.cameraSample.pFilm, removeNaNs(record.L()));

			/*Intersection isct;
			Spectrum AO;
			if (scene.intersect(ctx.primary, &isct)) {
				ShadingPoint p(isct.textureUV);
				auto mat = dynamic_cast<MixedMaterial*>(isct.primitive->material());
				film.addSample(ctx.cameraSample.pFilm,Shader::evaluate(
					dynamic_cast<DiffuseMaterial*>(mat->matA.get())->color, p).toVec3f());
			}*/
		}
	}
}