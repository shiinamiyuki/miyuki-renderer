#ifndef MIYUKI_AREA_H
#define MIYUKI_AREA_H

#include <core/lights/light.h>
#include <core/mesh.h>

namespace Miyuki {
	namespace Core {
		class AreaLight : public Light {
			Primitive* primitive;
		public:
			MYK_CLASS(AreaLight);
			AreaLight(const Primitive* primitive)
				:Light(EArea), primitive(primitive) {}
			 
			virtual Float power() const;

			virtual Spectrum L(const Ray& ray) const;

			virtual void
				sampleLi(const Intersection&, LightSamplingRecord&, VisibilityTester*) const;

			virtual Float pdfLi(const Intersection&, const Vec3f& wi) const = 0;
		};
		MYK_IMPL(AreaLight, "Light.Area");
		MYK_REFL(AreaLight, (Light), MYK_REFL_NIL);
	}
}


#endif