//
// Created by Shiina Miyuki on 2019/3/3.
//

#include "parameter.h"

namespace Miyuki {

    void ParameterSet::addInt(const std::string &name, int i) {
        ints[name] = i;
    }

    void ParameterSet::addFloat(const std::string &name, Float f) {
        floats[name] = f;
    }

    void ParameterSet::addVec3f(const std::string &name, const Vec3f &v) {
        vec3fs[name] = v;
    }

    void ParameterSet::addString(const std::string &name, const std::string &s) {
        strings[name] = s;
    }

#define HELPER(x)   auto iter = x.find(name);\
                            if(iter == x.end()){\
                            return defaultValue;\
                                            }\
                             return iter->second;

    Float ParameterSet::findFloat(const std::string &name, Float defaultValue) const {
        HELPER(floats)
    }

    int ParameterSet::findInt(const std::string &name, int defaultValue) const {
        HELPER(ints)
    }

    const Vec3f &ParameterSet::findVec3f(const std::string &name, const Vec3f &defaultValue) const {
        HELPER(vec3fs)
    }

    const std::string &ParameterSet::findString(const std::string &name, const std::string &defaultValue) const {
        HELPER(strings)
    }

    template<>
    Json::JsonObject IO::serialize<ParameterSet>(const ParameterSet &set) {
        Json::JsonObject result = Json::JsonObject::makeObject();
        result["ints"] = serialize(set.ints);
        result["floats"] = serialize(set.floats);
        result["vec3fs"] = serialize(set.vec3fs);
        result["strings"] = serialize(set.strings);
        return result;
    }

    template<>
    ParameterSet IO::deserialize<ParameterSet>(const Json::JsonObject &obj) {
        ParameterSet set;
        set.ints = deserialize<decltype(set.ints)>(obj["ints"]);
        set.floats = deserialize<decltype(set.floats)>(obj["floats"]);
        set.vec3fs = deserialize<decltype(set.vec3fs)>(obj["vec3fs"]);
        set.strings = deserialize<decltype(set.strings)>(obj["strings"]);
        return std::move(set);
    }
}