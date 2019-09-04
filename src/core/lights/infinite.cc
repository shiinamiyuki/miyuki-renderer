#include <core/lights/infinite.h>
#include <math/func.h>
#include <core/intersection.hpp>
#include <utils/thread.h>

namespace Miyuki {
	namespace Core {
		Float InfiniteAreaLight::power() const {
			return shader ? (shader->average().toFloat() * 4 * PI * worldRadius * worldRadius) : 0.0f;
		}

		static Point2f getUV(const Vec3f& wi) {
			auto w = Vec3f(wi.x, wi.z, wi.y);
			auto theta = SphericalTheta(w);
			auto sinTheta = std::sin(theta);
			auto phi = SphericalPhi(w);
			return Point2f(phi * INV2PI, 1.0f - theta * INVPI);
		}

		Spectrum InfiniteAreaLight::L(const Ray& ray) const {
			Point2f uv = getUV(worldToLight(Vec4f(ray.d, 1.0f)));
			return Shader::evaluate(shader, ShadingPoint(uv)).toVec3f();
		}

		void InfiniteAreaLight::sampleLi(const Intersection& isct,
			LightSamplingRecord& record,
			VisibilityTester* tester) const {
			Float mapPdf;
			if (!distribution) {
				mapPdf = 0.0f;
				record.Le = {};
				return;
			}
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
			record.wi = lightToWorld(Vec4f(sinTheta * cosPhi, cosTheta, sinTheta * sinPhi, 1));
			if (sinTheta == 0.0f)
				record.pdf = 0.0f;
			else
				record.pdf = mapPdf / (2 * PI * PI * sinTheta);
			uv[1] = 1.0f - uv[1];
			record.Le = Shader::evaluate(shader, ShadingPoint(uv)).toVec3f();
			tester->shadowRay = Ray(isct.p, record.wi, RayBias);
			//fmt::print("{} {} | {}  {}\n", uv[0], uv[1],record.Le.max(), mapPdf);
		}

		Float InfiniteAreaLight::pdfLi(const Intersection& isct, const Vec3f& wi)const {
			if (!distribution)return 0.0f;
			auto w = worldToLight(Vec4f(wi.x, wi.z, wi.y, 1));
			auto theta = SphericalTheta(w);
			auto sinTheta = std::sin(theta);
			auto phi = SphericalPhi(w);
			if (sinTheta == 0.0f)return 0.0f;
			return distribution->pdf(Point2f(phi * INV2PI, 1.0f - theta * INVPI))
				/ (2 * PI * PI * sinTheta);
		}

		void InfiniteAreaLight::doPreprocess() {
			trans = Matrix4x4::rotation(Vec3f(0, 0, 1), rotation.z);
			trans = trans.mult(Matrix4x4::rotation(Vec3f(0, 1, 0), rotation.x));
			trans = trans.mult(Matrix4x4::rotation(Vec3f(1, 0, 0), -rotation.y));
			Matrix4x4::inverse(trans, invtrans);

			if (!shader) {
				distribution = nullptr;
				return;
			}

			auto resolution = shader->resolution();
			if (resolution[0] * resolution[1] == 0)return;
			std::vector<Float> v(resolution[0] * resolution[1]);
			Thread::ParallelFor(0, resolution[1], [&](uint32_t j, uint32_t) {
				for (size_t i = 0; i < resolution[0]; i++) {
					Point2f uv(i, j);
					uv /= Point2f(resolution[0], resolution[1]);
					uv[1] = 1.0f - uv[1];
					Spectrum L = shader->eval(ShadingPoint(uv)).toVec3f();;
					v[i + j * resolution[0]] = L.luminance();
				}
			}, 64);
			distribution = std::make_unique<Distribution2D>(&v[0], resolution[0], resolution[1]);

		}
	}
}