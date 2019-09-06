#include <core/visibilitytester.h>
#include <core/intersection.hpp>
#include <core/scene.h>
#include <core/samplers/sampler.h>

namespace Miyuki {
	namespace Core {
		bool VisibilityTester::visible(Scene& scene) {
			Intersection intersection;
			if (!scene.intersect(shadowRay, &intersection)) {
				return true;
			}
			return intersection.geomId == geomId && intersection.primId == primId;
		}

		Spectrum VisibilityTester::Tr(Scene& scene, Sampler& sampler) {
			Ray ray = shadowRay;
			MediumStack stack;
			if (shadowRay.hasMedium()) {
				stack = *shadowRay.mediumStack;
			}
			ray.mediumStack = &stack;
			Spectrum Tr;
			while (true) {
				Intersection isct;
				bool hit = scene.intersect(ray, &isct);
				if (hit && isct.primitive->material()) {
					return Spectrum(0.0f);
				}
				if (!hit)break;
				auto medium = isct.primitive->medium();
				if (medium) {
					if (!stack.contains(medium)) {
						if (isct.isEntering()) {
							Tr *= ray.mediumStack->top()->Tr(ray, sampler);
							ray.mediumStack->push(medium);
						}
						else {
							Tr *= medium->Tr(ray, sampler);
							ray.mediumStack->pop();
						}
					}
					else {
						if (isct.isEntering()) {
							//??
							// mesh error
						}
						else {
							if (stack.top() == medium) {
								Tr *= stack.top()->Tr(ray, sampler);
								stack.pop();
							}
							else {
								Tr *= stack.top()->Tr(ray, sampler);
							}
						}
					}
				}
				ray = isct.spawnRay(ray.d);
				ray.mediumStack = &stack;
			}
			return Tr;
		}
	}
}