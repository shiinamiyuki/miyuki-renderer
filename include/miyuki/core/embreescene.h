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
		ssize_t GetEmbreeMemoryUsage();
		class Scene;

		class EmbreeScene final: public Accelerator {
		public:
			MYK_CLASS(EmbreeScene);

			EmbreeScene();

			void commit();

			void addMesh(std::shared_ptr<Mesh> mesh, uint32_t id)override;

			inline bool intersect(const Ray& ray, Intersection* isct)override;
			inline virtual void intersect4(const Ray4& ray, Intersection4* isct)override;
			inline virtual void intersect8(const Ray8& ray, Intersection8* isct)override;
			RTCScene getRTCScene() {
				return scene;
			}
			~EmbreeScene();

			virtual Bound3f getWorldBound()const override;
			virtual void  detachGeometry(int geomId)override;
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
		inline RTCRay4 toRTCRay4(const Ray4& ray) {
			RTCRay4 ray4;
			for (int i = 0; i < 4; i++) {
				ray4.dir_x[i] = ray.rays[i].d.x;
				ray4.dir_y[i] = ray.rays[i].d.y;
				ray4.dir_z[i] = ray.rays[i].d.z;
				ray4.org_x[i] = ray.rays[i].o.x;
				ray4.org_y[i] = ray.rays[i].o.y;
				ray4.org_z[i] = ray.rays[i].o.z;
				ray4.tfar[i] = ray.rays[i].tMax;
				ray4.tnear[i] = ray.rays[i].tMin;
			}
			return ray4;
		}

		inline RTCRay8 toRTCRay8(const Ray8& ray) {
			RTCRay8 ray8;
			for (int i = 0; i < 8; i++) {
				ray8.dir_x[i] = ray.rays[i].d.x;
				ray8.dir_y[i] = ray.rays[i].d.y;
				ray8.dir_z[i] = ray.rays[i].d.z;
				ray8.org_x[i] = ray.rays[i].o.x;
				ray8.org_y[i] = ray.rays[i].o.y;
				ray8.org_z[i] = ray.rays[i].o.z;
				ray8.tfar[i] = ray.rays[i].tMax;
				ray8.tnear[i] = ray.rays[i].tMin;
			}
			return ray8;
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

		inline void EmbreeScene::intersect4(const Ray4& ray, Intersection4* isct4) {
			RTCRayHit4 rayHit;
			rayHit.ray = toRTCRay4(ray);
			for (int i = 0; i < 4; i++) {
				rayHit.hit.geomID[i] = RTC_INVALID_GEOMETRY_ID;
				rayHit.hit.primID[i] = RTC_INVALID_GEOMETRY_ID;
				rayHit.hit.instID[0][i] = RTC_INVALID_GEOMETRY_ID;
			}
			int valid[4];
			for (int i = 0; i < 4; i++) {
				valid[i] = ray.rays[i].valid() ? -1 : 0;
			}
			RTCIntersectContext context;
			rtcInitIntersectContext(&context);
			rtcIntersect4(valid, scene, &context, &rayHit);

			for (int i = 0; i < 4; i++) {
				auto isct = &isct4->isct[i];
				isct->wo = -ray.rays[i].d;
				isct->primId = rayHit.hit.primID[i];
				isct->geomId = rayHit.hit.geomID[i];
				isct->distance = rayHit.ray.tfar[i];
				if (!isct->hit()) {
					continue;
				}
				isct->primitive = nullptr;
				isct->uv = Point2f(rayHit.hit.u[i], rayHit.hit.v[i]);
				isct->p = ray.rays[i].o + isct->hitDistance() * ray.rays[i].d;
			}
		}

		inline void EmbreeScene::intersect8(const Ray8& ray, Intersection8* isct8) {
			RTCRayHit8 rayHit;
			rayHit.ray = toRTCRay8(ray);
			for (int i = 0; i < 8; i++) {
				rayHit.hit.geomID[i] = RTC_INVALID_GEOMETRY_ID;
				rayHit.hit.primID[i] = RTC_INVALID_GEOMETRY_ID;
				rayHit.hit.instID[0][i] = RTC_INVALID_GEOMETRY_ID;
			}
			int valid[8];
			for (int i = 0; i < 8; i++) {
				valid[i] = ray.rays[i].valid() ? -1 : 0;
			}
			RTCIntersectContext context;
			rtcInitIntersectContext(&context);
			rtcIntersect8(valid, scene, &context, &rayHit);

			for (int i = 0; i < 8; i++) {
				auto isct = &isct8->isct[i];
				isct->wo = -ray.rays[i].d;
				isct->primId = rayHit.hit.primID[i];
				isct->geomId = rayHit.hit.geomID[i];
				isct->distance = rayHit.ray.tfar[i];
				if (!isct->hit()) {
					continue;
				}
				isct->primitive = nullptr;
				isct->uv = Point2f(rayHit.hit.u[i], rayHit.hit.v[i]);
				isct->p = ray.rays[i].o + isct->hitDistance() * ray.rays[i].d;
			}
		}
	}	
}

#endif //MIYUKI_EMBREESCENE_H
