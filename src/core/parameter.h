//
// Created by Shiina Miyuki on 2019/3/3.
//

#ifndef MIYUKI_PARAMETER_H
#define MIYUKI_PARAMETER_H

#include <utils/jsonparser.hpp>
#include "miyuki.h"
#include "core/geometry.h"
#include "io/serialize.h"

namespace Miyuki {
    class ParameterSet {
        std::unordered_map<std::string, int> ints;
        std::unordered_map<std::string, Float> floats;
        std::unordered_map<std::string, Vec3f> vec3fs;
        std::unordered_map<std::string, std::string> strings;

        template<typename T>
        friend Json::JsonObject IO::serialize(const T &set);

        template<typename T>
        friend T IO::deserialize(const Json::JsonObject &);

    public:
        void addInt(const std::string &name, int i);

        void addFloat(const std::string &name, Float f);

        void addVec3f(const std::string &name, const Vec3f &v);

        void addString(const std::string &name, const std::string &s);

        Float findFloat(const std::string &name, Float defaultValue) const;

        int findInt(const std::string &name, int defaultValue) const;

        const Vec3f &findVec3f(const std::string &name, const Vec3f &defaultValue) const;

        const std::string &findString(const std::string &name, const std::string &defaultValue) const;

    };
    namespace IO {
        template<>
        Json::JsonObject serialize<ParameterSet>(const ParameterSet &set);

        template<>
        ParameterSet deserialize<ParameterSet>(const Json::JsonObject &);
    }
}
#endif //MIYUKI_PARAMETER_H
