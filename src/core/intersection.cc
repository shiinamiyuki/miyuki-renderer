#include <core/intersection.hpp>
#include <core/materials/material.h>
#include <core/mesh.h>
namespace Miyuki {
	namespace Core {
		Spectrum Intersection::Le(const Ray& ray) {
			if (Vec3f::dot(Ns, ray.d) < 0)
				return primitive->material()->emission->eval(ShadingPoint(textureUV)).toFloat();
			return {};
		}
		Spectrum Intersection::Le(const Vec3f& wi) {
			if (Vec3f::dot(Ns, wi) < 0)
				return primitive->material()->emission->eval(ShadingPoint(textureUV)).toFloat();
			return {};
		}

	}
}
