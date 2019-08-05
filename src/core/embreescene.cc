//
// Created by Shiina Miyuki on 2019/2/28.
//

#include <core/embreescene.h>

namespace Miyuki {
	namespace Core {
		RTCDevice rtcDevice = nullptr;

		void Init() {
			GetEmbreeDevice();
		}

		void Exit() {
			rtcReleaseDevice(rtcDevice);
		}

		RTCDevice GetEmbreeDevice() {
			if (!rtcDevice) {
				rtcDevice = rtcNewDevice(nullptr);
			}
			Assert(rtcDevice);
			return rtcDevice;
		}

		void EmbreeScene::commit() {
			rtcCommitScene(scene);
		}

		EmbreeScene::EmbreeScene() {
			scene = rtcNewScene(GetEmbreeDevice());
		}

		void EmbreeScene::addMesh(std::shared_ptr<Mesh> mesh, uint32_t id) {
			RTCGeometry rtcMesh = rtcNewGeometry(GetEmbreeDevice(), RTC_GEOMETRY_TYPE_TRIANGLE);
			auto vertices =
				(Float*)rtcSetNewGeometryBuffer(rtcMesh,
					RTC_BUFFER_TYPE_VERTEX,
					0,
					RTC_FORMAT_FLOAT3,
					sizeof(Float) * 3,
					mesh->vertices.size());
			auto triangles = (uint32_t*)rtcSetNewGeometryBuffer(rtcMesh,
				RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3,
				sizeof(uint32_t) * 3,
				mesh->primitives.size());
			for (int32_t i = 0; i < mesh->primitives.size(); i++) {
				for (int32_t j = 0; j < 3; j++)
					triangles[3 * i + j] = (uint32_t)mesh->primitives[i].vertices[j];
			}
			for (int32_t i = 0; i < mesh->vertices.size(); i++) {
				auto& v = mesh->vertices[i];
				for (int32_t j = 0; j < 3; j++)
					vertices[3 * i + j] = v[j];
			}
			rtcCommitGeometry(rtcMesh);
			rtcAttachGeometryByID(scene, rtcMesh, id);
			//rtcReleaseGeometry(rtcMesh);
#if USE_EMBREE_GEOMETRY == 1
			mesh->rtcGeometry = rtcMesh;
			mesh->accelerator = this;
			mesh->vertices.clear();
			mesh->geomId = id;
			decltype(mesh->vertices) dummy;
			std::swap(dummy, mesh->vertices);
#endif
		}

		RTCRay toRTCRay(const Ray& _ray) {
			RTCRay ray;
			auto _o = _ray.o + EPS * _ray.d;
			ray.dir_x = _ray.d.x();
			ray.dir_y = _ray.d.y();
			ray.dir_z = _ray.d.z();
			ray.org_x = _o.x();
			ray.org_y = _o.y();
			ray.org_z = _o.z();
			ray.tnear = _ray.tMin;
			ray.tfar = _ray.tMax;
			ray.flags = 0;
			return ray;
		}

		bool EmbreeScene::intersect(const Ray& ray, Intersection* isct) {
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

		EmbreeScene::~EmbreeScene() {
			rtcReleaseScene(scene);
		}
		Bound3f EmbreeScene::getWorldBound()const {
			RTCBounds bounds;
			rtcGetSceneBounds(scene, &bounds);
			return Bound3f(
				 Point3f(bounds.lower_x, bounds.lower_y, bounds.lower_z),
				Point3f(bounds.upper_x, bounds.upper_y, bounds.upper_z));
		}
	}
}