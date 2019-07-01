#pragma once
#include "ray.h"
#include <utils/ref.hpp>
#include <math/transform.h>
#include <math/spectrum.h>
#include <embree3/rtcore.h>
namespace Miyuki {
	namespace Core {
		class BSDF;
		class BSSRDF;

		struct Primitive;

		// struct handling intersection info
		struct Intersection {
			// Initialized through EmbreeScene::intersection
			Float distance;
			Point2f uv, textureUV;
			Vec3f p;
			Vec3f wo;
			int32_t geomId, primId;

			// Initialized through Scene::postIntersection
			Vec3f Ng;
			Vec3f Ns;

			Ref<const Primitive> primitive;

			CoordinateSystem localFrame;
			BSDF* bsdf = nullptr;
			bool hit() {
				return primId != RTC_INVALID_GEOMETRY_ID && geomId != RTC_INVALID_GEOMETRY_ID;
			}
			Float hitDistance() const {
				return distance;
			}

			bool isSurfaceScatteringEvent()const {
				return bsdf != nullptr;
			}

			bool isVolumeScatteringEvent()const {
				return false;
			}
			bool hasScatteringFunction()const {
				return isSurfaceScatteringEvent() || isVolumeScatteringEvent();
			}

			Vec3f localToWord(const Vec3f& w) const {
				return localFrame.localToWorld(w);
			}

			Vec3f worldToLocal(const Vec3f& w) const {
				return localFrame.worldToLocal(w);
			}

			void computeLocalFrame(const Vec3f& normal) {
				localFrame = CoordinateSystem(normal);
			}
			Spectrum Le(const Ray& ray);
			Spectrum Le(const Vec3f& wi);

			// w in world space
			Ray spawnRay(const Vec3f& w) const {
				return Ray(p, w.normalized());
			}
		};

		struct Intersection4 {
			Intersection isct[4];
		};
	}
}