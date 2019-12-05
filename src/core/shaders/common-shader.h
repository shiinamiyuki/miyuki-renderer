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

#include <miyuki.renderer/shader.h>
#include <miyuki.foundation/serialize.hpp>
#include <miyuki.foundation/image.hpp>
#include <miyuki.foundation/property.hpp>

namespace miyuki::core {
    class FloatShader final : public Shader {
        Float value = 1.0f;
    public:
        FloatShader() = default;

        FloatShader(float v) : value(v) {}

        MYK_AUTO_SER(value)

        MYK_AUTO_INIT(value)

        MYK_PROP(value)

        MYK_DECL_CLASS(FloatShader, "FloatShader", interface = "Shader")

        [[nodiscard]] Spectrum evaluate(const ShadingPoint &point) const override {
            return miyuki::core::Spectrum(value);
        }
    };

    class RGBShader final : public Shader {
        RGBSpectrum value = RGBSpectrum(0);
    public:
        RGBShader() = default;

        explicit RGBShader(RGBSpectrum v) : value(v) {}

        MYK_AUTO_SER(value)

        MYK_AUTO_INIT(value)

        MYK_PROP(value)

        MYK_DECL_CLASS(RGBShader, "RGBShader", interface = "Shader")

        [[nodiscard]] Spectrum evaluate(const ShadingPoint &point) const override {
            return miyuki::core::Spectrum(value);
        }
    };


    class ImageTextureShader final : public Shader {
        std::shared_ptr<RGBAImage> image;
        std::string imagePath;
    public:
        MYK_AUTO_SER(imagePath)

        MYK_AUTO_INIT(imagePath)

       // MYK_PROP(imagePath)

        MYK_DECL_CLASS(ImageTextureShader, "ImageTextureShader", interface = "Shader")

        void preprocess() override;

        [[nodiscard]] Spectrum evaluate(const ShadingPoint &point) const override;
    };
}

#endif //MIYUKIRENDERER_COMMON_SHADER_H
