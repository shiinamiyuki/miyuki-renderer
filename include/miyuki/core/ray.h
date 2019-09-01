#ifndef MIYUKI_RAY_H
#define MIYUKI_RAY_H
#include <miyuki.h>
#include <core/mediumstack.hpp>

namespace Miyuki {
	namespace Core {
		extern Float RayBias;
		class Medium;

		struct Ray {
			mutable Float tMin, tMax;
			Float time = 0;
			Vec3f o, d;
			MediumStack* mediumStack = nullptr;
			Ray() :tMin(-1), tMax(-1) {}
			Ray(const Vec3f& o, const Vec3f& d)
				: o(o), d(d), tMin(RayBias), tMax(INF), time(0) {}
			Ray(const Vec3f& o, const Vec3f& d, Float tMin, Float tMax = INF, MediumStack* m = nullptr, Float time = 0)
				:o(o), d(d), tMin(tMin), tMax(tMax), time(time), mediumStack(m) {}
			bool valid()const {
				return tMin >= 0;
			}
			bool hasMedium()const {
				return mediumStack&& mediumStack->size() != 0;
			}
			static Ray FromTo(const Vec3f& p0, const Vec3f& p1) {
				auto w = (p1 - p0);
				return Ray(p0, w.normalized(), RayBias, w.length());
			}
		};

		struct RayDifferential : Ray {
			RayDifferential() :Ray() {}
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