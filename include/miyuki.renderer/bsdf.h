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

#ifndef MIYUKIRENDERER_BSDF_H
#define MIYUKIRENDERER_BSDF_H

#include <miyuki.foundation/interfaces.h>
#include <miyuki.foundation/spectrum.h>
#include <miyuki.serialize/serialize.hpp>


namespace miyuki::core {
    struct BSDFSample;
    struct ShadingPoint;

    class BSDF : public serialize::Serializable{
    public:
        MYK_INTERFACE(BSDF, "BSDF")
        enum Type : int {
            ENone = 0,
            EDiffuse = 1ULL,
            EGlossy = 1ULL << 1U,
            EReflection = 1ULL << 2U,
            ETransmission = 1ULL << 3U,
            ESpecular = 1ULL << 4U,
            EAll = EDiffuse | EGlossy | ETransmission | ESpecular | EReflection,
            EAllButSpecular = EAll & ~ESpecular
        };

        [[nodiscard]] virtual Type getBSDFType() const = 0;

        [[nodiscard]] virtual Spectrum evaluate(const ShadingPoint &, const Vec3f &wo, const Vec3f &wi) const = 0;

        [[nodiscard]] virtual Float evaluatePdf(const ShadingPoint &, const Vec3f &wo, const Vec3f &wi) const = 0;

        virtual void sample(Point2f u, const ShadingPoint &, BSDFSample &sample) const = 0;

        [[nodiscard]] virtual bool isSpecular() const {
            return getBSDFType() & ESpecular;
        }
    };

    struct BSDFSample {
        Vec3f wo;
        Vec3f wi;
        float pdf = 0;
        Spectrum f;
        BSDF::Type sampledType = BSDF::ENone;
    };

}
#endif //MIYUKIRENDERER_BSDF_H
