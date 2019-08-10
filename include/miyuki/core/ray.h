#ifndef MIYUKI_RAY_H
#define MIYUKI_RAY_H
#include <miyuki.h>


namespace Miyuki {
	namespace Core {
		extern Float RayBias;

		struct Ray {
			mutable Float tMin, tMax;
			Float time;
			Vec3f o, d;
			Ray() :tMin(-1), tMax(-1) {}
			Ray(const Vec3f& o, const Vec3f& d)
				: o(o), d(d), tMin(RayBias), tMax(INF), time(0) {}
			Ray(const Vec3f& o, const Vec3f& d, Float tMin, Float tMax = INF, Float time = 0)
				:o(o), d(d), tMin(tMin), tMax(tMax), time(time) {}
			bool valid()const {
				return tMin >= 0;
			}
		};

		struct RayDifferential : Ray {
			RayDifferential() :Ray(){}
			RayDifferential(const Ray& ray) :Ray(ray) {}
		};

		struct Ray4 {
			Ray rays[4];
		};

		struct Ray8 {
			Ray rays[8];
		};
	}
}
#endif // MIYUKI_RAY_H