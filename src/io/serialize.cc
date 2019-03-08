//
// Created by Shiina Miyuki on 2019/2/28.
//

#include "serialize.h"

namespace Miyuki {
    template<>
    Json::JsonObject IO::serialize<Vec3f>(const Vec3f &v) {
        auto arr = Json::JsonObject::makeArray();
        for (int i = 0; i < 3; i++)
            arr.getArray().emplace_back(v[i]);
        return arr;

    }
    template<>
    Vec3f IO::deserialize<Vec3f>(const Json::JsonObject &object) {
        if (!object.isArray())
            return Vec3f();
        return Vec3f{object[0].getFloat(), object[1].getFloat(), object[2].getFloat()};
    }
    template<>
    Json::JsonObject IO::serialize<Point2f>(const Point2f &v) {
        auto arr = Json::JsonObject::makeArray();
        for (int i = 0; i < 2; i++)
            arr.getArray().emplace_back(v[i]);
        return arr;
    }
}
