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

#include "common-shader.h"
#include <miyuki.foundation/imageloader.h>
#include <PerlinNoise.hpp>
#include <miyuki.foundation/log.hpp>

namespace miyuki::core {
    [[nodiscard]] Spectrum ImageTextureShader::evaluate(const ShadingPoint &point) const {
        return image ?  (*image)(mod(point.texCoord,vec2(1))) : Spectrum(0);
    }

    void ImageTextureShader::preprocess() {
        image = ImageLoader::getInstance()->loadRGBAImage(fs::path(imagePath));
        if(!image){
            log::log("Warning: ImageShader[{}] is not loaded correctly\n",imagePath);
        }
    }

    static const siv::PerlinNoise perlin(0);

    Spectrum NoiseShader::evaluate(const miyuki::core::ShadingPoint &point) const {
        return Spectrum(perlin.noise0_1(scale * point.texCoord.x, scale * point.texCoord.y));
    }
}