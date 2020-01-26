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

#ifndef MIYUKIRENDERER_GUIDED_PT_H
#define MIYUKIRENDERER_GUIDED_PT_H

#include <miyuki.renderer/integrator.h>
#include <miyuki.renderer/interfaces.h>
#include <miyuki.foundation/spectrum.h>
#include <miyuki.renderer/shader.h>
#include <miyuki.renderer/ray.h>
#include "sdtree.hpp"

namespace miyuki::core {
    class GuidedPathTracer final : public Integrator {
        int spp = 16;
        int minDepth = 3;
        int maxDepth = 5;
        bool denoise = false;
        bool enableNEE = true;
        int trainingPasses = 64;
    public:
        MYK_DECL_CLASS(GuidedPathTracer, "GuidedPathTracer", interface = "Integrator");

        MYK_SER(spp, minDepth, maxDepth, denoise, enableNEE, trainingPasses)


        Task<RenderOutput>
        createRenderTask(const RenderSettings &settings, const mpsc::Sender<std::shared_ptr<Film>> &tx) override;
    };
}
#endif //MIYUKIRENDERER_GUIDED_PT_H
