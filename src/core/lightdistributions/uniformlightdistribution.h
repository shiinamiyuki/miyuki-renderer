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

#ifndef MIYUKIRENDERER_UNIFORMLIGHTDISTRIBUTION_H
#define MIYUKIRENDERER_UNIFORMLIGHTDISTRIBUTION_H
#include <miyuki.renderer/lightdistribution.h>
#include <miyuki.foundation/interfaces.h>
#include <miyuki.renderer/sampler.h>
#include <miyuki.renderer/scene.h>
#include <miyuki.renderer/light.h>
#include <miyuki.foundation/log.hpp>

namespace miyuki::core {
    class UniformLightDistribution final : public LightDistribution {
        std::vector<std::shared_ptr<const Light>> lights;
        std::unordered_map<const Light *, Float> pdfMap;
    public:
        MYK_DECL_CLASS(UniformLightDistribution, "UniformLightDistribution", interface = "LightDistribution")

        const Light *sampleLight(Sampler &sampler, Float *pdf) override {
            if (lights.empty())
                return nullptr;
            auto idx = std::min<int>(sampler.next1D() * lights.size(), lights.size() - 1);
            *pdf = 1.0 / lights.size();
            return lights[idx].get();
        }

        Float lightPdf(const Light *light) override {
            auto it = pdfMap.find(light);
            if (it == pdfMap.end())return 0.0f;
            return it->second;
        }

        void build(Scene &scene) override {
            lights.clear();
            for (auto &i: scene.lights) {
                lights.emplace_back(i);
                pdfMap[i.get()] = 1.0 / scene.lights.size();
            }
            log::log("Lights: {}\n", lights.size());
        }
    };
}
#endif //MIYUKIRENDERER_UNIFORMLIGHTDISTRIBUTION_H
