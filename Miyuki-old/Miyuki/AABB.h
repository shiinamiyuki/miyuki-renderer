#pragma once

#include "Miyuki.h"
#include "Ray.h"
namespace Miyuki {
	struct AABB {
		vec3 min, max;

		AABB() {}

		AABB(const vec3 &_u, const vec3 &_v) : min(_u), max(_v) {}

		Float intersect(const Ray &ray,Float * f = nullptr) const;

		bool contains(const vec3 &p) const {
			for (int i = 0; i < 3; i++) {
				if (min.v[i] - eps <= p.v[i] && p.v[i] < max.v[i] + eps) {}
				else return false;
			}
			return true;
		}

		vec3 center() const { return 0.5 * (min + max); }

		bool contains(const AABB &box) const;
		double size(unsigned int n)const {
			return max.axis(n) - min.axis(n);
		}
		void merge(const AABB& box) {
		}
		static bool intersect(const AABB &a, const AABB &b);
	};

	
}