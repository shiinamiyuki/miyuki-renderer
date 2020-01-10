// MIT License
//
// Copyright (c) 2019 椎名深雪
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "embree-backend.h"
#include <miyuki.foundation/log.hpp>
#include <miyuki.renderer/ray.h>
#include <miyuki.renderer/scene.h>

#ifdef MYK_USE_EMBREE
#include <embree3/rtcore.h>
#endif




#define QUERY_PROP(prop) log::log("{}: {}\n", #prop, rtcGetDeviceProperty(device, prop) ? true : false)
namespace miyuki::core {
#ifdef MYK_USE_EMBREE
    class EmbreeAccelerator::Impl {
        RTCDevice device;
        RTCScene rtcScene = nullptr;
        const Scene *scene;

    public:
        Impl() {
            device = rtcNewDevice(nullptr);
            if (rtcGetDeviceError(device) != RTC_ERROR_NONE) {
                MIYUKI_THROW(std::runtime_error, "Failed to create Embree device");
            }
            QUERY_PROP(RTC_DEVICE_PROPERTY_NATIVE_RAY4_SUPPORTED);
            QUERY_PROP(RTC_DEVICE_PROPERTY_NATIVE_RAY8_SUPPORTED);
            QUERY_PROP(RTC_DEVICE_PROPERTY_NATIVE_RAY16_SUPPORTED);
        }

        ~Impl() { rtcReleaseDevice(device); }

        void build(const Scene &scene) {
            if (rtcScene != nullptr) {
                rtcReleaseScene(rtcScene);
            }
            this->scene = &scene;
            rtcScene = rtcNewScene(device);
            for (const auto &mesh : scene.meshes) {
                auto geometry = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
                rtcSetSharedGeometryBuffer(geometry, RTC_BUFFER_TYPE_VERTEX, 0,
                                           RTC_FORMAT_FLOAT3,
                                           &mesh->_vertex_data.position[0][0], 0,
                                           sizeof(mesh->_vertex_data.position[0]),
                                           mesh->_vertex_data.position.size());
                rtcSetSharedGeometryBuffer(geometry, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, &mesh->triangles[0], 0,
                                           sizeof(MeshTriangle), mesh->triangles.size());
                rtcCommitGeometry(geometry);
                rtcAttachGeometry(rtcScene, geometry);
                rtcReleaseGeometry(geometry);
            }
            rtcCommitScene(rtcScene);
        }

        static inline RTCRay toRTCRay(const Ray &_ray) {
            RTCRay ray;
            auto _o = _ray.o;
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

        bool intersect(const Ray &ray, Intersection &isct) {
            RTCRayHit rayHit;
            rayHit.ray = toRTCRay(ray);
            rayHit.ray.flags = 0;
            rayHit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
            rayHit.hit.primID = RTC_INVALID_GEOMETRY_ID;
            rayHit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;
            RTCIntersectContext context;
            rtcInitIntersectContext(&context);
            rtcIntersect1(rtcScene, &context, &rayHit);
            if (rayHit.hit.geomID == RTC_INVALID_GEOMETRY_ID || rayHit.hit.primID == RTC_INVALID_GEOMETRY_ID)
                return false;
            isct.shape = &scene->meshes[rayHit.hit.geomID]->triangles[rayHit.hit.primID];
            isct.Ng = normalize(Vec3f(rayHit.hit.Ng_x, rayHit.hit.Ng_y, rayHit.hit.Ng_z));
            isct.uv = Point2f(rayHit.hit.u, rayHit.hit.v);
            isct.distance = rayHit.ray.tfar;
            isct.p = ray.o + isct.distance * ray.d;
            return true;
        }

        bool occlude(const Ray &ray) {
            RTCRay rtcRay = toRTCRay(ray);
            RTCIntersectContext context;
            rtcInitIntersectContext(&context);
            rtcOccluded1(rtcScene, &context, &rtcRay);
            return rtcRay.tfar < 0;
        }
    };

    EmbreeAccelerator::EmbreeAccelerator() : impl(new Impl()) {}

    void EmbreeAccelerator::build(Scene &scene) { impl->build(scene); }

    bool EmbreeAccelerator::intersect(const Ray &ray, Intersection &isct) { return impl->intersect(ray, isct); }

    bool EmbreeAccelerator::occlude(const Ray &ray) {
        return impl->occlude(ray);
    }

    EmbreeAccelerator::~EmbreeAccelerator() { delete impl; }


#else
    EmbreeAccelerator::EmbreeAccelerator(){}
    EmbreeAccelerator::~EmbreeAccelerator() {}
    bool EmbreeAccelerator::occlude(const Ray &ray) {
        MIYUKI_NOT_IMPLEMENTED();
    }
    void miyuki::core::EmbreeAccelerator::build(miyuki::core::Scene &scene) { MIYUKI_NOT_IMPLEMENTED(); }

    bool miyuki::core::EmbreeAccelerator::intersect(const miyuki::core::Ray &ray, miyuki::core::Intersection &isct) {
        MIYUKI_NOT_IMPLEMENTED();
        return false;
    }
#endif
}