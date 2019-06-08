#include <core/intersection.hpp>
#include <materials/material.h>
#include <core/mesh.h>
namespace Miyuki {
	Spectrum Intersection::Le(const Ray& ray) {
		/*if (Vec3f::dot(Ns, ray.d) < 0)
			return primitive->material()->emission.evalUV(textureUV);*/
		return {};
	}
	Spectrum Intersection::Le(const Vec3f& wi) {
	/*	if (Vec3f::dot(Ns, wi) < 0)
			return primitive->material()->emission.evalUV(textureUV);*/
		return {};
	}
}
