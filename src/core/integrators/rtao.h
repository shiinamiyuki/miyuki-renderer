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

#include <api/integrator.h>
#include <api/serialize.hpp>

namespace miyuki::core {
    class RTAO final : public Integrator {
        int spp;
        float occludeDistance = 100000;
    public:
        MYK_DECL_CLASS(RTAO, "RTAO", interface = "Integrator")

        MYK_AUTO_SER(spp, occludeDistance)

        MYK_AUTO_INIT(spp, occludeDistance)

        void render(const std::shared_ptr<Scene> &ptr, const std::shared_ptr<Camera> &sharedPtr,
                    const std::shared_ptr<Sampler> &ptr1, Film &film) override;

    };
}


#endif //MIYUKIRENDERER_RTAO_H
