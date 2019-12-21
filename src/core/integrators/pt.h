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

#ifndef MIYUKIRENDERER_PT_H
#define MIYUKIRENDERER_PT_H

#include <miyuki.renderer/integrator.h>
#include <miyuki.foundation/property.hpp>
#include <miyuki.foundation/serialize.hpp>

namespace miyuki::core {
    class PathTracer final : public Integrator {
        int spp = 16;
        int minDepth = 3;
        int maxDepth = 5;
        bool denoise = false;
    public:
        MYK_DECL_CLASS(PathTracer, "PathTracer", interface = "Integrator");

        MYK_AUTO_SER(spp, minDepth, maxDepth, denoise)

        MYK_AUTO_INIT(spp, minDepth, maxDepth, denoise)

        MYK_PROP(spp, minDepth, maxDepth, denoise)

        virtual Task<RenderOutput>
        createRenderTask(const RenderSettings &settings, const mpsc::Sender<std::shared_ptr<Film>> &tx) override;
    };
} // namespace miyuki::core

#endif //MIYUKIRENDERER_PT_H
