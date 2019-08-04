#include <core/lights/infinite.h>
#include <math/func.h>

namespace Miyuki {
	namespace Core {
		Float InfiniteAreaLight::power() const {
			return shader ? shader->average().toFloat() : 0.0f;
		}


		Spectrum InfiniteAreaLight::L(const Ray& ray) const {
			Point2f uv;
			uv[0] = 0.5f + std::atan2(ray.d.z(), ray.d.x()) / (2 * PI);
			uv[1] = 1.0f - (0.5f - std::asin(ray.d.y()) * INVPI);
			return Shader::evaluate(shader, ShadingPoint(uv)).toVec3f();
		}

		void InfiniteAreaLight::sampleLi(const Intersection&,
			LightSamplingRecord&,
			VisibilityTester*) const {
			// NOT IMPLEMEMTED
			std::abort(); 
		}

		Float InfiniteAreaLight::pdfLi(const Intersection&, const Vec3f& wi)const {
			// NOT IMPLEMEMTED
			return 0.0f;
		}

	}
}