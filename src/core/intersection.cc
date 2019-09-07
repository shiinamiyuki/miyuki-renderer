#include <core/intersection.hpp>
#include <core/materials/material.h>
#include <core/mesh.h>
#include <core/medium/medium.h>
namespace Miyuki {
	namespace Core {
		Spectrum Intersection::Le(const Ray& ray) {
			if (primitive->material()) {
				if (Vec3f::dot(Ns, ray.d) < 0)
					return Shader::evaluate(primitive->material()->emission, ShadingPoint(textureUV)).toVec3f();
			}
			return {};
		}
		Spectrum Intersection::Le(const Vec3f& wi) {
			if (primitive->material()) {
				if (Vec3f::dot(Ns, wi) < 0)
					return Shader::evaluate(primitive->material()->emission, ShadingPoint(textureUV)).toVec3f();
			}
			return {};
		}
		Material* Intersection::material()const {
			return primitive->material();
		}
		Medium* Intersection::medium()const {
			return primitive->medium();
		}
		bool Intersection::isEntering()const {
			return Vec3f::dot(wo, primitive->Ng()) > 0.0f;
		}
	}
}
