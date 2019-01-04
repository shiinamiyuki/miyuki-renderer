#pragma once
#include "Miyuki.h"
#include "Float4.h"
namespace Miyuki {
	struct Ray {
		vec3 o;
		vec3 d;
		vec3 invd;
		Ray(const vec3 & _o, const vec3&_d) :o(_o), d(_d) { invd = vec3(1,1,1) / d; }
	};

	struct SIMDRay {
		simdVec o;
		simdVec d;
		SIMDRay(const simdVec & _o, const simdVec&_d) :o(_o), d(_d) {}
		SIMDRay(const Ray & ray) {
			for (int i = 0; i < simdVec::width(); i++) {
				o.x[i] = ray.o.x();
				o.y[i] = ray.o.y();
				o.z[i] = ray.o.z();

				d.x[i] = ray.d.x();
				d.y[i] = ray.d.y();
				d.z[i] = ray.d.z();
			}
		}
	};
}