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

#ifndef MIYUKIRENDERER_RTAO_H
#define MIYUKIRENDERER_RTAO_H

#include <miyuki.renderer/integrator.h>
#include <miyuki.foundation/property.hpp>
#include <miyuki.foundation/serialize.hpp>

namespace miyuki::core {
    class RTAO final : public Integrator {
        int spp;
        float occludeDistance = 100000;

        RenderOutput render(
                const Task<void>::ContFunc &, const RenderSettings &settings,const mpsc::Sender<std::shared_ptr<Film>>& tx);

    public:
        MYK_DECL_CLASS(RTAO, "RTAO", interface = "Integrator");

        MYK_AUTO_SER(spp, occludeDistance)

        MYK_AUTO_INIT(spp, occludeDistance)

        MYK_PROP(spp, occludeDistance)

        virtual Task<RenderOutput> createRenderTask(const RenderSettings &settings,const mpsc::Sender<std::shared_ptr<Film>>& tx) override;
    };
} // namespace miyuki::core

#endif // MIYUKIRENDERER_RTAO_H
