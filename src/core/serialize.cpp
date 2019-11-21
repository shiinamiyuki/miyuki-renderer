// MIT License
//
// Copyright (c) 2019 椎名深雪
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <api/serialize.hpp>

#include <api/log.hpp>
#include <api/object.hpp>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <unordered_set>

namespace miyuki {
    struct ObjectManager {
        static ObjectManager *_instance;

        static ObjectManager *instance() {
            static std::once_flag flag;
            std::call_once(flag, [&]() { _instance = new ObjectManager(); });
            return _instance;
        }

        std::unordered_map<std::string, std::weak_ptr<Object>> bindings;
        std::unordered_map<std::string, Type *> types;
        std::unordered_map<std::string, std::set<std::string>> impls;
    };

    ObjectManager *ObjectManager::_instance;

    void RegisterObject(const std::string &alias, Type *type) {
        ObjectManager::instance()->types[alias] = type;
        log::log("Registered {}\n", alias);
    }

    void BindInterfaceImplementation(const std::string &interface, const std::string &impl) {
        auto &m = ObjectManager::instance()->impls;
        if (m.find(interface) == m.end()) {
            m[interface] = {};
        }
        m[interface].insert(impl);
    }

    std::shared_ptr<Object> CreateObject(const std::string &type) {
        auto it = ObjectManager::instance()->types.find(type);
        if (it != ObjectManager::instance()->types.end()) {
            auto ty = it->second;
            auto entity = ty->create();
            if (!entity) {
                log::log("failed to create entity with type {}\n", type);
                return nullptr;
            }
            return entity;
        }
        return {};
    }

    std::shared_ptr<Object> CreateObjectParams(const nlohmann::json &params) {
        if (params.is_string()) {
            auto name = params.get<std::string>();
            return GetObject(name);
        } else {
            auto entity = CreateObject(params.at("type").get<std::string>());
            if (params.contains("@ref")) {
                BindObject(entity, params.at("@ref").get<std::string>());
            }
            entity->initialize(params);
            return entity;
        }
    }

    void BindObject(const std::shared_ptr<Object> &entity, const std::string &name) {
        auto &bindings = ObjectManager::instance()->bindings;
        bindings[name] = entity;
    }

    std::shared_ptr<Object> GetObject(const std::string &name) {
        auto &bindings = ObjectManager::instance()->bindings;
        auto iter = bindings.find(name);
        if (iter == bindings.end()) {
            return nullptr;
        }
        auto &wp = iter->second;
        if (wp.expired()) {
            bindings.erase(iter);
            return nullptr;
        }
        return wp.lock();
    }

    std::string Object::toString() const { return fmt::format("[{} at {}]", getType()->name(), (void *)this); }

    namespace serialize {
        void WriteObject(serialize::OutputArchive &ar, const std::shared_ptr<Object> &e) { ar(e); }

        std::shared_ptr<Object> ReadObject(serialize::InputArchive &ar) {
            std::shared_ptr<Object> e;
            ar(e);
            return e;
        }
    } // namespace serialize
} // namespace miyuki
