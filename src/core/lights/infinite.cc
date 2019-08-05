#include <core/lights/infinite.h>
#include <math/func.h>
#include <core/intersection.hpp>

namespace Miyuki {
	namespace Core {
		Float InfiniteAreaLight::power() const {
			return shader ? (shader->average().toFloat() * 4 * PI * worldRadius * worldRadius) : 0.0f;
		}

		static Point2f getUV(const Vec3f& wi) {
			auto w = Vec3f(wi.x(), wi.z(), wi.y());
			auto theta = SphericalTheta(w);
			auto sinTheta = std::sin(theta);
			auto phi = SphericalPhi(w);
			return Point2f(phi * INV2PI, 1.0f - theta * INVPI);
		}

		Spectrum InfiniteAreaLight::L(const Ray& ray) const {
			Point2f uv = getUV(ray.d);
			return Shader::evaluate(shader, ShadingPoint(uv)).toVec3f();
		}

		void InfiniteAreaLight::sampleLi(const Intersection& isct,
			LightSamplingRecord& record,
			VisibilityTester* tester) const {
			Float mapPdf;
			auto uv = distribution->sampleContinuous(record.u, &mapPdf);
			if (mapPdf == 0.0f) {
				record.Le = {};
				record.pdf = 0.0f;
				return;
			}
			auto theta = uv[1] * PI;
			auto phi = uv[0] * 2.0f * PI;
			Float cosTheta = std::cos(theta);
			Float sinTheta = std::sin(theta);
			Float sinPhi = std::sin(phi);
			Float cosPhi = std::cos(phi);
			record.wi = Vec3f(sinTheta * cosPhi, cosTheta, sinTheta * sinPhi);
			if (sinTheta == 0.0f)
				record.pdf = 0.0f;
			else
				record.pdf = mapPdf / (2 * PI * PI * sinTheta);
			uv[1] = 1.0f - uv[1];
			record.Le = Shader::evaluate(shader, ShadingPoint(uv)).toVec3f();
			tester->shadowRay = Ray(isct.p, record.wi, RayBias);
			fmt::print("{} {} | {}  {}\n", uv[0], uv[1],record.Le.max(), mapPdf);
		}

		Float InfiniteAreaLight::pdfLi(const Intersection&isct, const Vec3f& wi)const {
			auto w = Vec3f(wi.x(), wi.z(), wi.y());
			auto theta = SphericalTheta(w);
			auto sinTheta = std::sin(theta);
			auto phi = SphericalPhi(w);
			if (sinTheta == 0.0f)return 0.0f;
			return distribution->pdf(Point2f(phi * INV2PI, 1.0f - theta * INVPI))
				/ (2 * PI * PI * sinTheta);
		}

		void InfiniteAreaLight::preprocess() {
			if (!shader)return;
			auto resolution = shader->resolution();
			if (resolution[0] * resolution[1] == 0)return;
			std::vector<Float> v(resolution[0] * resolution[1]);
			for (int j = 0; j < resolution[1]; j++) {
				for (int i = 0; i < resolution[0]; i++) {				
					Point2f uv(i, j);
					uv /= Point2f(resolution[0],resolution[1]);
					uv[1] = 1.0f - uv[1];
					Spectrum L = shader->eval(ShadingPoint(uv)).toVec3f();;
					v[i + j * resolution[0]] = L.luminance(); 
				}
			}
			distribution = std::make_unique<Distribution2D>(&v[0], resolution[0], resolution[1]);
		}
	}
}