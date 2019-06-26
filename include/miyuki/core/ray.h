#ifndef MIYUKI_RAY_H
#define MIYUKI_RAY_H
#include <miyuki.h>
#include <math/vec.hpp>
#include <utils/ref.hpp>
namespace Miyuki {
	namespace Core {
		extern Float RayBias;

		struct Ray {
			mutable Float near, far;
			Float time;
			Vec3f o, d;
			Ray() :near(-1), far(-1) {}
			Ray(const Vec3f& o, const Vec3f& d)
				: o(o), d(d), near(RayBias), far(INF), time(0) {}
			Ray(const Vec3f& o, const Vec3f& d, Float near, Float far = INF, Float time = 0)
				:o(o), d(d), near(near), far(far), time(time) {}
		};

		struct RayDifferential : Ray {
			RayDifferential(const Ray& ray) :Ray(ray) {}
		};

		struct Ray4 {
			Ray rays[4];
		};

	}
}
#endif // MIYUKI_RAY_H