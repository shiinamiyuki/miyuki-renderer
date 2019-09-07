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
		Medium* UpdateMediumStack(const Ray& ray, const Intersection& isct) {
			auto medium = isct.primitive->medium();
			auto& stack = *ray.mediumStack;
			Medium* result = nullptr;
			if (medium) {
				if (!stack.contains(medium)) {
					if (isct.isEntering()) {
						if (stack.size() != 0)
							result = stack.top();
						stack.push(medium);
					}
					else {
						result = medium;
						if (stack.size() != 0)
							stack.pop();
					}
				}
				else {
					if (isct.isEntering()) {
						//??
						// mesh error
					}
					else {
						if (stack.top() == medium) {
							result = stack.top();
							stack.pop();
						}
						else {
							// This is because we want to handle medium like Blender foes
							// A medium interface is represented by overlapping mesh boundaries
							result = stack.top();
						}
					}
				}
			}
			return result;
		}
		Spectrum Tr(const Ray& ray, const Intersection& isct, Sampler& sampler) {
			auto medium = isct.primitive->medium();
			auto& stack = *ray.mediumStack;
			Spectrum Tr(1);
			Medium* evaled = UpdateMediumStack(ray, isct);
			if (evaled) {
				Ray _ray = ray;
				_ray.tMax = isct.distance;
				Tr *= evaled->Tr(_ray, sampler);
			}
			//fmt::print("{}\n", Tr.max());
			return Tr;
		}
		Spectrum VisibilityTester::Tr(Scene& scene, Sampler& sampler) {
			Ray ray = shadowRay;
			MediumStack stack;
			if (shadowRay.hasMedium()) {
				stack = *shadowRay.mediumStack;
			}
			ray.mediumStack = &stack;
			Spectrum Tr(1);
			Float dist = 0;
			while (true) {
				Intersection isct;
				bool hit = scene.intersect(ray, &isct);
				if (hit && isct.geomId == geomId && isct.primId == primId) {
					//fmt::print("end {}\n", Tr.max());
					return Tr;
				}
				if (hit && isct.primitive->material()) {
					//fmt::print("fuck\n");
					return Spectrum(0.0f);
				}
				if (!hit)break;

				Tr *= Core::Tr(ray, isct, sampler);
				//fmt::print("{}\n", Tr.max());

				dist += isct.distance;
				ray = isct.spawnRay(ray.d);
				ray.tMax = shadowRay.tMax - dist;
				ray.mediumStack = &stack;
			}
			
			return Tr;
		}
	}
}