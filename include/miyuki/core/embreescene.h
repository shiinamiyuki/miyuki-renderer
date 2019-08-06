//
// Created by Shiina Miyuki on 2019/2/28.
//

#ifndef MIYUKI_EMBREESCENE_H
#define MIYUKI_EMBREESCENE_H

#include "miyuki.h"
#include "mesh.h"
#include <core/ray.h>
#include <core/intersection.hpp>
#include <core/accelerators/accelerator.h>
#include <embree3/rtcore.h>

namespace Miyuki {
	namespace Core {
		RTCDevice GetEmbreeDevice();

		class Scene;

		class EmbreeScene final: public Accelerator {
		public:
			MYK_CLASS(EmbreeScene);

			EmbreeScene();

			void commit();

			void addMesh(std::shared_ptr<Mesh> mesh, uint32_t id)override;

			inline bool intersect(const Ray& ray, Intersection* isct)override;

			RTCScene getRTCScene() {
				return scene;
			}
			~EmbreeScene();

			virtual Bound3f getWorldBound()const override;
		private:
			RTCScene scene;
		};
		MYK_IMPL(EmbreeScene, "Accelerator.Embree");
		MYK_REFL(EmbreeScene, (Accelerator), MYK_REFL_NIL);

		inline RTCRay toRTCRay(const Ray& _ray) {
			RTCRay ray;
			auto _o = _ray.o + EPS * _ray.d;
			ray.dir_x = _ray.d.x;
			ray.dir_y = _ray.d.y;
			ray.dir_z = _ray.d.z;
			ray.org_x = _o.x;
			ray.org_y = _o.y;
			ray.org_z = _o.z;
			ray.tnear = _ray.tMin;
			ray.tfar = _ray.tMax;
			ray.flags = 0;
			return ray;
		}

		inline bool EmbreeScene::intersect(const Ray& ray, Intersection* isct) {
			RTCRayHit rayHit;
			rayHit.ray = toRTCRay(ray);
			rayHit.ray.flags = 0;
			rayHit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
			rayHit.hit.primID = RTC_INVALID_GEOMETRY_ID;
			rayHit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;
			RTCIntersectContext context;
			rtcInitIntersectContext(&context);
			rtcIntersect1(scene, &context, &rayHit);

			isct->wo = -ray.d;
			isct->primId = rayHit.hit.primID;
			isct->geomId = rayHit.hit.geomID;
			isct->distance = rayHit.ray.tfar;
			if (!isct->hit()) {
				return false;
			}
			isct->primitive = nullptr;
			isct->uv = Point2f{ rayHit.hit.u, rayHit.hit.v };
			isct->p = ray.o + isct->hitDistance() * ray.d;
			return true;
		}
	}	
}

#endif //MIYUKI_EMBREESCENE_H
