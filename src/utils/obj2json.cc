//
// Created by Shiina Miyuki on 2019/2/28.
//

#include "obj2json.h"
#include "io/io.h"
#include "io/serialize.h"

namespace Miyuki {
    namespace IO {
        Json::JsonObject serialize(const tinyobj::real_t v[3]) {
            auto arr = Json::JsonObject::makeArray();
            for (int i = 0; i < 3; i++)
                arr.getArray().emplace_back(v[i]);
            return arr;
        }
    }

    std::tuple<Json::JsonObject, Json::JsonObject>
    MTL2Json(const std::string &filename) {
        Json::JsonObject mtl = Json::JsonObject::makeObject();
        Json::JsonObject shape = Json::JsonObject::makeObject();
        IO::readUnderPath(filename, [&](const std::string &file) {
            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;
            std::string err;
            fmt::print("Loading OBJ file {}\n", filename);
            bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, file.c_str());

            if (!err.empty()) { // `err` may contain warning message.
                fmt::print(stderr, "{}\n", err);
            }
            if (!ret) {
                fmt::print(stderr, "error loading OBJ file {}\n", file);
                return;
            }

            auto func = [&](const tinyobj::real_t v[3], const std::string &texName) -> Json::JsonObject {
                auto obj = Json::JsonObject::makeObject();
                obj["albedo"] = IO::serialize(v);
                obj["texture"] = texName;
                return obj;
            };
            for (const auto &s : shapes) {
                if (!shape.hasKey(s.name)) {
                    if (s.mesh.material_ids.size() > 1) {
                        fmt::print(stderr, "Only one material per object!\n");
                    }
                    shape[s.name] = materials[s.mesh.material_ids[0]].name;
                }
            }
            for (const auto &material : materials) {
                if (mtl.hasKey(material.name)) {
                    fmt::print("{} already exists, skipping", material.name);
                    continue;
                }
                Json::JsonObject m = Json::JsonObject::makeObject();
                m["ka"] = func(material.emission, material.emissive_texname);
                m["kd"] = func(material.diffuse, material.diffuse_texname);
                m["ks"] = func(material.specular, material.specular_texname);
                m["roughness"] = Json::JsonObject(material.roughness);
                m["Ni"] = Json::JsonObject(material.ior);
                m["Tr"] = Json::JsonObject(1 - material.dissolve);

                mtl[material.name] = m;
            }
        });
        return {mtl, shape};
    }

    void MergeMTL2Json(const std::string &mtl, const std::string &json) {
        Json::JsonObject object;
        {
            std::ifstream stream(json);

            if (stream) {
                std::string content((std::istreambuf_iterator<char>(stream)),
                                    (std::istreambuf_iterator<char>()));
                object = Json::parse(content);
                stream.close();
            } else {
                object = Json::JsonObject::makeObject();
            }

        }
        auto tuple = MTL2Json(mtl);
        auto m = std::get<0>(tuple);
        auto s = std::get<1>(tuple);
        if (!object.hasKey("materials")) {
            object["materials"] = Json::JsonObject::makeObject();
        }
        if (!object.hasKey("shapes")) {
            object["shapes"] = Json::JsonObject::makeObject();
        }
        for (const auto &i: m.getObject()) {
            if (object["materials"].hasKey(i.first)) {
                fmt::print(stderr, "Material {} already exists\n", i.first);
            }
            object["materials"][i.first] = i.second;
        }
        for (const auto &i:s.getObject()) {
            if (object["shapes"].hasKey(i.first)) {
                fmt::print(stderr, "Shape {} already exists\n", i.first);
            }
            object["shapes"][i.first] = i.second;
        }
        {
            std::ofstream stream(json);
            if (!stream.is_open()) {
                fmt::print(stderr, "Cannot open {}\n", json);
            }
            stream << object.toString() << std::ends;
        }
    }
}
