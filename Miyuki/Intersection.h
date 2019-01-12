#pragma once
#include "Miyuki.h"
namespace Miyuki {
	class Primitive;
	struct Intersection {
		vec3 normal;
		Float distance;
		Primitive * object, *exclude;
		bool test;
		Intersection():distance(-1),object(nullptr), exclude(nullptr),test(false){}
		void reset();
		bool hit()const { return distance >= eps; }
		void merge(Primitive * o, Float d, const vec3 & n);
	};
}