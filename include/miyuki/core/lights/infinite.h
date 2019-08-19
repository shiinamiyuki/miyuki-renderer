#pragma once

#include <core/lights/light.h>
#include <core/shaders/shader.h>
#include <math/distribution2d.h>
#include <math/transform.h>

namespace Miyuki {
	namespace Core {
		class InfiniteAreaLight : public Light {
			Float worldRadius = -1.0f;
			std::unique_ptr<Distribution2D> distribution;
			
			Matrix4x4 trans, invtrans;
		public:
			Vec3f rotation;
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
			virtual void doPreprocess()override;

			Vec4f lightToWorld(const Vec4f& w) const {
				return trans.mult(w);
			}

			Vec4f worldToLight(const Vec4f& w) const  {
				return invtrans.mult(w);
			}
		};
		MYK_IMPL(InfiniteAreaLight, "Light.InfiniteArea");
		MYK_REFL(InfiniteAreaLight, (Light), (shader)(rotation));
	}
}