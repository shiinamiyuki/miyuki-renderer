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

#ifndef MIYUKIRENDERER_GRAPH_H
#define MIYUKIRENDERER_GRAPH_H

#include <api/bsdf.h>
#include <api/camera.h>
#include <api/entity.hpp>
#include <api/integrator.h>
#include <api/sampler.h>
#include <api/scene.h>
#include <api/serialize.hpp>
#include <api/shape.h>
#include <cereal/types/vector.hpp>

namespace miyuki::core {

    class SceneGraph final : public Entity {
        std::shared_ptr<Camera> camera;
        std::shared_ptr<Integrator> integrator;
        std::shared_ptr<Sampler> sampler;
        std::vector<std::shared_ptr<MeshBase>> shapes;
        Point2i filmDimension;

      public:
        MYK_AUTO_SER(camera, sampler, integrator, shapes, filmDimension)

        MYK_AUTO_INIT(camera, sampler, integrator, shapes, filmDimension)

        MYK_DECL_CLASS(SceneGraph, "SceneGraph")

        void render(const std::string &outImageFile);
    };
} // namespace miyuki::core
#endif // MIYUKIRENDERER_GRAPH_H
