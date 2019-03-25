//
// Created by Shiina Miyuki on 2019/3/6.
//

#include "materialfactory.h"
#include "core/mesh.h"

namespace Miyuki {

    MaterialFactory::MaterialFactory() {

    }

    void MaterialFactory::applyMaterial(Json::JsonObject shapes, Json::JsonObject mtl, Mesh &mesh) {
        mesh.materials.clear();
        for (const auto &name:mesh.names) {
            const auto &mtlName = shapes[name].getString();
            mesh.materials.emplace_back(createMaterial(mtlName, mtl[mtlName]));
        }
    }

    Texture MaterialFactory::deserialize(Json::JsonObject obj) {
        std::string texture;
        if (obj.hasKey("texture"))
            texture = obj["texture"].getString();
        return Texture(IO::deserialize<Vec3f>(obj["albedo"]),
                       texture.empty() ? nullptr : loader.load(texture));
    }

    std::shared_ptr<Material> MaterialFactory::_createMaterial(const std::string &name, Json::JsonObject mtl) {
        MaterialInfo info;
        info.ka = deserialize(mtl["ka"]);
        info.kd = deserialize(mtl["kd"]);
        info.ks = deserialize(mtl["ks"]);
        if (mtl.hasKey("emission")) {
            info.emission = mtl["emission"].getFloat();
        }
        if (mtl.hasKey("sigma")) {
            info.sigma = mtl["sigma"].getFloat();
        }
        if (mtl.hasKey("Ni")) {
            info.Ni = mtl["Ni"].getFloat();
        }
        if (mtl.hasKey("Tr")) {
            info.Tr = mtl["Tr"].getFloat();
        }
        if (mtl.hasKey("alphaX")) {
            info.alphaX = mtl["alphaX"].getFloat();
        }
        if (mtl.hasKey("alphaY")) {
            info.alphaY = mtl["alphaY"].getFloat();
        }
        if (mtl.hasKey("roughness")) {
            info.roughness = mtl["roughness"].getFloat();
        }
        info.ka.multiplier = info.emission;
        return std::make_shared<PBRMaterial>(info);
    }

    std::shared_ptr<Material> MaterialFactory::createMaterial(const std::string &name, Json::JsonObject mtl) {
        auto iter = materials.find(name);
        if (iter != materials.end()) {
            return iter->second;
        }
        auto mat = _createMaterial(name, std::move(mtl));
        materials[name] = mat;
        return mat;
    }

    void MaterialFactory::modifyMaterialByNameFromJson(const std::string &name, Json::JsonObject obj) {
        materials[name] = _createMaterial(name, std::move(obj));
    }

    std::shared_ptr<Material> MaterialFactory::getMaterialByName(const std::string &name) {
        auto iter = materials.find(name);
        if (iter == materials.end())
            return nullptr;
        return iter->second;
    }
}