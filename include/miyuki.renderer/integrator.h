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

#ifndef MIYUKIRENDERER_INTEGRATOR_H
#define MIYUKIRENDERER_INTEGRATOR_H

#include <miyuki.foundation/math.hpp>
#include <miyuki.foundation/object.hpp>
#include <miyuki.foundation/mpsc.hpp>
#include <miyuki.foundation/task.hpp>


namespace miyuki::core {
    class Camera;

    class Scene;

    class Sampler;

    class LightDistribution;

    struct Film;

    struct RenderSettings {
        Point2i filmDimension;
        size_t tileSize = 16;
        std::shared_ptr<Scene> scene;
        std::shared_ptr<Camera> camera;
        std::shared_ptr<Sampler> sampler;
        std::shared_ptr<LightDistribution> lightDistribution;
    };

    struct RenderOutput {
        std::shared_ptr<Film> film;

    };

    class Integrator : public Object {
    public:
        MYK_INTERFACE(Integrator, "Integrator")

        virtual Task<RenderOutput> createRenderTask(const RenderSettings &settings, const mpsc::Sender<std::shared_ptr<Film>>& tx) = 0;
    };
}


#endif //MIYUKIRENDERER_INTEGRATOR_H
