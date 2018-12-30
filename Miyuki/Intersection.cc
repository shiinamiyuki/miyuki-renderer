#include "Intersection.h"
#include "Primitive.h"
using namespace Miyuki;
void Miyuki::Intersection::reset() { 
	distance = -1;
	if (object)exclude = object;
}
void Miyuki::Intersection::merge(Primitive * o, Float d, const vec3 & n) {
	bool f = false;
	assert(d >= eps);
	if (!hit()) {
		f = true;
	}
	else if (d < distance) {
		f = true;
	}
	if (f && ( o != exclude)) {//avoid self-intersection
		normal = n;
		distance = d;
		object = o;
	}
}
