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

#ifndef MIYUKIRENDERER_RAY_H
#define MIYUKIRENDERER_RAY_H

#include <miyuki.foundation/defs.h>
#include <miyuki.foundation/math.hpp>


namespace miyuki::core {
    template<class Value>
    struct TRay {
        using Vector = Array<Value, 3>;
        Vector o;
        Vector d;
        Value tMin, tMax;

        TRay() : tMin(-1), tMax(-1) {}

        TRay(const Vector &o, const Vector &d, Value tMin, Value tMax = Value(MaxFloat))
                : o(o), d(d), tMin(tMin), tMax(tMax) {}
    };

    using Ray = TRay<float>;
    using Ray4 = TRay<float4>;
    using Ray8 = TRay<float8>;

    extern Float RayBias;

    class Shape;

    class BSDF;

    class Material;

    struct MeshTriangle;

    template<class Value>
    struct TIntersection {
        using Vector3 = Array<Value, 3>;
        using Vector2 = Array<Value, 2>;
        static const int N = LengthOf<Value>;
        TArray<const MeshTriangle *, N> shape = nullptr;
        TArray<const Material *, N> material = nullptr;
        Value distance = MaxFloat;
        Vector3 wo;
        Vector3 p;
        Vector3 Ns, Ng;
        Vector2 uv;
        CoordinateSystem <Value> localFrame;

        [[nodiscard]] bool hit() const {
            return shape != nullptr;
        }

        void computeLocalFrame() {
            localFrame = CoordinateSystem(Ns);
        }

        [[nodiscard]] Vector3 worldToLocal(const Vector3 &v) const {
            return localFrame.worldToLocal(v);
        }

        [[nodiscard]] Vector3 localToWorld(const Vector3 &v) const {
            return localFrame.localToWorld(v);
        }

        // w should be normalized
        [[nodiscard]] TRay<Value> spawnRay(const Vector3 &w) const {
            auto t = RayBias / abs(dot(w, Ng));
            return TRay<Value>(p, w, t, MaxFloat);
        }

        [[nodiscard]] TRay<Value> spawnTo(const Point3f &p) const {
            return TRay<Value>(this->p, (p - this->p), RayBias, Value(1.0f));
        }
    };

    using Intersection = TIntersection<float>;
    using Intersection4 = TIntersection<float4>;
    using Intersection8 = TIntersection<float8>;

}
#endif //MIYUKIRENDERER_RAY_H
