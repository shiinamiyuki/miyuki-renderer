//
// Created by Shiina Miyuki on 2019/2/28.
//

#ifndef MIYUKI_SERIALIZE_H
#define MIYUKI_SERIALIZE_H

#include "io.h"
#include "utils/jsonparser.hpp"
#include "core/geometry.h"

namespace Miyuki {
    namespace IO {
        template<class T>
        struct Serializer {
            static Json::JsonObject serialize(const T &v);
        };

        template<class T>
        struct Deserializer {
            static T deserialize(const Json::JsonObject &obj);
        };

        template<class T>
        Json::JsonObject serialize(const T &v) {
            return Serializer<T>::serialize(v);
        }

        template<>
        inline Json::JsonObject serialize<int>(const int &i) {
            return Json::JsonObject(i);
        }

        template<>
        inline Json::JsonObject serialize<float>(const float &i) {
            return Json::JsonObject(i);
        }
        template<>
        inline Json::JsonObject serialize<std::string>(const std::string &i) {
            return Json::JsonObject(i);
        }
        template<>
        Json::JsonObject serialize<Vec3f>(const Vec3f &v);
        
        template<>
        Json::JsonObject serialize<Point2f>(const Point2f &v);

        template<class T>
        T deserialize(const Json::JsonObject &obj) {
            return std::move(Deserializer<T>::deserialize(obj));
        }

        template<>
        inline int deserialize<int>(const Json::JsonObject &obj) {
            return obj.getInt();
        }

        template<>
        inline float deserialize<float>(const Json::JsonObject &obj) {
            return obj.getFloat();
        }
        template<>
        inline std::string deserialize<std::string>(const Json::JsonObject &obj) {
            return obj.getString();
        }
        template<>
        Vec3f deserialize<Vec3f>(const Json::JsonObject &);

        template<class V>
        struct Serializer<std::unordered_map<std::string, V>> {
            static Json::JsonObject serialize(const std::unordered_map<std::string, V> &map) {
                auto result = Json::JsonObject::makeObject();
                for (const auto &i:map) {
                    result[i.first] = IO::serialize<decltype(i.second)>(i.second);
                }
                return result;
            }
        };

        template<class V>
        struct Deserializer<std::unordered_map<std::string, V>> {
            static std::unordered_map<std::string, V> deserialize(const Json::JsonObject &obj) {
                auto result = std::unordered_map<std::string, V>();
                for (const auto &i:obj.getObject()) {
                    result[i.first] = IO::deserialize<V>(i.second);
                }
                return std::move(result);
            }
        };
    }
}

#endif //MIYUKI_SERIALIZE_H
