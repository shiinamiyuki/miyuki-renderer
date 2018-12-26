#pragma once
#include "Miyuki.h"
namespace Miyuki {
	class Primitive;
	struct Intersection {
		vec3 normal;
		Float distance;
		Primitive * object, *exclude;
		Intersection():distance(-1),object(nullptr), exclude(nullptr){}
		void reset() { distance = -1; if (object)exclude = object; }
		bool hit()const { return distance >= eps; }
		void merge(Primitive * o, Float d, const vec3 & n) {
			bool f = false;
			assert(d >= eps);
			if (!hit()) {
				f = true;
			}
			else if (d < distance) {
				f = true;
			}
			if (f && o != exclude) {//avoid self-intersection
				normal = n;
				distance = d;
				object = o;
			}
		}
	};
}