//
// Created by Shiina Miyuki on 2019/3/6.
//

#ifndef MIYUKI_MATERIALFACTORY_H
#define MIYUKI_MATERIALFACTORY_H

#include "material.h"

namespace Miyuki {
    struct Mesh;

    class Scene;

    class MaterialFactory {
        friend class Scene;

        ImageLoader loader;
        std::unordered_map<std::string, std::shared_ptr<Material>> materials;
    public:
        MaterialFactory();

        std::shared_ptr<Material> _createMaterial(const std::string &name, Json::JsonObject mtl);

        std::shared_ptr<Material> createMaterial(const std::string &name, Json::JsonObject mtl);

        Texture deserialize(Json::JsonObject);

        void applyMaterial(Json::JsonObject shapes, Json::JsonObject mtl, Mesh &);

        void modifyMaterialByNameFromJson(const std::string &name, Json::JsonObject);

        std::shared_ptr<Material> getMaterialByName(const std::string &name);
    };
}
#endif //MIYUKI_MATERIALFACTORY_H
