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

#include <miyuki.renderer/graph.h>
#include <miyuki.renderer/shape.h>
#include <miyuki.renderer/sampler.h>
#include <miyuki.foundation/film.h>
#include <miyuki.foundation/log.hpp>

namespace miyuki::core {
    Task<RenderOutput> SceneGraph::createRenderTask(const mpsc::Sender<std::shared_ptr<Film>> &tx) {

        camera->preprocess();
        integrator->preprocess();
        sampler->preprocess();
        auto scene = std::make_shared<Scene>();
        for (const auto &i: shapes) {
            if (auto mesh = std::dynamic_pointer_cast<Mesh>(i)) {
                scene->meshes.emplace_back(mesh);
            } else if (auto instance = std::dynamic_pointer_cast<MeshInstance>(i)) {
                scene->instances.emplace_back(instance);
            } else {
                MIYUKI_THROW(std::runtime_error, "Unknown shape type");
            }
        }
        scene->preprocess();

        RenderSettings settings{};
        settings.filmDimension = filmDimension;
        return integrator->createRenderTask(settings, scene, camera, sampler, tx);
    }

    void SceneGraph::render(const std::string &outImageFile) {
        auto [tx, rx] = mpsc::channel<std::shared_ptr<Film>>();
        Task<RenderOutput> task = createRenderTask(tx);
        log::log("Start Rendering...\n");
        task.launch();
        std::shared_ptr<Film> film;
        if (auto r = task.wait()) {
            film = r.value().film;
        }
        if (film) {
            film->writeImage(outImageFile);
        } else {
            log::log("Render failed\n");
        }
    }
}
