//
// Created by Shiina Miyuki on 2019/2/28.
//

#include <core/embreescene.h>

namespace Miyuki {
	RTCDevice rtcDevice = nullptr;
	RTCDevice Core::GetEmbreeDevice() {
		if (!rtcDevice) {
			rtcDevice = rtcNewDevice(nullptr);
		}
		Assert(rtcDevice);
		return rtcDevice;
	}
#define QUERY_PROP(prop) \
			fmt::print("{}: {}\n",#prop, rtcGetDeviceProperty(Core::GetEmbreeDevice(), prop) ? true : false)		
	void Init() {
		Core::GetEmbreeDevice();
		QUERY_PROP(RTC_DEVICE_PROPERTY_NATIVE_RAY4_SUPPORTED);
		QUERY_PROP(RTC_DEVICE_PROPERTY_NATIVE_RAY8_SUPPORTED);
		QUERY_PROP(RTC_DEVICE_PROPERTY_NATIVE_RAY16_SUPPORTED);
	}

	void Exit() {
		rtcReleaseDevice(rtcDevice);
	}
	
	namespace Core {


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

		void EmbreeScene::detachGeometry(int id) {
			rtcDetachGeometry(scene, id);
		}
	}
}