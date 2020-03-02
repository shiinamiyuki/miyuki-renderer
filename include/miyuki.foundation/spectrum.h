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

#ifndef MIYUKIRENDERER_SPECTRUM_H
#define MIYUKIRENDERER_SPECTRUM_H

#include <miyuki.foundation/defs.h>
#include <miyuki.foundation/math.hpp>
#include <miyuki.serialize/serialize.hpp>

namespace miyuki::core {

    class RGBSpectrum : public Vec3f {
    public:
        using Vec3f::Vec3f;

        RGBSpectrum(const Vec3f &v) : Vec3f(v) {}

        [[nodiscard]] Float luminance()const{
            return dot(*this, float3(0.2126f,0.7152f,0.0722f));
        }
    };

    using Spectrum = RGBSpectrum;

    inline bool IsBlack(const Spectrum &s) {
        return (s.r() < 0 || s.g() < 0 || s.b() < 0) || (s.r() == 0 && s.g() == 0 && s.b() == 0);
    }

    inline Spectrum RemoveNaN(const Spectrum & s){
        auto removeNan = [=](float x){return std::isnan(x) || std::isinf(x) ? 0 : x;};
        return Spectrum(removeNan(s[0]),
                        removeNan(s[1]),
                        removeNan(s[2]));
    }

}

#endif //MIYUKIRENDERER_SPECTRUM_H
