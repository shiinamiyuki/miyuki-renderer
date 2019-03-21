//
// Created by Shiina Miyuki on 2019/2/28.
//

#include "embreescene.h"

namespace Miyuki {
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

    void EmbreeScene::addMesh(std::shared_ptr<Mesh> mesh, int id) {
        RTCGeometry rtcMesh = rtcNewGeometry(GetEmbreeDevice(), RTC_GEOMETRY_TYPE_TRIANGLE);
        auto vertices =
                (Float *) rtcSetNewGeometryBuffer(rtcMesh,
                                                  RTC_BUFFER_TYPE_VERTEX,
                                                  0,
                                                  RTC_FORMAT_FLOAT3,
                                                  sizeof(Float) * 3,
                                                  mesh->vertices.size());
        auto triangles = (uint32_t *) rtcSetNewGeometryBuffer(rtcMesh,
                                                              RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3,
                                                              sizeof(uint32_t) * 3,
                                                              mesh->primitives.size());
        for (int32_t i = 0; i < mesh->primitives.size(); i++) {
            for (int32_t j = 0; j < 3; j++)
                triangles[3 * i + j] = (uint32_t) mesh->primitives[i].vertices[j];
        }
        for (int32_t i = 0; i < mesh->vertices.size(); i++) {
            auto &v = mesh->vertices[i];
            for (int32_t j = 0; j < 3; j++)
                vertices[3 * i + j] = v[j];
        }
        rtcCommitGeometry(rtcMesh);
        rtcAttachGeometryByID(scene, rtcMesh, id);
        rtcReleaseGeometry(rtcMesh);
#if USE_EMBREE_GEOMETRY == 1
        mesh->rtcGeometry = rtcMesh;
        mesh->vertices.clear();
        decltype(mesh->vertices) dummy;
        std::swap(dummy, mesh->vertices);
#endif
    }

    EmbreeScene::~EmbreeScene() {
        rtcReleaseScene(scene);
    }
}