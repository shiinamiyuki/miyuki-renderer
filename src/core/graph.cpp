//
// Created by shiinamiyuki on 10/31/2019.
//

#include <api/graph.h>
#include <api/shape.h>
#include <api/sampler.h>
#include <api/film.h>
#include <api/log.hpp>

namespace miyuki::core {
    void SceneGraph::render(const std::string &outImageFile) {
        camera->preprocess();
        integrator->preprocess();
        sampler->preprocess();
        for (auto &i:shapes) {
            i->preprocess();
        }
        Film film(filmDimension[0], filmDimension[1]);
        auto scene = std::make_shared<Scene>();
        for (auto i: shapes) {
            scene->primitives.emplace_back(i);
        }
        scene->preprocess();
        log::log("Start Rendering...\n");
        integrator->render(scene, camera, sampler, film);
        film.writeImage(outImageFile);
        log::log("Written to {}\n", outImageFile);

    }
}
