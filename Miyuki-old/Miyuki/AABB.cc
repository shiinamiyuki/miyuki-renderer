
#include "aabb.h"
using namespace Miyuki;
Float AABB::intersect(const Ray &ray,Float * f) const {
	auto _tMin = (min - ray.o) * ray.invd;
	auto _tMax = (max - ray.o) * ray.invd;
	/*
	auto tMin = ::min(_tMin, _tMax);
	auto tMax = ::max(_tMin, _tMax);
	//if (tMin.x() > tMax.y() || tMin.y() > tMax.x())
	//	return 0;
	//if (tMin.x() > tMax.z() || tMin.z() > tMax.x())
	//		return 0;
	auto t1 = tMin.max();
	auto t2 = tMax.min();
	if (t2 < 0)return 0;
	if (t1 > t2)return 0;
	if(t1 > eps)
	return t1;
	return 0;*/
	
	Float tmin = _tMin.x();
	Float tmax = _tMax.x();
	if (tmin > tmax)
		std::swap(tmin, tmax);
	Float tymin = _tMin.y();
	Float tymax = _tMax.y();
	if (tymin > tymax)
		std::swap(tymin, tymax);
	if (tmin > tymax || tymin > tmax)
		return false;

	if (tymin > tmin)
		tmin = tymin;

	if (tymax < tmax)
		tmax = tymax;
	Float tzmin = _tMin.z();
	Float tzmax = _tMax.z();

	if (tzmin > tzmax) std::swap(tzmin, tzmax);
	if (tmin > tzmax || tzmin > tmax)
		return false;

	if (tzmin > tmin)
		tmin = tzmin;

	if (tzmax < tmax)
		tmax = tzmax;

	if (tmin < eps) {
		return tmax;
	}
	else if (tmax < eps) {
		return tmin;
	}
	else {
		return std::min(tmin, tmax);
	}
	
}

bool AABB::contains(const AABB &box) const {
    return contains(box.min) && contains(box.max);
}

bool AABB::intersect(const AABB &a, const AABB &b) {
    for(unsigned int i =0 ;i<3;i++){
        if(abs(a.center().axis(i) - b.center().axis(i)) * 2<= a.size(i) + b.size(i) + eps){}
        else{return false;}
    }
    return true;
}
