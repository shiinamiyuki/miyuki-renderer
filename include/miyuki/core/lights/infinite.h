#pragma once

#include <core/lights/light.h>
#include <core/shaders/shader.h>
#include <math/distribution2d.h>

namespace Miyuki {
	namespace Core {
		class InfiniteAreaLight : public Light {
			Float worldRadius = -1.0f;
			std::unique_ptr<Distribution2D> distribution;
		public:
			MYK_CLASS(InfiniteAreaLight);
			Box<Shader> shader;
			InfiniteAreaLight() :Light(Light::Type(EArea|EInfinite)) {}

			virtual Float power() const override;

			virtual Spectrum L(const Ray& ray) const override;

			virtual void
				sampleLi(const Intersection&, LightSamplingRecord&, VisibilityTester*) const override;

			virtual Float pdfLi(const Intersection&, const Vec3f& wi)const override;

			void setWorldRadius(Float r) {
				worldRadius = r;
			}
			virtual void preprocess()override;
		};
		MYK_IMPL(InfiniteAreaLight, "Light.InfiniteArea");
		MYK_REFL(InfiniteAreaLight, (Light), (shader));
	}
}