#include <core/lights/area.h>
#include <core/materials/material.h>
#include <math/sampling.h>

namespace Miyuki {
	namespace Core {
		Float AreaLight::power() const {
			return primitive->material() ? primitive->material()->emission->average().toFloat() * primitive->area(): 0.0f;
		}
		Spectrum AreaLight::L(const Ray& ray) const {
			Intersection isct;
			primitive->intersect(ray, &isct);
			return primitive->material() ?
				primitive->material()->emission->eval(ShadingPoint(isct.uv)).toFloat() : Spectrum();
		}

		void AreaLight::sampleLi(const Intersection& isct, LightSamplingRecord& record, VisibilityTester* tester) const {
			Point2f uv;
			auto p = UniformTriangleSampling(record.u, primitive->v(0), primitive->v(1), primitive->v(2),&uv);
			Vec3f w = (p - isct.p);
			auto dist = w.length();
			auto invDist2 = 1 / (dist * dist);
			w /= dist;
			tester->shadowRay = Ray(p, -1 * w, RayBias, dist + 10 * RayBias);
			tester->geomId = isct.geomId;
			tester->primId = isct.primId;

			// convert area to solid angle
			Float solidAngle = primitive->area() * -Vec3f::dot(w, primitive->Ng()) * invDist2;
			record.p = p;
			record.pdf = std::max<Float>(0.0f, 1.0f / solidAngle);
			record.wi = w;
			record.Le = primitive->material()->emission->eval(ShadingPoint(uv)).toVec3f();
		}

		Float AreaLight::pdfLi(const Intersection& intersection, const Vec3f& wi) const {
			Ray ray(intersection.p, wi);
			Intersection isct;
			if (!primitive->intersect(ray, &isct)) {
				return 0.0f;
			}
			Float invDist2 = 1.0f / (isct.hitDistance() * isct.hitDistance());
			Float solidAngle = primitive->area() * Vec3f::absDot(wi, primitive->Ng()) * invDist2;
			return 1.0f / solidAngle;
		}
	}
}