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
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef MIYUKIRENDERER_BIDIR_HPP
#define MIYUKIRENDERER_BIDIR_HPP

#include <miyuki.foundation/defs.h>
#include <miyuki.foundation/array.hpp>
#include <miyuki.foundation/log.hpp>
#include <miyuki.renderer/bsdf.h>
#include <miyuki.renderer/ray.h>
#include <miyuki.renderer/camera.h>
#include <miyuki.renderer/sampler.h>
#include <miyuki.renderer/sampling.h>
#include <miyuki.renderer/scene.h>
#include <miyuki.renderer/lightdistribution.h>
#include <miyuki.renderer/light.h>


namespace miyuki::core::bidir {
    struct CameraRecord {
        Camera *camera = nullptr;
        CameraSample sample;
    };
    struct LightRecord {
        Light *light = nullptr;
        LightRaySample sample;
    };
    struct SurfaceRecord {
        Intersection *intersection;
        ShadingPoint sp;
    };

    struct Vertex {
        enum Type {
            ENone,
            ECamera,
            ELight,
            ESurface
        };
        Type type;
        union {
            CameraRecord cameraRecord;
            LightRecord lightRecord;
            SurfaceRecord surfaceRecord{};
        };

        Float pdfFwd = 0;
        Float pdfRev = 0;
        bool isSpecular = false;
        Spectrum beta;

        Vertex() : surfaceRecord(), type(ENone) {}

        explicit Vertex(const CameraRecord &cameraRecord1) : cameraRecord(cameraRecord1), type(ECamera) {}

        explicit Vertex(const LightRecord &lightRecord1) : lightRecord(lightRecord1), type(ELight) {}

        explicit Vertex(const SurfaceRecord &surfaceRecord1) : surfaceRecord(surfaceRecord1), type(ESurface) {}

        [[nodiscard]] Point3f p() const {
            switch (type) {
                case Type::ESurface: {
                    return surfaceRecord.intersection->p;
                }
                case Type::ELight: {
                    return lightRecord.sample.ray.o;
                }
                case Type::ECamera: {
                    return cameraRecord.sample.ray.o;
                }
                default: {
                    MIYUKI_CHECK(false);
                    return Vec3f(0);
                }
            }
        }

        [[nodiscard]] Normal3f Ng() const {
            switch (type) {
                case Type::ESurface: {
                    return surfaceRecord.intersection->Ng;
                }
                case Type::ELight: {
                    return lightRecord.sample.normal;
                }
                default: {
                    MIYUKI_CHECK(false);
                    return Vec3f(0);
                }
            }
        }

        [[nodiscard]] Normal3f Ns() const {
            switch (type) {
                case Type::ESurface: {
                    return surfaceRecord.intersection->Ns;
                }
                case Type::ELight: {
                    return lightRecord.sample.normal;
                }
                default: {
                    MIYUKI_CHECK(false);
                    return Vec3f(0);
                }
            }
        }


        [[nodiscard]] bool isConnectable() const {
            switch (type) {
                case Type::ESurface: {
                    return !isSpecular;
                }
                case Type::ELight: {
                    return true;
                }
                case Type::ECamera: {
                    return true;
                }
                default: {
                    MIYUKI_CHECK(false);
                    return false;
                }
            }
        }


        [[nodiscard]] bool onSurface() const {
            return true;
        }

        [[nodiscard]] Float convertSAToArea(Float pdf, const Vertex &next) const {
            auto wi = next.p() - p();
            auto dist2 = dot(wi, wi);
            auto invDist2 = 1.0f / dist2;
            if (onSurface()) {
                pdf *= std::abs(dot(Ng(), wi * invDist2));
            }
            return pdf * invDist2;
        }

        Float pdf3(Scene &scene, const Vertex* prev, const Vertex & next){

        }

        [[nodiscard]] Spectrum f(const Vertex &next) const {
            auto wi = next.p() - p();
            auto dist2 = dot(wi, wi);
            if (dist2 == 0)return Spectrum(0);
            wi /= std::sqrt(dist2);
            switch (type) {
                case Type::ESurface: {
                    auto wo = surfaceRecord.intersection->worldToLocal(surfaceRecord.intersection->wo);
                    wi = surfaceRecord.intersection->worldToLocal(wi);
                    return surfaceRecord.intersection->material->bsdf->evaluate(surfaceRecord.sp, wo, wi);
                }
                default: {
                    MIYUKI_CHECK(false);
                    return Spectrum(0);
                }
            }
        }


    };

    struct LightPath : Array<Vertex> {
        LightPath(Vertex *buffer, size_t maxLength) : Array(buffer, maxLength) {}
    };
}
#endif //MIYUKIRENDERER_BIDIR_HPP
