//
// Created by shiinamiyuki on 10/31/2019.
//

#ifndef MIYUKIRENDERER_GRAPH_H
#define MIYUKIRENDERER_GRAPH_H

#include <api/entity.hpp>
#include <api/serialize.hpp>
#include <api/bsdf.h>
#include <api/integrator.h>
#include <api/scene.h>
#include <api/camera.h>
#include <api/sampler.h>
#include <api/shape.h>
#include <cereal/types/vector.hpp>


namespace miyuki::core {

    class SceneGraph final : public Entity {
        std::shared_ptr<Camera> camera;
        std::shared_ptr<Integrator> integrator;
        std::shared_ptr<Sampler> sampler;
        std::vector<std::shared_ptr<Shape>> shapes;
        Point2i filmDimension;
    public:
        MYK_AUTO_SER(camera, sampler, integrator, shapes, filmDimension)

        MYK_AUTO_INIT(camera, sampler, integrator, shapes, filmDimension)

        MYK_DECL_CLASS(SceneGraph, "SceneGraph")

        void render(const std::string&outImageFile);
    };
}
#endif //MIYUKIRENDERER_GRAPH_H
