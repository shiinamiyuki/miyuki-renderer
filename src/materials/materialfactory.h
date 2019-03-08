//
// Created by Shiina Miyuki on 2019/3/6.
//

#ifndef MIYUKI_MATERIALFACTORY_H
#define MIYUKI_MATERIALFACTORY_H

#include "material.h"

namespace Miyuki {
    struct Mesh;

    class MaterialFactory {
        std::unordered_map<std::string, std::shared_ptr<Material>> materials;
    public:
        MaterialFactory();

        std::shared_ptr<Material> createMaterial(const std::string& name,Json::JsonObject mtl);

        void applyMaterial(Json::JsonObject shapes, Json::JsonObject mtl, Mesh &);
    };
}
#endif //MIYUKI_MATERIALFACTORY_H
