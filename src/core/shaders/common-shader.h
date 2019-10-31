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

#ifndef MIYUKIRENDERER_COMMON_SHADER_H
#define MIYUKIRENDERER_COMMON_SHADER_H

#include <api/shader.h>
#include <api/serialize.hpp>

namespace miyuki::core {
    class FloatShader final : public Shader {
        Float value = 1.0f;
    public:
        MYK_AUTO_SER(value)

        MYK_AUTO_INIT(value)

        MYK_DECL_CLASS(FloatShader, "FloatShader", interface = "Shader")

        [[nodiscard]] Spectrum evaluate(const ShadingPoint &point) const override {
            return miyuki::core::Spectrum(value);
        }
    };

    class RGBShader final : public Shader {
        Vec3f value = 1.0f;
    public:
        MYK_AUTO_SER(value)

        MYK_AUTO_INIT(value)

        MYK_DECL_CLASS(RGBShader, "RGBShader", interface = "Shader")

        [[nodiscard]] Spectrum evaluate(const ShadingPoint &point) const override {
            return miyuki::core::Spectrum(value);
        }
    };
}

#endif //MIYUKIRENDERER_COMMON_SHADER_H
