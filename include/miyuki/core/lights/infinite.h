#pragma once

#include <core/lights/light.h>
#include <core/shaders/shader.h>

namespace Miyuki {
	namespace Core {
		class InfiniteAreaLight : public Light {
		public:
			MYK_CLASS(InfiniteAreaLight);
			Box<Shader> shader;
			InfiniteAreaLight() :Light(Light::Type(EArea|EInfinite)) {}

			virtual Float power() const override;

			virtual Spectrum L(const Ray& ray) const override;

			virtual void
				sampleLi(const Intersection&, LightSamplingRecord&, VisibilityTester*) const override;

			virtual Float pdfLi(const Intersection&, const Vec3f& wi)const override;
		};
		MYK_IMPL(InfiniteAreaLight, "Light.InfiniteArea");
		MYK_REFL(InfiniteAreaLight, (Light), (shader));
	}
}