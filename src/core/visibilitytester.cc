#include <core/visibilitytester.h>
#include <core/intersection.hpp>
#include <core/scene.h>
#include <core/samplers/sampler.h>

namespace Miyuki {
	namespace Core {
		bool VisibilityTester::visible(Scene& scene) {
			Intersection intersection;
			Ray ray = Ray::FromTo(p0, p1);
			if (!scene.intersect(ray, &intersection)) {
				return true;
			}
			if (geomId != -1 && primId != -1) {
				if (intersection.geomId == geomId && intersection.primId == primId)
					return true;
			}
			return false;
		}

		Spectrum VisibilityTester::Tr(Scene& scene, Sampler& sampler) {
			Ray ray = Ray::FromTo(p0, p1);
			Spectrum Tr;
			while (true) {
				Intersection isct;
				bool hit = scene.intersect(ray, &isct);
				if(hit && isct.primitive->material()){
					return Spectrum(0.0f);
				}
				if (ray.medium) {
					Tr *= ray.medium->Tr(ray, sampler);
				}
				if (!hit)break;
				ray = isct.spawnRay(ray.d);
			}
			return Tr;
		}
	}
}