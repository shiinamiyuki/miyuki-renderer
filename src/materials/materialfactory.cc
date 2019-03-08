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

    std::shared_ptr<Material> MaterialFactory::createMaterial(const std::string &name, Json::JsonObject mtl) {
        auto iter = materials.find(name);
        if (iter != materials.end()) {
            return iter->second;
        }
        MaterialInfo info;
        info.ka = IO::deserialize<Texture>(mtl["ka"]);
        info.kd = IO::deserialize<Texture>(mtl["kd"]);
        info.ks = IO::deserialize<Texture>(mtl["ks"]);
        if(mtl.hasKey("sigma")){
            info.sigma = mtl["sigma"].getFloat();
        }
        if(mtl.hasKey("Ni")){
            info.Ni = mtl["Ni"].getFloat();
        }
        if(mtl.hasKey("alphaX")){
            info.alphaX = mtl["alphaX"].getFloat();
        }
        if(mtl.hasKey("alphaY")){
            info.alphaY = mtl["alphaY"].getFloat();
        }
        if(mtl.hasKey("roughness")){
            info.roughness  = mtl["roughness"].getFloat();
        }
        if(mtl.hasKey("Tr")){
            info.roughness  = mtl["Tr"].getFloat();
        }
        return std::make_shared<PBRMaterial>(info);
    }
}