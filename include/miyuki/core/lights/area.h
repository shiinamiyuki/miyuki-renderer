#ifndef MIYUKI_AREA_H
#define MIYUKI_AREA_H

#include <core/lights/light.h>
#include <core/mesh.h>

namespace Miyuki {
	namespace Core {
		class AreaLight : public Light {
			const Primitive* primitive = nullptr;
		public:
			MYK_CLASS(AreaLight);
			AreaLight() :Light(EArea) {}
			AreaLight(const Primitive* primitive)
				:Light(EArea), primitive(primitive) {}
			 
			virtual Float power() const override;

			virtual Spectrum L(const Ray& ray) const override;

			virtual void
				sampleLi(const Intersection&, LightSamplingRecord&, VisibilityTester*) const override;

			virtual Float pdfLi(const Intersection&, const Vec3f& wi)const override;
		};
		MYK_IMPL(AreaLight, "Light.Area");
		MYK_REFL(AreaLight, (Light), MYK_REFL_NIL);
	}
}


#endif