#ifndef MIYUKI_LIGHT_H
#define MIYUKI_LIGHT_H

#include <reflection.h>
#include <core/ray.h>
#include <utils/preprocessable.hpp>
#include <core/visibilitytester.h>

namespace Miyuki {
	namespace Core {
		class Scene;
		struct Intersection;
		
		struct LightSamplingRecord {
			Point2f u;
			Vec3f p; // point on light
			Spectrum Le;// sampled emission
			Vec3f wi; // sampled direction, surface -> light
			Float pdf = 0; // sampled pdf
		};

		class Light : public Reflective, public Preprocessable {
		public:
			enum Type {
				EDeltaPosition = 1,
				EDeltaDirection = 2,
				EArea = 4,
				EInfinite = 8,
			};
		private:
			Type lightType;
		public:
			MYK_ABSTRACT(Light);

			Light(Type ty) :lightType(ty) {}

			bool isDelta()const {
				return lightType & (EDeltaDirection | EDeltaPosition);
			}

			virtual Float power() const = 0;

			virtual Spectrum L(const Ray& ray) const = 0;

			virtual void
				sampleLi(const Intersection&, LightSamplingRecord&, VisibilityTester*) const = 0;

			virtual Float pdfLi(const Intersection&, const Vec3f& wi) const = 0;		

			virtual void preprocess()override {}
		};
		MYK_SAVE_LOAD_TRIVIAL(Light::Type);
		MYK_REFL(Light, (Reflective), (lightType));
	}
}

#endif