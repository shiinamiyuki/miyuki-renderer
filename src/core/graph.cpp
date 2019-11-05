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
        Film film(filmDimension[0], filmDimension[1]);
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
        log::log("Start Rendering...\n");
        integrator->render(scene, camera, sampler, film);
        film.writeImage(outImageFile);
        log::log("Written to {}\n", outImageFile);

    }
}
